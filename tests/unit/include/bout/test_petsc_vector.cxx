#include "bout/build_defines.hxx"

#include <utility>

#include "gtest/gtest.h"

#include "bout/field2d.hxx"
#include "bout/field3d.hxx"
#include "bout/fieldperp.hxx"
#include "bout/operatorstencil.hxx"
#include "bout/petsc_interface.hxx"
#include "bout/region.hxx"
#include <cstdio>      // For vsnprintf and fflush
#include <iostream>    // For std::cout (already included via gtest, but explicit)
#include <petscsys.h>  // For PetscGetVersion and PetscErrorPrintf
#include <sstream>     // For std::stringstream
#include <cstdarg>     // For va_list, va_start, va_end
#include <unistd.h>    // For dup, pipe, read, STDERR_FILENO
#include <sys/types.h> // For ssize_t (if not in unistd.h)

#if BOUT_HAS_PETSC

#include <petscconf.h>

#include "fake_mesh_fixture.hxx"

// The unit tests use the global mesh
using namespace bout::globals;

// Reuse the "standard" fixture for FakeMesh
template <typename F>
class PetscVectorTest : public FakeMeshFixture {
public:
  using ind_type = typename F::ind_type;
  //  WithQuietOutput all{output};
  F field;
  OperatorStencil<ind_type> stencil;
  IndexerPtr<F> indexer;

  PetscVectorTest()
      : FakeMeshFixture(), field(1.5, bout::globals::mesh),
        stencil(squareStencil<ind_type>(bout::globals::mesh)),
        indexer(std::make_shared<GlobalIndexer<F>>(bout::globals::mesh, stencil)) {
    PetscErrorPrintf = PetscErrorPrintfNone;
  }

  virtual ~PetscVectorTest() { PetscErrorPrintf = PetscErrorPrintfDefault; }
};

using FieldTypes = ::testing::Types<Field3D, Field2D, FieldPerp>;
TYPED_TEST_SUITE(PetscVectorTest, FieldTypes);

void testArraysEqual(PetscScalar* s1, PetscScalar* s2, PetscInt n) {
  for (int i = 0; i < n; i++) {
    EXPECT_DOUBLE_EQ(s1[i], s2[i]);
  }
}

void testVectorsEqual(Vec* v1, Vec* v2) {
  PetscScalar *v1Contents, *v2Contents;
  PetscInt n1, n2;
  VecGetArray(*v1, &v1Contents);
  VecGetArray(*v2, &v2Contents);
  VecGetLocalSize(*v1, &n1);
  VecGetLocalSize(*v2, &n2);
  ASSERT_EQ(n1, n2);
  testArraysEqual(v1Contents, v2Contents, n1);
}

// Test constructor from field
TYPED_TEST(PetscVectorTest, FieldConstructor) {
  BOUT_FOR(i, this->field.getRegion("RGN_ALL")) {
    this->field[i] = static_cast<BoutReal>(i.ind);
  }
  PetscVector<TypeParam> vector(this->field, this->indexer);
  Vec* vectorPtr = vector.get();
  PetscScalar* vecContents;
  PetscInt n;
  VecGetArray(*vectorPtr, &vecContents);
  VecGetLocalSize(*vectorPtr, &n);
  ASSERT_EQ(n, this->field.getNx() * this->field.getNy() * this->field.getNz());
  TypeParam result = vector.toField();
  BOUT_FOR(i, this->field.getRegion("RGN_NOY")) { EXPECT_EQ(result[i], this->field[i]); }
}

// Test copy constructor
TYPED_TEST(PetscVectorTest, CopyConstructor) {
  SCOPED_TRACE("CopyConstructor");
  PetscVector<TypeParam> vector(this->field, this->indexer);
  PetscVector<TypeParam> copy(vector);
  Vec *vectorPtr = vector.get(), *copyPtr = copy.get();
  EXPECT_NE(vectorPtr, copyPtr);
  testVectorsEqual(vectorPtr, copyPtr);
}

// Test move constructor
TYPED_TEST(PetscVectorTest, MoveConstructor) {
  PetscVector<TypeParam> vector(this->field, this->indexer);
  Vec vectorPtr = *vector.get();
  EXPECT_NE(vectorPtr, nullptr);
  PetscVector<TypeParam> moved(std::move(vector));
  Vec movedPtr = *moved.get();
  EXPECT_EQ(vectorPtr, movedPtr);
}

