#include "gtest/gtest.h"

#include "boutcomm.hxx"

TEST(BoutCommTest, Get) {
  auto instance1 = BoutComm::get();
  auto instance2 = BoutComm::get();

  EXPECT_EQ(instance1, instance2);
}

TEST(BoutCommTest, Rank) {
  int rank;
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);

  EXPECT_EQ(BoutComm::rank(), rank);
}

TEST(BoutCommTest, Size) {
  int size;
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  EXPECT_EQ(BoutComm::size(), size);
}
