#include <benchmark/benchmark.h>

#include "short_code.hpp"

void ShortCodeBenchmark(benchmark::State& state) {
  for (auto _ : state) {
    auto code = linkshrink::utils::GenerateShortCode(8);
    benchmark::DoNotOptimize(code);
  }
}

BENCHMARK(ShortCodeBenchmark);