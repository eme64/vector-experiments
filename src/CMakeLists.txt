cmake_minimum_required(VERSION 3.6)

# ---------------------- benchmark
project(benchmark)

add_executable(benchmark benchmark.cpp)

set_target_properties(benchmark PROPERTIES
   CXX_STANDARD 11
   CXX_STANDARD_REQUIRED YES
   CXX_EXTENSIONS NO
)