// Test assignment from field
TYPED_TEST(PetscVectorTest, FieldAssignment) {
  SCOPED_TRACE("FieldAssignment");
  PetscVector<TypeParam> vector(this->field, this->indexer);
  const TypeParam val(-10.);
  vector = val;
  Vec* vectorPtr = vector.get();
  PetscScalar* vecContents;
  PetscInt n;
  VecGetArray(*vectorPtr, &vecContents);
  VecGetLocalSize(*vectorPtr, &n);
  ASSERT_EQ(n, this->field.getNx() * this->field.getNy() * this->field.getNz());
  TypeParam result = vector.toField();
  BOUT_FOR(i, this->field.getRegion("RGN_NOY")) { EXPECT_EQ(result[i], val[i]); }
}

// Test copy assignment
TYPED_TEST(PetscVectorTest, CopyAssignment) {
  SCOPED_TRACE("CopyAssignment");
  PetscVector<TypeParam> vector(this->field, this->indexer);
  PetscVector<TypeParam> copy = vector;
  Vec* vectorPtr = vector.get();
  Vec* copyPtr = copy.get();
  EXPECT_NE(vectorPtr, copyPtr);
  testVectorsEqual(vectorPtr, copyPtr);
}

// Test move assignment
TYPED_TEST(PetscVectorTest, MoveAssignment) {
  PetscVector<TypeParam> vector(this->field, this->indexer);
  Vec vectorPtr = *vector.get();
  EXPECT_NE(vectorPtr, nullptr);
  PetscVector<TypeParam> moved = std::move(vector);
  Vec movedPtr = *moved.get();
  EXPECT_EQ(vectorPtr, movedPtr);
}

TYPED_TEST(PetscVectorTest, SetElement) {
  SCOPED_TRACE("FieldAssignment");
  PetscVector<TypeParam> vector(this->field, this->indexer);
  const TypeParam val(-10.);

  BOUT_FOR(index, val.getRegion("RGN_ALL")) { vector(index) = val[index]; }
  vector.assemble();

  Vec* vectorPtr = vector.get();
  PetscScalar* vecContents = nullptr;
  PetscInt size = 0;
  VecGetArray(*vectorPtr, &vecContents);
  VecGetLocalSize(*vectorPtr, &size);
  ASSERT_EQ(size, this->field.size());
  TypeParam result = vector.toField();
  BOUT_FOR(i, this->field.getRegion("RGN_NOY")) { EXPECT_EQ(result[i], val[i]); }
}

// Test getting elements
TYPED_TEST(PetscVectorTest, TestGetElements) {
  PetscVector<TypeParam> vector(this->field, this->indexer);
  BOUT_FOR(i, this->field.getRegion("RGN_NOBNDRY")) {
    vector(i) = (2.5 * this->field[i] - 1.0);
  }
  vector.assemble();
  TypeParam result = vector.toField();
  Vec* rawvec = vector.get();
  PetscScalar* vecContents = nullptr;
  VecAssemblyBegin(*rawvec);
  VecAssemblyEnd(*rawvec);
  VecGetArray(*rawvec, &vecContents);
  BOUT_FOR(i, this->field.getRegion("RGN_NOBNDRY")) {
    EXPECT_EQ(result[i], 2.5 * this->field[i] - 1.0);
  }
}

// Test getting constant elements
TYPED_TEST(PetscVectorTest, TestGetElementsConst) {
  const PetscVector<TypeParam> vector(this->field, this->indexer);
  BOUT_FOR(i, this->field.getRegion("RGN_NOBNDRY")) {
    const BoutReal element = vector(i);
    EXPECT_EQ(element, this->field[i]);
  }
}

#ifdef PETSC_USE_DEBUG

// Test trying to get an element from an uninitialised vector
TYPED_TEST(PetscVectorTest, TestGetUninitialised) {
  PetscVector<TypeParam> vector;
  typename TypeParam::ind_type index(0);
  EXPECT_THROW(vector(index), BoutException);
}

#if CHECKLEVEL >= 3
// Test trying to get an element that is out of bounds
TYPED_TEST(PetscVectorTest, TestGetOutOfBounds) {
  PetscVector<TypeParam> vector(this->field, this->indexer);
  typename TypeParam::ind_type index1(this->field.getNx() * this->field.getNy()
                                      * this->field.getNz());
  EXPECT_THROW(vector(index1), BoutException);
  typename TypeParam::ind_type index2(-1);
  EXPECT_THROW(vector(index2), BoutException);
  typename TypeParam::ind_type index3(10000000);
  EXPECT_THROW(vector(index3), BoutException);
}
#endif // CHECKLEVEL >= 3

TYPED_TEST(PetscVectorTest, TestMixedSetting) {
  PetscVector<TypeParam> vector(this->field, this->indexer);
  typename TypeParam::ind_type index1 = *(this->field.getRegion("RGN_NOBNDRY").begin());
  typename TypeParam::ind_type index2(index1.ind + 1);
  const PetscScalar r = 3.141592;
  vector(index1) = r;
  vector(index2) += r;
  vector.assemble();
  PetscScalar* vecContents = nullptr;
  VecGetArray(*(vector.get()), &vecContents);
  ASSERT_EQ(vecContents[index1.ind], r);
  ASSERT_EQ(vecContents[index2.ind], this->field[index2] + r);
}

