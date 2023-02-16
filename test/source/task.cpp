// Copyright © Conor Williams <conorwilliams@outlook.com>

// SPDX-License-Identifier: MPL-2.0

// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at https://mozilla.org/MPL/2.0/.

#include <numeric>
#include <span>

#include <catch2/benchmark/catch_benchmark.hpp>
#include <catch2/catch_test_macros.hpp>

// #define NLOG
// #define NDEBUG

#include "libfork/inline.hpp"
#include "libfork/task.hpp"
#include "libfork/utility.hpp"

// NOLINTBEGIN No need to check the tests for style.

using namespace lf;

task<int, inline_context> fib(int x) {
  //
  if (x < 2) {
    co_return x;
  }

  future<int> a, b;

  co_await fork(a, fib, x - 1);
  co_await just(b, fib, x - 2);

  co_await join();

  co_return a + b;
}

// template <context Stack, typename T>
// task<T, Stack> reduce(std::span<T> range, std::size_t grain) {
//   if (range.size() <= grain) {
//     co_return std::reduce(range.begin(), range.end());
//   }

//   future<T> a, b;

//   auto head = range.size() / 2;
//   auto tail = range.size() - head;

//   co_await fork(a, reduce<Stack>(range.first(head), grain));
//   co_await just(b, reduce<Stack>(range.last(tail), grain));

//   co_await join;

//   co_return a + b;
// }

task<int, inline_context> fwd(int value) {
  co_return value;
}

TEST_CASE("Basic task manipulation", "[task]") {
  //

  inline_context context{};

  REQUIRE(sync_wait(context, fib(0)) == 0);
  REQUIRE(sync_wait(context, fib(1)) == 1);
  REQUIRE(sync_wait(context, fib(2)) == 1);
  REQUIRE(sync_wait(context, fib(3)) == 2);
  REQUIRE(sync_wait(context, fib(4)) == 3);
  REQUIRE(sync_wait(context, fib(5)) == 5);

  // context.submit_task();

  // task<int, inline_context>::promise_type::ptr = task<int, inline_context>::promise_type::buf;

  // std::vector<int> range(8);

  // for (int count = 1; auto& elem : range) {
  //   elem = count++;
  // }

  // {
  //   auto task = reduce<inline_context, int>(range, 1);

  //   future<int> result;

  //   task.m_promise->set_result_ptr(result);
  //   task.m_promise->m_stack = &context;
  //   task.m_promise->m_this.resume();

  //   REQUIRE(result == std::reduce(range.begin(), range.end()));
  // }

  // BENCHMARK("task based reduce") {
  //   auto task = reduce<inline_context, int>(range, 1);

  //   future<int> result;

  //   task.m_promise->set_result_ptr(result);
  //   task.m_promise->m_stack = &context;
  //   task.m_promise->m_this.resume();

  //   return result;
  // };

  // BENCHMARK("std::reduce") {
  //   return std::reduce(range.begin(), range.end());
  // };

  REQUIRE(false);
}

// NOLINTEND