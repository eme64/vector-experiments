#include "vector.hpp"

void init_random(Vectors* v) {
  v->fill_random();
}

void init_zero(Vectors* v) {
  v->fill_const(0);
}

void impl_identity_ref(Vectors* v) {
  const uint64_t limit = v->length;
  char* aa = v->a;
  char* bb = v->b;
  for (uint64_t i = 0; i < limit; i++) {
    bb[i] = aa[i];
  }
}

void impl_identity_impl_256(Vectors* v) {
  const uint64_t limit = v->length;
  char* aa = v->a;
  char* bb = v->b;
  for (uint64_t i = 0; i < limit; i+=32) {
    __m256i val = _mm256_loadu_si256((__m256i*) &aa[i]);
    _mm256_storeu_si256((__m256i*) &bb[i], val);
  }
}

void impl_add_long_ref(Vectors* v) {
  const uint64_t limit = v->length / 8;
  int64_t* aa = (int64_t*)v->a;
  int64_t* bb = (int64_t*)v->b;
  int64_t* cc = (int64_t*)v->c;
  for (uint64_t i = 0; i < limit; i++) {
    cc[i] = aa[i] + bb[i];
  }
}

void impl_add_long_impl_4(Vectors* v) {
  const uint64_t limit = v->length;
  char* aa = v->a;
  char* bb = v->b;
  char* cc = v->c;
  for (uint64_t i = 0; i < limit; i+=32) {
    __m256i val_a = _mm256_loadu_si256((__m256i*) &aa[i]);
    __m256i val_b = _mm256_loadu_si256((__m256i*) &bb[i]);
    __m256i val_c = _mm256_add_epi64(val_a, val_b);
    _mm256_storeu_si256((__m256i*) &cc[i], val_c);
  }
}

void impl_add_char_ref(Vectors* v) {
  const uint64_t limit = v->length;
  char* aa = v->a;
  char* bb = v->b;
  char* cc = v->c;
  for (uint64_t i = 0; i < limit; i++) {
    cc[i] = aa[i] + bb[i];
  }
}

void impl_upper_char_ref(Vectors* v) {
  const uint64_t limit = v->length;
  uint8_t* aa = (uint8_t*)v->a;
  uint8_t* bb = (uint8_t*)v->b;
  for (uint64_t i = 0; i < limit; i++) {
    uint8_t c = aa[i];
    if (c >= 97 && c <= 122) {
      c -= 32; // lower letters to upper letters
    }
    bb[i] = c;
  }
}

void impl_upper_char_impl_32(Vectors* v) {
  const uint64_t limit = v->length;
  char* aa = v->a;
  char* bb = v->b;
  __m256i con_m_32 = _mm256_set1_epi8(-32);
  __m256i con_97   = _mm256_set1_epi8(97);
  __m256i con_122  = _mm256_set1_epi8(122);
  for (uint64_t i = 0; i < limit; i+=32) {
    __m256i val_a   = _mm256_loadu_si256((__m256i*) &aa[i]);
    __m256i val_sub = _mm256_add_epi8(val_a, con_m_32);
    _mm256_storeu_si256((__m256i*) &bb[i], val_sub);
  }
}

Functions::Functions() {
  // identity
  add_init("identity",   "random",    init_random);
  add_init("identity",   "zero",      init_zero);
  add_impl("identity",   "ref",       impl_identity_ref);
  add_impl("identity",   "impl_256",  impl_identity_impl_256);
  // add_long
  add_init("add_long",   "random",    init_random);
  add_impl("add_long",   "ref",       impl_add_long_ref);
  add_impl("add_long",   "impl_4",    impl_add_long_impl_4);
  // add_char
  add_init("add_char",   "random",    init_random);
  add_impl("add_char",   "ref",       impl_add_char_ref);
  // upper_char
  add_init("upper_char", "random",    init_random);
  add_impl("upper_char", "ref",       impl_upper_char_ref);
  add_impl("upper_char", "impl_32",   impl_upper_char_impl_32);
}

void Functions::dump() {
  std::cout << "Functions::dump" << std::endl;
  for (auto it : _map_impl) {
    std::cout << "  " << it.first << std::endl;
    FunctionImpls &inits = _map_init[it.first];
    std::cout << "    inits:" << std::endl;
    for (auto it2 : inits) {
      std::cout << "      " << it2.first << std::endl;
    }
    std::cout << "    impls:" << std::endl;
    for (auto it2 : it.second) {
      std::cout << "      " << it2.first << std::endl;
    }
  }
}

void Functions::verify() {
  std::cout << "Functions::verify" << std::endl;
  for (auto it : _map_impl) {
    FunctionImpls &inits = _map_init[it.first];
    FunctionImpls &impls = it.second;
    for (auto it2 : inits) {
      // Initialize input:
      Vectors init;
      it2.second(&init);
      // Compute reference output:
      Vectors output_ref;
      output_ref.copy_from(&init);
      Function &ref = impls["ref"];
      ref(&output_ref);
      // Compute output for all impls:
      Vectors output;
      for (auto it3 : impls) {
        std::string name = "verify." + it.first + "." + it2.first + "." + it3.first;
        std::cout << "  " << name << std::endl;
        output.copy_from(&init);
        Function &impl = it3.second;
        impl(&output);
        output.verify_equals(name, &output_ref);
      }
    }
  }
}

void Functions::benchmark(int warmup, int iterations, int repetitions) {
  std::cout << "Functions::benchmark" << std::endl;
  Timer t;
  for (auto it : _map_impl) {
    FunctionImpls &inits = _map_init[it.first];
    FunctionImpls &impls = it.second;
    for (auto it3 : impls) {
      Function &impl = it3.second;
      for (auto it2 : inits) {
        Function &init = it3.second;
        std::string name = "benchmark." + it.first + "." + it2.first + "." + it3.first;
        // Initialize input:
        Vectors vector;
        init(&vector);
        // Warmup
        for (int i = 0; i < warmup; i++) {
          impl(&vector);
        }
        for (int j = 0; j < repetitions; j++) {
          t.start();
          for (int i = 0; i < iterations; i++) {
            impl(&vector);
          }
          t.stop();
          std::cout << "  " << name << " " << t.millisecs() << std::endl;
        }
      }
    }
  }
}

