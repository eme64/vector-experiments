#include <iostream>

#include "vector.hpp"
#include "cli.hpp"

int main(int argc, char* argv[]) {
  std::cout << "Benchmark Vectors" << std::endl;

  Cli cli(argc, argv, "benchmark");

  cli.addFlag('d', "dump",   "Dump all available functions.");
  cli.addFlag('v', "verify", "Verify correctness of implementations.");
  cli.addOption('f', "functions", "", "Prefix of function names to run.");
  cli.addOption('w', "warmup", "100000", "Number of warmup iterations.");
  cli.addOption('i', "iterations", "100000", "Number of benchmark iterations.");
  cli.addOption('r', "repetitions", "1", "Number of measurement repetitions.");

  if (!cli.parse()) {return -1;}

  int warmup = stoi(cli.option("warmup"));
  int iterations = stoi(cli.option("iterations"));
  int repetitions = stoi(cli.option("repetitions"));

  Functions f(cli.option("functions"));
  if (cli.flag("dump")) {
    f.dump();
  }
  if (cli.flag("verify")) {
    f.verify();
  }
  f.benchmark(warmup, iterations, repetitions);
}
