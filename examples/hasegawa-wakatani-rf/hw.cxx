
#include <bout/physicsmodel.hxx>
#include <smoothing.hxx>
#include <invert_laplace.hxx>
#include <derivs.hxx>

#include <fstream>
#include <algorithm>
#include <cmath>

class HW : public PhysicsModel {
private:
  Field3D n, vort;  // Evolving density and vorticity
  Field3D phi;      // Electrostatic potential

  // Model parameters
  BoutReal alpha;      // Adiabaticity (~conductivity)
  BoutReal kappa;      // Density gradient drive
  BoutReal Dvort, Dn;  // Diffusion 
  bool modified; // Modified H-W equations?
  
  // Poisson brackets: b0 x Grad(f) dot Grad(g) / B = [f, g]
  // Method to use: BRACKET_ARAKAWA, BRACKET_STD or BRACKET_SIMPLE
  BRACKET_METHOD bm; // Bracket method for advection terms
  
  std::unique_ptr<Laplacian> phiSolver; // Laplacian solver for vort -> phi

  // Simple implementation of 4th order perpendicular Laplacian
  Field3D Delp4(const Field3D &var) {
    Field3D tmp;
    tmp = Delp2(var);
    mesh->communicate(tmp);
    tmp.applyBoundary("neumann");
    return Delp2(tmp);

    //return Delp2(var);
  }
  
protected:
  int init(bool UNUSED(restart)) {

    auto& options = Options::root()["hw"];
    alpha = options["alpha"].withDefault(1.0);
    kappa = options["kappa"].withDefault(0.1);
    Dvort = options["Dvort"].withDefault(1e-2);
    Dn = options["Dn"].withDefault(1e-2);

    modified = options["modified"].withDefault(false);

    SOLVE_FOR(n, vort);
    SAVE_REPEAT(phi);

    // Split into convective and diffusive parts
    setSplitOperator();

    phiSolver = Laplacian::create();
    phi = 0.; // Starting phi

    auto phi_boundary_file = options["phi_boundary_file"]
      .doc("A file containing the potential at the outer boundary along Z")
      .withDefault<std::string>("none");
    if (phi_boundary_file != "none") {
      // Set the boundary flags so that the imposed phi boundary will be used
      output.write("Setting outer boundary from file. Overrides outer boundary condition");
      phiSolver->setOuterBoundaryFlags(INVERT_SET);

      if (mesh->lastX()) {
        output.write("Reading phi boundary data from '{}'\n", phi_boundary_file);

        // Read a file containing the boundary data
        std::fstream input(phi_boundary_file);

        std::vector<BoutReal> phi_values;
        while (input.good()) {
          BoutReal value;
          if (input >> value) {
            if (!std::isfinite(value)) {
              throw BoutException("Non-finite value ({}) encountered in phi boundary input at position {}",
                                  value, phi_values.size());
            }
            phi_values.push_back(value);
          }
        }
        auto minmax = std::minmax_element(std::begin(phi_values), std::end(phi_values));
        output.write("  => Read {} values. Minmum {}, Maximum {}\n",
                     phi_values.size(), *minmax.first, *minmax.second);

        // Ratio of the number of Z points in the input, to number of Z points in the domain
        BoutReal size_ratio = static_cast<BoutReal>(phi_values.size()) / mesh->LocalNz;

        // Now interpolate and set phi boundaries
        // For now a simple linear interpolation
        for (int z = 0; z < mesh->LocalNz; z++) {
          BoutReal float_index = static_cast<BoutReal>(z) * size_ratio;

          int left_index = static_cast<int>(float_index); // Truncate down to index
          int right_index = (left_index + 1) % phi_values.size(); // Wrap around periodic domain

          BoutReal delta = float_index - left_index; // Between 0 and 1
          BoutReal value = phi_values[left_index] * (1 - delta) + phi_values[right_index] * delta;

          // Set both points either side of the boundary
          phi(mesh->xend, mesh->ystart, z) = phi(mesh->xend + 1, mesh->ystart, z) = value;
        }
      } else {
        output.write("Reading phi boundary data on another processor\n");
      }
    }

    // Use default flags 

    // Choose method to use for Poisson bracket advection terms
    switch(options["bracket"].withDefault(0)) {
    case 0: {
      bm = BRACKET_STD; 
      output << "\tBrackets: default differencing\n";
      break;
    }
    case 1: {
      bm = BRACKET_SIMPLE; 
      output << "\tBrackets: simplified operator\n";
      break;
    }
    case 2: {
      bm = BRACKET_ARAKAWA; 
      output << "\tBrackets: Arakawa scheme\n";
      break;
    }
    case 3: {
      bm = BRACKET_CTU; 
      output << "\tBrackets: Corner Transport Upwind method\n";
      break;
    }
    default:
      output << "ERROR: Invalid choice of bracket method. Must be 0 - 3\n";
      return 1;
    }
    
    return 0;
  }

  int convective(BoutReal UNUSED(time)) {
    // Non-stiff, convective part of the problem

    // Need to move the boundary because phisolver takes the value in the guard cell
    if (mesh->lastX()) {
      for (int z = 0; z < mesh->LocalNz; z++) {
        phi(mesh->xend + 1, mesh->ystart, z) = 0.5 * (phi(mesh->xend, mesh->ystart, z) + phi(mesh->xend+1, mesh->ystart, z));
      }
    }

    // Solve for potential
    phi = phiSolver->solve(vort, phi);

    // Communicate variables
    mesh->communicate(n, vort, phi);
    
    // Modified H-W equations, with zonal component subtracted from resistive coupling term
    Field3D nonzonal_n = n;
    Field3D nonzonal_phi = phi;
    if(modified) {
      // Subtract average in Y and Z
      nonzonal_n -= averageY(DC(n));
      nonzonal_phi -= averageY(DC(phi));
    }
    
    ddt(n) = -bracket(phi, n, bm) + alpha*(nonzonal_phi - nonzonal_n) - kappa*DDZ(phi);
    
    ddt(vort) = -bracket(phi, vort, bm) + alpha*(nonzonal_phi - nonzonal_n);
  
    return 0;
  }
  
  int diffusive(BoutReal UNUSED(time)) {
    // Diffusive terms
    mesh->communicate(n, vort);
    ddt(n) = -Dn*Delp4(n);
    ddt(vort) = -Dvort*Delp4(vort);
    return 0;
  }
};

// Define a main() function
BOUTMAIN(HW);
