#include <nanobench.h>

#include "libfork/core.hpp"

#include "libfork/schedule/busy_pool.hpp"
#include "libfork/schedule/unit_pool.hpp"

// NOLINTBEGIN

LF_NOINLINE auto fib_returns(int n) -> int {
  if (n < 2) {
    return n;
  }

  return fib_returns(n - 1) + fib_returns(n - 2);
}

LF_NOINLINE auto fib_ref_help(int &ret, int n) -> void {
  if (n < 2) {
    ret = n;

  } else {
    int a, b;

    fib_ref_help(a, n - 1);
    fib_ref_help(b, n - 2);

    ret = a + b;
  }
}

LF_NOINLINE auto fib_ref(int n) -> int {
  int ret;
  fib_ref_help(ret, n);
  return ret;
}

inline constexpr lf::async fib = [](auto fib, int n) LF_STATIC_CALL -> lf::task<int, "fib"> {

  if (n < 2) {
    co_return n;
  }

  int a, b;

  co_await lf::fork(a, fib)(n - 1);
  co_await lf::call(b, fib)(n - 2);

  co_await lf::join;

  co_return a + b;
};

// inline constexpr auto c_fib_invoke = ASYNC(int n) -> lf::task<int> {
//   if (n < 2) {
//     co_return n;
//   }

//   co_return co_await self(n - 1) + co_await self(n - 2);
// };

auto main() -> int {
  //
  ankerl::nanobench::Bench bench;

  bench.title("Fibonacci");
  bench.warmup(100);
  bench.relative(true);
  bench.performanceCounters(true);
  // bench.minEpochIterations(10);

  volatile int in = 30;

  for (std::size_t i = 1; i <= std::thread::hardware_concurrency(); ++i) {

    lf::busy_pool sch{i};

    bench.run("async busy pool n=" + std::to_string(i), [&] {
      ankerl::nanobench::doNotOptimizeAway(lf::sync_wait(sch, fib, in));
    });
  }

  bench.run("ref inline", [&] {
    ankerl::nanobench::doNotOptimizeAway(fib_ref(in));
  });

  bench.run("ret inline", [&] {
    ankerl::nanobench::doNotOptimizeAway(fib_returns(in));
  });

  return 0;
}
