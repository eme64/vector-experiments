#include "vector.hpp"

void func_identity_ref(Vectors* v) {
  const uint64_t limit = v->length;
  char* aa = v->a;
  char* bb = v->b;
  for (uint64_t i = 0; i < limit; i++) {
    bb[i] = aa[i];
  }
}

void func_identity_impl0(Vectors* v) {
  const uint64_t limit = v->length;
  char* aa = v->a;
  char* bb = v->b;
  for (uint64_t i = 0; i < limit; i+=32) {
    __m256i val = _mm256_loadu_si256((__m256i*) &aa[i]);
    _mm256_storeu_si256((__m256i*) &bb[i], val);
  }
}

Functions::Functions() {
  // identity
  add_function("identity", "ref",   func_identity_ref);
  add_function("identity", "impl0", func_identity_ref);
}

void Functions::dump() {
  std::cout << "Functions::dump" << std::endl;
  for (auto it : _map) {
    std::cout << "  " << it.first << std::endl;
    for (auto it2 : it.second) {
      std::cout << "    " << it2.first << std::endl;
    }
  }
}

