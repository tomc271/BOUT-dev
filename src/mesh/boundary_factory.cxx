#include "bout/assert.hxx"
#include "bout/parallel_boundary_op.hxx"
#include "bout/parallel_boundary_region.hxx"
#include <bout/boundary_factory.hxx>
#include <bout/boundary_region_iter.hxx>
#include <bout/boundary_standard.hxx>
#include <bout/globals.hxx>
#include <bout/options.hxx>
#include <bout/utils.hxx>

#include <array>
#include <list>
#include <map>
#include <string>
#include <string_view>

using std::list;
using std::string;

#include <bout/output.hxx>

BoundaryFactory* BoundaryFactory::instance = nullptr;

BoundaryFactory::BoundaryFactory() {
  add(new BoundaryDirichlet(), "dirichlet");
  add(new BoundaryDirichlet(), "dirichlet_o2"); // Synonym for "dirichlet"
  add(new BoundaryDirichlet_O3(), "dirichlet_o3");
  add(new BoundaryDirichlet_O4(), "dirichlet_o4");
  add(new BoundaryDirichlet_4thOrder(), "dirichlet_4thorder");
  add(new BoundaryNeumann(), "neumann");
  add(new BoundaryNeumann(), "neumann_O2"); // Synonym for "neumann"
  add(new BoundaryNeumann_4thOrder(), "neumann_4thorder");
  add(new BoundaryNeumann_O4(), "neumann_O4");
  add(new BoundaryNeumannPar(), "neumannpar");
  add(new BoundaryNeumann_NonOrthogonal(), "neumann_nonorthogonal");
  add(new BoundaryRobin(), "robin");
  add(new BoundaryConstGradient(), "constgradient");
  add(new BoundaryZeroLaplace(), "zerolaplace");
  add(new BoundaryZeroLaplace2(), "zerolaplace2");
  add(new BoundaryConstLaplace(), "constlaplace");
  add(new BoundaryFree(), "free");
  add(new BoundaryFree_O2(), "free_o2");
  add(new BoundaryFree_O3(), "free_o3");

  addMod(new BoundaryRelax(), "relax");
  addMod(new BoundaryWidth(), "width");
  addMod(new BoundaryToFieldAligned(), "toFieldAligned");
  addMod(new BoundaryFromFieldAligned(), "fromFieldAligned");

  // Parallel boundaries
  add(new BoundaryOpPar_dirichlet_o1(), "parallel_dirichlet_o1");
  add(new BoundaryOpPar_dirichlet_o2(), "parallel_dirichlet_o2");
  add(new BoundaryOpPar_dirichlet_o3(), "parallel_dirichlet_o3");
  add(new BoundaryOpPar_neumann_o1(), "parallel_neumann_o1");
  add(new BoundaryOpPar_neumann_o2(), "parallel_neumann_o2");
  add(new BoundaryOpPar_neumann_o3(), "parallel_neumann_o3");
}

BoundaryFactory::~BoundaryFactory() {
  // Free any boundaries
  for (const auto& [key, op] : opmap) {
    delete op;
  }
  for (const auto& [key, mod] : modmap)
    delete mod;
  for (const auto& [key, par_op] : par_opmap)
    delete par_op;
}

BoundaryFactory* BoundaryFactory::getInstance() {
  if (instance == nullptr) {
    // Create the singleton object
    instance = new BoundaryFactory();
  }
  return instance;
}

void BoundaryFactory::cleanup() {
  if (instance == nullptr) {
    return;
  }

  // Just delete the instance
  delete instance;
  instance = nullptr;
}

