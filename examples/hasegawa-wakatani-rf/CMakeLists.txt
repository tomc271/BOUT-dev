cmake_minimum_required(VERSION 3.13)

project(hasegama-wakatani-rf LANGUAGES CXX)

if (NOT TARGET bout++::bout++)
  find_package(bout++ REQUIRED)
endif()

bout_add_example(hasegama-wakatani-rf SOURCES hw.cxx)
