// Copyright © Conor Williams <conorwilliams@outlook.com>

// SPDX-License-Identifier: MPL-2.0

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <catch2/benchmark/catch_benchmark.hpp> // for Benchmark, BENCHMARK
#include <catch2/catch_test_macros.hpp>         // for StringRef, TEST_CASE
#include <iostream>                             // for char_traits, basic_ostream, operator<<
#include <memory>                               // for allocator

// #define LF_DEFAULT_LOGGING

#include "libfork/core.hpp" // for task, sync_wait, worker_init, co_new

namespace {

struct noise {
  noise() { std::cout << "cons()" << std::endl; }
  ~noise() { std::cout << "dest()" << std::endl; }
};

inline constexpr auto fib = [](auto fib, int n) -> lf::task<int> {
  //
  // noise _;

  if (n < 2) {
    co_return n;
  }

  int a, b;

  co_await lf::fork(&a, fib)(n - 1);
  co_await lf::call(&b, fib)(n - 2);

  co_return a + b;
};

inline constexpr auto co_fib = [](auto co_fib, int n) -> lf::task<int> {
  if (n < 2) {
    co_return n;
  }

  auto [r] = co_await lf::co_new<int>(2);

  co_await lf::fork(&r[0], co_fib)(n - 1);
  co_await lf::call(&r[1], co_fib)(n - 2);

  co_await lf::join;

  co_return r[1] + r[0];
};

struct scheduler : lf::impl::immovable<scheduler> {

  void schedule(lf::submit_handle job) {

    context->schedule(job);

    resume(context->try_pop_all());
  }

  ~scheduler() { lf::finalize(context); }

 private:
  lf::worker_context *context = lf::worker_init(lf::nullary_function_t{[]() {}});
};

LF_NOINLINE constexpr auto sfib(int &ret, int n) -> void {
  if (n < 2) {
    ret = n;
    return;
  }

  int a, b;

  sfib(a, n - 1);
  sfib(b, n - 2);

  ret = a + b;
}

auto test(auto) -> lf::task<> { co_return {}; }

} // namespace

TEST_CASE("Benchmarks", "[core][benchmark]") {

  scheduler sch{};

  volatile int in = 20;

  BENCHMARK("Fibonacci serial") {
    int out = 0;
    sfib(out, in);
    return out;
  };

  BENCHMARK("Fibonacci parall") {
    //
    return lf::sync_wait(sch, fib, in);
  };

  BENCHMARK("Fibonacci parall co_alloc") {
    //
    return lf::sync_wait(sch, co_fib, in);
  };
}
