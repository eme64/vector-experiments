#include "vector.hpp"

void init_random(Vectors* v) {
  v->fill_random();
}

void init_zero(Vectors* v) {
  v->fill_const(0);
}

void init_str(Vectors* v) {
  v->fill_const(0);
  v->fill_str(v->a);
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

// -fno-tree-loop-if-convert
// Prevents the if-conversion, with which it is as fast as impl_32_v2
// or even a tiny bit faster.
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
  __m256i con_96   = _mm256_set1_epi8(96);
  __m256i con_123  = _mm256_set1_epi8(123);
  for (uint64_t i = 0; i < limit; i+=32) {
    __m256i val_a   = _mm256_loadu_si256((__m256i*) &aa[i]);
    __m256i val_sub = _mm256_add_epi8(val_a, con_m_32);
    __m256i cmp0 = _mm256_cmpgt_epi8(val_a, con_96); // c >= 97
    __m256i cmp1 = _mm256_cmpgt_epi8(con_123, val_a);  // c <= 122
    __m256i cmp  = _mm256_and_si256(cmp0, cmp1);
    __m256i val_b = _mm256_blendv_epi8(val_a, val_sub, cmp);
    _mm256_storeu_si256((__m256i*) &bb[i], val_b);
  }
}

void impl_upper_char_impl_32_v2(Vectors* v) {
  const uint64_t limit = v->length;
  char* aa = v->a;
  char* bb = v->b;
  __m256i con_m_32   = _mm256_set1_epi8(-32);
  __m256i con_m_97   = _mm256_set1_epi8(-97);
  __m256i con_25     = _mm256_set1_epi8(25);
  __m256i con_0      = _mm256_set1_epi8(0);
  for (uint64_t i = 0; i < limit; i+=32) {
    __m256i val_a   = _mm256_loadu_si256((__m256i*) &aa[i]);
    __m256i val_sh  = _mm256_add_epi8(val_a, con_m_97); // sh = a[i] - 97
    __m256i val_su  = _mm256_subs_epu8(val_sh, con_25); // su = sub_no_underflow(sh, 25)
    __m256i cmp     = _mm256_cmpeq_epi8(val_su, con_0); // su == 0
    __m256i val_sub = _mm256_add_epi8(val_a, con_m_32); // sh = a[i] - 32
    __m256i val_b = _mm256_blendv_epi8(val_a, val_sub, cmp);
    _mm256_storeu_si256((__m256i*) &bb[i], val_b);
  }
}

void impl_strlen_ref(Vectors* v) {
  const uint64_t limit = v->length;
  uint8_t* aa = (uint8_t*)v->a;
  uint64_t* bb = (uint64_t*)v->b;
  uint64_t i;
  for (i = 0; i < limit; i++) {
    if (aa[i] == 0) {
      break; // found termination character
    }
  }
  bb[0] = i; // i contains strlen, write to output
}

void impl_strlen_impl_32(Vectors* v) {
  const uint64_t limit = v->length;
  uint8_t* aa = (uint8_t*)v->a;
  uint64_t* bb = (uint64_t*)v->b;
  __m256i con_0      = _mm256_set1_epi8(0);
  uint64_t i;
  for (i = 0; i < limit; i += 32) {
    __m256i val_a   = _mm256_loadu_si256((__m256i*) &aa[i]);
    __m256i cmp     = _mm256_cmpeq_epi8(val_a, con_0); // c == 0
    int is_zero     = _mm256_testz_si256(cmp, cmp); // is zero
    if (!is_zero) {
      // found termination character in vector
      for (; i < limit; i++) {
        if (aa[i] == 0) {
          break;
        }
      }
      bb[0] = i; // i contains strlen, write to output
      return;
    }
  }
  bb[0] = -1; // never found it, put in impossible value
}

Functions::Functions(std::string functions_prefix)
    : _functions_prefix(functions_prefix) {
  // identity
  add_init("identity",   "random",      init_random);
  add_init("identity",   "zero",        init_zero);
  add_impl("identity",   "ref",         impl_identity_ref);
  add_impl("identity",   "impl_256",    impl_identity_impl_256);
  // add_long
  add_init("add_long",   "random",      init_random);
  add_impl("add_long",   "ref",         impl_add_long_ref);
  add_impl("add_long",   "impl_4",      impl_add_long_impl_4);
  // add_char
  add_init("add_char",   "random",      init_random);
  add_impl("add_char",   "ref",         impl_add_char_ref);
  // upper_char
  add_init("upper_char", "random",      init_random);
  add_impl("upper_char", "ref",         impl_upper_char_ref);
  add_impl("upper_char", "impl_32",     impl_upper_char_impl_32);
  add_impl("upper_char", "impl_32_v2",  impl_upper_char_impl_32_v2);
  // strlen
  add_init("strlen",     "str",         init_str);
  add_impl("strlen",     "ref",         impl_strlen_ref);
  add_impl("strlen",     "impl_32",     impl_strlen_impl_32);
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
    for (int j = 0; j < repetitions; j++) {
      for (auto it2 : inits) {
        Function &init = it2.second;
        // Initialize input:
        Vectors vector;
        init(&vector);
        for (auto it3 : impls) {
          Function &impl = it3.second;
          std::string name = "benchmark." + it.first + "." + it2.first + "." + it3.first;
          // Warmup
          for (int i = 0; i < warmup; i++) {
            impl(&vector);
          }
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