// Test destroy
TYPED_TEST(PetscVectorTest, TestDestroy) {
  PetscVector<TypeParam> vector(this->field, this->indexer);
  Vec oldVec = *vector.get();
  Vec newVec;
  PetscErrorCode err;
  vector.destroy();
  err = VecDuplicate(oldVec, &newVec);
  ASSERT_NE(err, 0); // If original vector was destroyed, should not
                     // be able to duplicate it.
}

#endif // PETSC_USE_DEBUG

// Test swap
TYPED_TEST(PetscVectorTest, TestSwap) {
  PetscVector<TypeParam> lhs(this->field, this->indexer), rhs(this->field, this->indexer);
  Vec l0 = *lhs.get(), r0 = *rhs.get();
  EXPECT_NE(l0, nullptr);
  EXPECT_NE(r0, nullptr);
  swap(lhs, rhs);
  Vec l1 = *lhs.get(), r1 = *rhs.get();
  EXPECT_NE(l0, l1);
  EXPECT_NE(r0, r1);
  EXPECT_EQ(l0, r1);
  EXPECT_EQ(r0, l1);
}

#include <petscsys.h>  // For PetscGetVersion and PetscErrorPrintf
#include <sstream>     // For std::stringstream (already present)
#include <stdarg.h>    // For va_list, va_start, va_end
#include <cstdio>      // For vsnprintf and fflush
#include <unistd.h>    // For dup, pipe, read, STDERR_FILENO
#include <sys/types.h> // For ssize_t

// Helper: std::stringstream for PETSc error capture (static for global access)
static std::stringstream petsc_capture_stream;

// Custom printf handler: Matches PetscErrorCode (*)(const char format[], ...)
// Always returns 0 (PETSc success); warnings suppressed as intentional.
static PetscErrorCode petsc_capture_printf(const char *format, ...) {
  if (!format) return 0;

  // Format the variadic message into a buffer
  char buffer[1024];  // Reasonable size; PETSc messages are short
  va_list args;
  va_start(args, format);
  int len = std::vsnprintf(buffer, sizeof(buffer), format, args);
  va_end(args);

  // Append formatted message (with null-terminator if truncated)
  if (len > 0) {
    petsc_capture_stream << std::string(buffer, std::min(static_cast<size_t>(len), sizeof(buffer) - 1));
  }
  // Intentional: Always return success (0) for PETSc compatibility
  return 0;
}

