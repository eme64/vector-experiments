#include <iostream>

#include "vector.hpp"

int main(int argc, char* argv[]) {
  std::cout << "Benchmark Vectors" << std::endl;

  Functions f;
  f.dump();
  f.verify();
  f.benchmark(10e5, 10e5, 2);
}
