#include <iostream>

#include "vector.hpp"

int main(int argc, char* argv[]) {
  std::cout << "Benchmark Vectors" << std::endl;

  Functions f;
  f.dump();

  Vectors v;
  v.dump();
  v.fill_random();
  v.dump();
  Vectors v2;
  v2.dump();
  v.verify_equals("v_v2", &v2);
  v2.copy_from(&v);
  v2.dump();
}
