#include "bout/build_defines.hxx"

#include <utility>

#include "gtest/gtest.h"

#include "bout/field2d.hxx"
#include "bout/field3d.hxx"
#include "bout/fieldperp.hxx"
#include "bout/operatorstencil.hxx"
#include "bout/petsc_interface.hxx"
#include "bout/region.hxx"
#include <sstream>

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
  WithQuietOutput all{output};
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

TYPED_TEST(PetscVectorTest, ReproducesNoisyPETScWarnings) #include <petscsys.h>  // For PetscGetVersion

 // TYPED_TEST to conditionally reproduce noise based on PETSc version
    TYPED_TEST(PetscVectorTest, ReproducesNoisyPETScWarnings) {
  SCOPED_TRACE("ReproducesNoisyPETScWarnings");

  // Get PETSc version to conditionally enable noise checks
  char petsc_version_str[256];
  PetscErrorCode ierr = PetscGetVersion(petsc_version_str, PETSC_VERSION_RELEASE);
  ASSERT_EQ(ierr, 0);
  int major, minor, patch;
  sscanf(petsc_version_str, "%d.%d.%d", &major, &minor, &patch);
  int version_num = major * 10000 + minor * 100 + patch;

  const int MIN_VERSION_FOR_WARNINGS = 31800;  // 3.18.0; adjust if exact version known

  if (version_num < MIN_VERSION_FOR_WARNINGS) {
    GTEST_SKIP() << "PETSc version " << petsc_version_str
                 << " (< 3.18) does not produce assembly warnings; skipping noise checks.";
  }

  // Temporarily restore default PETSc error printing to allow warnings to be visible
  auto old_error_printf = PetscErrorPrintf;
  PetscErrorPrintf = PetscErrorPrintfDefault;

  // Capture stderr to verify noisy output
  std::streambuf* old_cerr_buf = std::cerr.rdbuf();
  std::stringstream capture_stream;
  std::cerr.rdbuf(capture_stream.rdbuf());

  // Reproduce the issue: Create vector from field (assembles initially), then perform
  // element-wise sets via proxy. Each vector(index) creates an Element, calling
  // VecGetValues (get) during potential re-assembly phase, mixing with VecSetValues (set)
  // in operator=, triggering one warning per element in recent PETSc versions.
  PetscVector<TypeParam> vector(this->field, this->indexer);
  const TypeParam val(-10.);

  // Loop over all indices to trigger multiple warnings (noisy behavior)
  BOUT_FOR(index, val.getRegion("RGN_ALL")) {
    vector(index) = val[index];
  }
  vector.assemble();  // Finalizes, but warnings occur during the sets/gets above

  // Restore stderr and PETSc printing
  std::cerr.rdbuf(old_cerr_buf);
  PetscErrorPrintf = old_error_printf;

  // Verify noisy behavior: Captured output should not be empty (contains warnings)
  std::string captured_output = capture_stream.str();
  EXPECT_FALSE(captured_output.empty())
      << "Expected noisy PETSc warning messages in stderr, but captured empty output.";

  // More specific: Check for relevant warning keywords (e.g., "VecGetValues" or "assembly")
  EXPECT_NE(captured_output.find("VecGetValues"), std::string::npos)
      << "Expected warnings mentioning VecGetValues or assembly state.";

  // Number of warnings roughly matches number of elements (one per element)
  // Count lines containing warning keywords for better accuracy
  size_t num_warning_lines = 0;
  std::istringstream iss(captured_output);
  std::string line;
  while (std::getline(iss, line)) {
    if (line.find("VecGetValues") != std::string::npos || line.find("assembly") != std::string::npos) {
      ++num_warning_lines;
    }
  }
  int num_elements = this->field.getRegion("RGN_ALL").size();
  // Loose tolerance: At least half the elements should trigger a warning line
  EXPECT_GE(num_warning_lines, static_cast<size_t>(num_elements / 2))
      << "Expected ~" << num_elements << " warnings (one per element), got " << num_warning_lines
      << " relevant lines. Captured: " << captured_output;

  // Always verify functional correctness (values set properly despite warnings)
  TypeParam result = vector.toField();
  BOUT_FOR(i, this->field.getRegion("RGN_NOY")) {
    EXPECT_DOUBLE_EQ(result[i], val[i]);
  }
}
#endif // BOUT_HAS_PETSC