// Updated TYPED_TEST with BOUT_FOR for ind_type compatibility
TYPED_TEST(PetscVectorTest, ReproducesNoisyPETScWarnings) {
  SCOPED_TRACE("ReproducesNoisyPETScWarnings");

  // Get PETSc version to conditionally enable noise checks
  char petsc_version_str[256] = {0};
  PetscErrorCode ierr = PetscGetVersion(petsc_version_str, sizeof(petsc_version_str));
  ASSERT_EQ(ierr, 0) << "Failed to get PETSc version";
  int major = 0, minor = 0, patch = 0;
  // Parsing: Match literal prefix to skip "PETSc Release Version "
  if (sscanf(petsc_version_str, "PETSc Release Version %d.%d.%d", &major, &minor, &patch) != 3) {
    ADD_FAILURE() << "Failed to parse PETSc version: " << petsc_version_str;
    major = 0;  // Fallback to skip
  }
  int version_num = major * 10000 + minor * 100 + patch;
  std::string version_str_for_msg = std::to_string(major) + "." + std::to_string(minor) + "." + std::to_string(patch);

  const int MIN_VERSION_FOR_WARNINGS = 31800;  // 3.18.0

  if (version_num < MIN_VERSION_FOR_WARNINGS) {
    GTEST_SKIP() << "PETSc version " << version_str_for_msg
                 << " (< 3.18) does not produce assembly warnings; skipping noise checks.";
  }

  // Clear capture streams
  petsc_capture_stream.str("");
  petsc_capture_stream.clear();

  // Temporarily set custom PETSc error printf to capture warnings directly
  auto old_error_printf = PetscErrorPrintf;
  PetscErrorPrintf = petsc_capture_printf;

  // FD Redirection for stderr only (captures PetscErrorPrintf and fprintf(stderr))
  int saved_stderr = dup(STDERR_FILENO);
  int stderr_pipe[2];
  pipe(stderr_pipe);

  // Redirect stderr to pipe
  dup2(stderr_pipe[1], STDERR_FILENO);

  // Reproduce the issue: Initial construction assembles from field.
  // Loop mixes = (INSERT_VALUES) and += (ADD_VALUES), without assembly between.
  // This mixes modes, triggering per-switch warnings in debug PETSc.
  PetscVector<TypeParam> vector(this->field, this->indexer);
  const TypeParam val(-10.);
  const BoutReal delta = 5.0;  // For += on initial field value (1.5)

  // Use BOUT_FOR for proper ind_type
  BOUT_FOR(ind, val.getRegion("RGN_ALL")) {
    // DEBUG: Force a test warning to verify capture (to stderr via PetscErrorPrintf, no comm)
    PetscErrorPrintf("Test warning for element %d\n", ind.ind);

    if (ind.ind % 2 == 0) {
      vector(ind) = val[ind];  // Triggers VecSetValues(INSERT_VALUES)
    } else {
      vector(ind) += delta;  // Triggers VecSetValues(ADD_VALUES) after prior INSERT
    }
  }
  vector.assemble();  // Ends assembly; mixing warnings during loop

  // Flush buffers before reading
  fflush(stderr);
  fflush(NULL);

  // Restore stderr
  dup2(saved_stderr, STDERR_FILENO);
  close(saved_stderr);
  close(stderr_pipe[1]);

  // Read captured output from stderr pipe
  std::string stderr_captured;
  char buf[1024];
  ssize_t bytes;
  while ((bytes = read(stderr_pipe[0], buf, sizeof(buf))) > 0) {
    stderr_captured.append(buf, bytes);
  }
  close(stderr_pipe[0]);

  std::string all_captured = petsc_capture_stream.str() + stderr_captured;

  // First, check if test warnings captured (verifies mechanism works)
  EXPECT_FALSE(all_captured.empty())
      << "No output captured at all (test warnings missing). Check PETSc init or FDs. "
      << "Version: " << version_str_for_msg << ". All captured: [" << all_captured << "]";

  bool test_warning_captured = (all_captured.find("Test warning") != std::string::npos);
  EXPECT_TRUE(test_warning_captured)
      << "Test warnings not captured—mechanism not working. Version: " << version_str_for_msg
      << ". All captured: [" << all_captured << "]";

  // Now check for real PETSc warnings (keywords for mixing modes)
  bool has_petsc_warning = (all_captured.find("mix INSERT_VALUES and ADD_VALUES") != std::string::npos ||
                            all_captured.find("Cannot mix INSERT_VALUES") != std::string::npos ||
                            all_captured.find("assembly") != std::string::npos ||
                            all_captured.find("VecSetValues") != std::string::npos ||
                            all_captured.find("state") != std::string::npos);
  if (test_warning_captured && !has_petsc_warning) {
    EXPECT_TRUE(has_petsc_warning) << "No PETSc mixing warnings detected; repro incomplete. "
                                   << "Mixing INSERT/ADD triggered, but BOUT-dev may assemble per-op. "
                                   << "Check petscvector.cxx for VecAssembly calls in Element. "
                                   << "Version: " << version_str_for_msg << ". All captured: [" << all_captured << "]";
  } else if (!test_warning_captured) {
    ADD_FAILURE() << "Capture broken—no test warnings. Debug FD setup.";
  } else {
    // Full verification if PETSc warnings appear
    EXPECT_TRUE(has_petsc_warning)
        << "Expected PETSc keywords like 'mix INSERT_VALUES and ADD_VALUES' or 'assembly'. "
        << "Version: " << version_str_for_msg << ". All captured: [" << all_captured << "]";

    // Count real warning lines (exclude test lines)
    size_t num_petsc_warning_lines = 0;
    std::istringstream iss(all_captured);
    std::string line;
    while (std::getline(iss, line)) {
      if (line.find("mix INSERT_VALUES") != std::string::npos ||
          line.find("Cannot mix") != std::string::npos ||
          line.find("assembly") != std::string::npos ||
          line.find("VecSetValues") != std::string::npos) {
        ++num_petsc_warning_lines;
      }
    }
    int num_elements = val.getRegion("RGN_ALL").size();
    EXPECT_GE(num_petsc_warning_lines, static_cast<size_t>(num_elements / 5))
        << "Expected ~" << (num_elements / 2) << " PETSc warnings (per mode switch), got "
        << num_petsc_warning_lines << ". All captured: [" << all_captured << "]";
  }

  // Always verify functionality (mixed ops work despite warnings)
  TypeParam result = vector.toField();
  BOUT_FOR(i, this->field.getRegion("RGN_NOY")) {
    if (i.ind % 2 == 0) {
      EXPECT_DOUBLE_EQ(result[i], -10.0);  // From = (INSERT)
    } else {
      EXPECT_DOUBLE_EQ(result[i], 1.5 + 5.0);  // Initial + delta from += (ADD)
    }
  }
}
#endif // BOUT_HAS_PETSC