BoundaryOpBase* BoundaryFactory::create(std::string_view name_sv,
                                        BoundaryRegionBase* region) {
  string name(name_sv); // Convert string_view to string for local mutations
  // Search for a string of the form: modifier(operation)
  auto pos = name.find('(');

  if (pos == string::npos) {
    // No more (opening) brackets. Should be a boundary operation
    // Need to strip whitespace

    if (name == "null" || name == "none") {
      return nullptr;
    }

    if (region->isParallel) {
      // Parallel boundary
      if (auto* pop = findBoundaryOpPar(trim(name)); pop != nullptr) {
        return pop->clone(dynamic_cast<bout::boundary::BoundaryRegionFCI*>(region), {},
                          {});
      }
      throw BoutException("Could not find parallel boundary condition '{:s}'", name);
    }

    // Clone the boundary operation, passing the region to operate over,
    // an empty args list and empty keyword map
    // Perpendicular boundary
    if (auto* op = findBoundaryOp(trim(name)); op != nullptr) {
      return op->clone(region->getLegacyPointer(), {}, {});
    }
    throw BoutException("Could not find boundary condition '{:s}'", name);
  }

  // Contains a bracket. Find the last bracket and remove
  auto pos2 = name.rfind(')');
  if (pos2 == string::npos) {
    output_warn << "\tWARNING: Unmatched brackets in boundary condition: " << name
                << endl;
  }

  // Find the function name before the bracket
  string func = trim(name.substr(0, pos));
  // And the argument inside the bracket
  string arg = trim(name.substr(pos + 1, pos2 - pos - 1));
  // Split the argument on commas
  // NOTE: Commas could be part of sub-expressions, so
  //       need to take account of brackets
  list<string> arglist;
  std::map<std::string, std::string> keywords;
  int level = 0;
  int start = 0;
  for (string::size_type i = 0; i < arg.length(); i++) {
    switch (arg[i]) {
    case '(':
    case '[':
    case '<':
      level++;
      break;
    case ')':
    case ']':
    case '>':
      level--;
      break;
    case ',': {
      if (level == 0) {
        string s = arg.substr(start, i - start);

        // Check if s contains '=', and if so treat as a keyword
        if (auto poseq = s.find('='); poseq != string::npos) {
          keywords[trim(s.substr(0, poseq))] = trim(s.substr(poseq + 1));
        } else {
          // No '=', so a positional argument
          arglist.push_back(trim(s));
        }
        start = i + 1;
      }
      break;
    }
    };
  }

  string s = arg.substr(start);
  if (auto poseq = s.find('='); poseq != string::npos) {
    keywords[trim(s.substr(0, poseq))] = trim(s.substr(poseq + 1));
  } else {
    // No '=', so a positional argument
    arglist.push_back(trim(s));
  }

  // Test if func is a modifier
  if (auto* mod = findBoundaryMod(func); mod != nullptr) {
    // The first argument should be an operation
    auto* op = dynamic_cast<BoundaryOp*>(create(arglist.front(), region));
    if (op == nullptr) {
      return nullptr;
    }

    // Remove the first element (name of operation)
    arglist.pop_front();

    // Clone the modifier, passing in the operator and remaining strings as argument
    return mod->cloneMod(op, arglist);
  }

  if (auto* pop = findBoundaryOpPar(trim(func)); pop != nullptr) {
    // An operation with arguments
    if (region->isParallel) {
      return pop->clone(dynamic_cast<bout::boundary::BoundaryRegionFCI*>(region), arglist,
                        keywords);
    }
    if (region->isX) {
      return pop->clone(dynamic_cast<bout::boundary::BoundaryRegionX*>(region), arglist,
                        keywords);
    }
    if (region->isY) {
      return pop->clone(dynamic_cast<bout::boundary::BoundaryRegionY*>(region), arglist,
                        keywords);
    }
  }
  if (!region->isParallel) {
    // Legacy perpendicular boundary
    if (auto* op = findBoundaryOp(trim(func)); op != nullptr) {
      // An operation with arguments
      return op->clone(region->getLegacyPointer(), arglist, keywords);
    }
  }

  // Otherwise nothing matches
  throw BoutException("  Boundary setting is neither an operation nor modifier: {:s}\n",
                      func);

  return nullptr;
}

