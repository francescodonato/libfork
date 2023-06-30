#include <chrono>
#include <iostream>
#include <thread>
#include <vector>

#include "../bench.hpp"

#include "libfork/core.hpp"
#include "libfork/schedule/busy.hpp"

using namespace std;
using namespace lf;

inline constexpr async_fn dfs = [](auto dfs, size_t depth, size_t breadth, unsigned long *sum) -> task<void> {
  if (depth == 0) {
    *sum = 1;
    co_return;
  }

  vector<unsigned long> sums(breadth);

  for (size_t i = 0; i < breadth - 1; ++i) {
    co_await lf::fork(dfs)(depth - 1, breadth, &sums[i]);
  }
  co_await lf::call(dfs)(depth - 1, breadth, &sums.back());

  co_await join;

  *sum = 0;
  for (size_t i = 0; i < breadth; ++i) {
    *sum += sums[i];
  }
};

void run(std::string name, size_t depth = 8, size_t breadth = 8) {
  benchmark(name, [&](std::size_t num_threads, auto &&bench) {
    // Set up
    unsigned long answer;

    auto pool = lf::busy_pool{num_threads};

    bench([&] {
      unsigned long tmp = 0;

      lf::sync_wait(pool, dfs, depth, breadth, &tmp);

      answer = tmp;
    });

    return answer;
  });
}

int main(int argc, char *argv[]) {
  run("fork-dfs-3,3", 3, 3);
  run("fork-dfs-5,5", 5, 5);
  run("fork-dfs-6,6", 5, 6);
  run("fork-dfs-7,7", 7, 7);
  return 0;
}