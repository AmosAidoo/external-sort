cmake_minimum_required(VERSION 3.15..3.31)
project(ExternalSort VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_subdirectory(tests)

add_library(ExternalSort STATIC 
  src/sort.cpp
  include/sort.h
)
target_include_directories(ExternalSort PUBLIC include)