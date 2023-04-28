cmake_minimum_required(VERSION 3.6)

# ----------------------- vector
set(VECTOR_SOURCE_FILES
   vector.cpp
)
set(VECTOR_HEADER_FILES
   vector.hpp
)
add_library(vector ${VECTOR_SOURCE_FILES} ${VECTOR_HEADER_FILES})

set_target_properties(vector PROPERTIES
   CXX_STANDARD 11
   CXX_STANDARD_REQUIRED YES
   CXX_EXTENSIONS NO
   COMPILE_FLAGS "-Wall -march=native -mavx -mfma -O3"
)
# ---------------------- benchmark
project(benchmark)

add_executable(benchmark benchmark.cpp)

target_link_libraries(benchmark PUBLIC vector)

set_target_properties(benchmark PROPERTIES
   CXX_STANDARD 11
   CXX_STANDARD_REQUIRED YES
   CXX_EXTENSIONS NO
   COMPILE_FLAGS "-Wall -march=native -mavx -mfma -O3"
)