BoundaryOpBase* BoundaryFactory::createFromOptions(std::string_view varname_sv,
                                                   BoundaryRegionBase* region) {
  if (region == nullptr)
    return nullptr;
  string varname(varname_sv);

  output_info << "\t" << region->label << " region: ";

  std::array<string, 5> sides = {
      region->label,
      "all",                                 // [1] fallback
      "all",                                 // [2] fallback
      "all",                                 // [3] fallback
      region->isParallel ? "par_all" : "all" // [4]
  };

  ASSERT2(region->location != BNDRY_INVALID)
  switch (region->location) {
  case BNDRY_XIN:
    sides[1] = "xin";
    break;
  case BNDRY_XOUT:
    sides[1] = "xout";
    break;
  case BNDRY_YDOWN:
    sides[1] = "ydown";
    break;
  case BNDRY_YUP:
    sides[1] = "yup";
    break;
  case BNDRY_PAR_FWD_XIN:
    sides[1] = "par_yup_xin";
    sides[2] = "par_xin";
    sides[3] = "par_yup";
    break;
  case BNDRY_PAR_FWD_XOUT:
    sides[1] = "par_yup_xout";
    sides[2] = "par_xout";
    sides[3] = "par_yup";
    break;
  case BNDRY_PAR_BKWD_XIN:
    sides[1] = "par_ydown_xin";
    sides[2] = "par_xin";
    sides[3] = "par_ydown";
    break;
  case BNDRY_PAR_BKWD_XOUT:
    sides[1] = "par_ydown_xout";
    sides[2] = "par_xout";
    sides[3] = "par_ydown";
    break;
  default:
    break;
  }

  // Get options
  Options* options = Options::getRoot();

  // Get variable options
  Options* varOpts = options->getSection(varname);
  string prefix("bndry_");
  string set;

  /// First try looking for (var, ...)
  for (const auto& side : sides) {
    if (varOpts->isSet(prefix + side)) {
      varOpts->get(prefix + side, set, "");
      return create(set, region);
    }
  }

  // Get the "all" options
  varOpts = options->getSection("all");

  /// First try looking for (all, ...)
  for (const auto& side : sides) {
    if (varOpts->isSet(prefix + side)) {
      varOpts->get(prefix + side, set,
                   region->isParallel ? "parallel_dirichlet_o2" : "dirichlet");
      return create(set, region);
    }
  }

  /// Then (all, all)
  if (region->isParallel) {
    // Different default for parallel boundary regions
    varOpts->get(prefix + "par_all", set, "parallel_dirichlet_o2");
  } else {
    varOpts->get(prefix + "all", set, "dirichlet");
  }
  return create(set, region);
  // Defaults to Dirichlet conditions, to prevent undefined boundary
  // values. If a user want to override, specify "none" or "null"
}

void BoundaryFactory::add(BoundaryOp* bop, std::string_view name) {
  string lower_name = lowercase(string(name));
  // C++20: .contains() allows us to check existence effortlessly
  if (modmap.contains(lower_name) || opmap.contains(lower_name)) {
    output_error << "ERROR: Trying to add an already existing boundary: " << name << endl;
    return;
  }
  opmap[lower_name] = bop;
}

void BoundaryFactory::add(BoundaryOpPar* bop, std::string_view name) {
  string lower_name = lowercase(string(name));
  if (par_opmap.contains(lower_name)) {
    output_error << "ERROR: Trying to add an already existing boundary: " << name << endl;
    return;
  }
  par_opmap[lower_name] = bop;
}

void BoundaryFactory::addMod(BoundaryModifier* bmod, std::string_view name) {
  string lower_name = lowercase(string(name));
  if (modmap.contains(lower_name) || opmap.contains(lower_name)) {
    output_error << "ERROR: Trying to add an already existing boundary modifier: " << name
                 << endl;
    return;
  }
  modmap[lower_name] = bmod;
}

BoundaryOp* BoundaryFactory::findBoundaryOp(std::string_view s) {
  // C++17 Init-statement
  if (auto it = opmap.find(lowercase(string(s))); it != opmap.end()) {
    return it->second;
  }
  return nullptr;
}

BoundaryModifier* BoundaryFactory::findBoundaryMod(std::string_view s) {
  if (auto it = modmap.find(lowercase(string(s))); it != modmap.end()) {
    return it->second;
  }
  return nullptr;
}

BoundaryOpPar* BoundaryFactory::findBoundaryOpPar(std::string_view s) {
  if (auto it = par_opmap.find(lowercase(string(s))); it != par_opmap.end()) {
    return it->second;
  }
  return nullptr;
}
