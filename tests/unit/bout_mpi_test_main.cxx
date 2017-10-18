#include <stdio.h>

#include "mpi_test_extras.hxx"

#include "gtest/gtest.h"
#include "boutcomm.hxx"
#include "bout/array.hxx"

#include <mpi.h>

GTEST_API_ int main(int argc, char **argv) {
  printf("Running main() from bout_mpi_test_main.cxx\n");
  testing::InitGoogleTest(&argc, argv);

  ::testing::AddGlobalTestEnvironment(new MPIEnvironment(&argc, &argv));

  int result = RUN_ALL_TESTS();

  // Clean up the array store, so valgrind doesn't report false
  // positives
  Array<double>::cleanup();

  return result;
}
