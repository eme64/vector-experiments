#include <immintrin.h>
#include <cstdint>
#include <stdlib.h>
#include <cassert>
#include <iomanip>
#include <map>
#include <iostream>
#include <cstring>
#include <random>

#include "timer.hpp"

class Vectors {
private:
  std::random_device _rdev;
  std::default_random_engine _rng;

public:
  // Data arrays
  char* a = nullptr;
  char* b = nullptr;
  char* c = nullptr;
  char* d = nullptr;
  char* e = nullptr;
  char* f = nullptr;

  // Length of all data arrays
  uint64_t length;

  Vectors(uint64_t l = 8 * 1024) : _rng(_rdev()), length(l) {
    assert(l > 0 && l % 64 == 0 && "Vectors length must be divisible by 64");
    a = allocate();
    b = allocate();
    c = allocate();
    d = allocate();
    e = allocate();
    f = allocate();
  }

  ~Vectors() {
    free(a);
    free(b);
    free(c);
    free(d);
    free(e);
    free(f);
  }

private:
  char* allocate() {
    return (char*)aligned_alloc(64, length); // 64 byte aligned
  }

public:
  void dump() {
    dump("a", a);
    dump("b", b);
    dump("c", c);
    dump("d", d);
    dump("e", e);
    dump("f", f);
  }

private:
  void dump(const char* name, char* arr) {
    std::cout << std::setw(4) << std::left << name;
    for(uint64_t i = 0; i < length; i += 8) {
      uint64_t v = *((uint64_t*)(arr+i));
      std::cout << std::hex << std::setfill('0') << std::setw(16) << v << " ";
    }
    std::cout << std::setfill(' ') << std::endl;
  }

public:
  void copy_from(const Vectors* other) {
    assert(length == other->length && "must have same length to copy");
    std::memcpy(a, other->a, length);
    std::memcpy(b, other->b, length);
    std::memcpy(c, other->c, length);
    std::memcpy(d, other->d, length);
    std::memcpy(e, other->e, length);
    std::memcpy(f, other->f, length);
  }

  void fill_random() {
    fill_random(a);
    fill_random(b);
    fill_random(c);
    fill_random(d);
    fill_random(e);
    fill_random(f);
  }

  void fill_random(char* arr) {
    std::uniform_int_distribution<uint64_t> dist(0, 0xFFFFFFFFFFFFFFFF);
    for(uint64_t i = 0; i < length; i += 8) {
      *((uint64_t*)(arr+i)) = dist(_rng);
    }
  }

  void fill_const(uint64_t val) {
    fill_const(a, val);
    fill_const(b, val);
    fill_const(c, val);
    fill_const(d, val);
    fill_const(e, val);
    fill_const(f, val);
  }

  void fill_const(char* arr, uint64_t val) {
    std::uniform_int_distribution<uint64_t> dist(0, 0xFFFFFFFFFFFFFFFF);
    for(uint64_t i = 0; i < length; i += 8) {
      *((uint64_t*)(arr+i)) = val;
    }
  }

  void fill_str(char* arr) {
    std::uniform_int_distribution<uint64_t> ldist(length/2, length);
    uint64_t l = ldist(_rng);
    std::uniform_int_distribution<uint8_t> dist(1, 0xFF);
    for(uint64_t i = 0; i < length; i++) {
      arr[i] = (i < l) ? dist(_rng) : 0;
    }
  }

  void verify_equals(std::string name, const Vectors* other) {
    verify_equals(name + "_a", a, other->a);
    verify_equals(name + "_b", b, other->b);
    verify_equals(name + "_c", c, other->c);
    verify_equals(name + "_d", d, other->d);
    verify_equals(name + "_e", e, other->e);
    verify_equals(name + "_f", f, other->f);
  }

  void verify_equals(std::string name, char* v1, char* v2) {
    for(uint64_t i = 0; i < length; i += 8) {
      uint64_t val1 = *((uint64_t*)(v1+i));
      uint64_t val2 = *((uint64_t*)(v2+i));
      if (val1 != val2) {
        std::cout << "verify_equals failed: wrong result" << std::endl;
        std::cout << "  " << name << std::endl;
        std::cout << "  i = " << i << std::endl;
        std::cout << "  val1 = " << std::hex << std::setfill('0') << std::setw(16) << val1 << " " << std::endl;
        std::cout << "  val2 = " << std::hex << std::setfill('0') << std::setw(16) << val2 << " " << std::endl;
        assert(false && "verify_equals failed: wrong result");
      }
    }
  }
};


class Functions {
public:
  typedef void (*Function)(Vectors*);
  typedef std::map<std::string, Function> FunctionImpls;
  typedef std::map<std::string, FunctionImpls> FunctionMap;

private:
  FunctionMap _map_init;
  FunctionMap _map_impl;
  std::string _functions_prefix;

  void add_init(std::string func_name, std::string init_name, Function f) {
    if (func_name.rfind(_functions_prefix, 0) != 0) { return; }
    if (_map_init.find(func_name) == _map_init.end()) {
      _map_init[func_name] = FunctionImpls();
      _map_impl[func_name] = FunctionImpls();
    }
    _map_init[func_name][init_name] = f;
  }

  void add_impl(std::string func_name, std::string impl_name, Function f) {
    if (func_name.rfind(_functions_prefix, 0) != 0) { return; }
    if (_map_impl.find(func_name) == _map_impl.end()) {
      _map_init[func_name] = FunctionImpls();
      _map_impl[func_name] = FunctionImpls();
    }
    _map_impl[func_name][impl_name] = f;
  }

public:
  Functions(std::string functions_prefix);

  void dump();

  void verify();

  void benchmark(int warmup, int iterations, int repetitions);
};


