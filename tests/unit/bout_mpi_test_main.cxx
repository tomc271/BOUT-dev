#include <stdio.h>

#include "mpi-printer.h"

#include "gtest/gtest.h"
#include "boutcomm.hxx"
#include "bout/array.hxx"

GTEST_API_ int main(int argc, char **argv) {
  printf("Running main() from bout_test_main.cxx\n");
  testing::InitGoogleTest(&argc, argv);
  
  MPI_Init(&argc, &argv);
  gmx::test::initMPIOutput();
  // // Initialise MPI
  // BoutComm::setArgs(argc, argv);

  // auto comm = BoutComm::get();

  // int rank;
  // MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  // // Only print from rank 0
  // ::testing::TestEventListeners& listeners =
  //     ::testing::UnitTest::GetInstance()->listeners();
  // if (rank != 0) {
  //   delete listeners.Release(listeners.default_result_printer());
  // }  

  int result = RUN_ALL_TESTS();

  // Clean up the array store, so valgrind doesn't report false
  // positives
  Array<double>::cleanup();

  // Finalise MPI
  MPI_Finalize();
  // BoutComm::cleanup();
  return result;
}
