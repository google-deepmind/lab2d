// Copyright (C) 2016-2019 The DMLab2D Authors.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//      https://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.//
// GMock matchers for NResultsOr.
//
// * IsOkAndHolds:
//
//     extern NResultsOr f();
//     EXPECT_THAT(f(), IsOkAndHolds(1));     // Return exactly one result.
//     EXPECT_THAT(f(), IsOkAndHolds(Ge(3));  // Return at least three results.

#ifndef DMLAB2D_LIB_LUA_N_RESULTS_OR_TEST_UTIL_H_
#define DMLAB2D_LIB_LUA_N_RESULTS_OR_TEST_UTIL_H_

#include <ostream>
#include <string>

#include "dmlab2d/lib/lua/n_results_or.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d::lua {

inline std::ostream& operator<<(std::ostream& os, const NResultsOr& value) {
  if (value.ok()) {
    return os << "is OK and has value " << value.n_results();
  } else {
    return os << "has error: " << value.error();
  }
}

namespace testing {
namespace internal {

class IsOkAndHoldsImpl : public ::testing::MatcherInterface<const NResultsOr&> {
 public:
  template <typename M>
  explicit IsOkAndHoldsImpl(M matcher)
      : matcher_(::testing::SafeMatcherCast<int>(matcher)) {}

 private:
  void DescribeTo(std::ostream* os) const override {
    *os << "is OK and has a value that ";
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(std::ostream* os) const override {
    *os << "isn't OK or has a value that ";
    matcher_.DescribeNegationTo(os);
  }

  bool MatchAndExplain(
      const NResultsOr& actual_value,
      ::testing::MatchResultListener* result_listener) const override {
    if (!actual_value.ok()) {
      *result_listener << "which has error: " << actual_value.error();
      return false;
    }

    ::testing::StringMatchResultListener listener;

    const bool matches =
        matcher_.MatchAndExplain(actual_value.n_results(), &listener);
    const std::string explanation = listener.str();

    if (!explanation.empty()) {
      *result_listener << "which contains value "
                       << ::testing::PrintToString(actual_value.n_results())
                       << ", " << explanation;
    }

    return matches;
  }

  const ::testing::Matcher<int> matcher_;
};

class StatusIsImpl : public ::testing::MatcherInterface<const NResultsOr&> {
 public:
  template <typename M>
  explicit StatusIsImpl(M matcher)
      : matcher_(::testing::SafeMatcherCast<std::string>(matcher)) {}

 private:
  void DescribeTo(std::ostream* os) const override {
    *os << "is in error state with error value that ";
    matcher_.DescribeTo(os);
  }

  void DescribeNegationTo(std::ostream* os) const override {
    *os << "isn't in error state or has an error value that ";
    matcher_.DescribeNegationTo(os);
  }

  bool MatchAndExplain(
      const NResultsOr& actual_value,
      ::testing::MatchResultListener* result_listener) const override {
    if (actual_value.ok()) {
      *result_listener << "which is OK (with " << actual_value.n_results()
                       << " arguments)";
      return false;
    }

    ::testing::StringMatchResultListener listener;

    const bool matches =
        matcher_.MatchAndExplain(actual_value.error(), &listener);
    const std::string explanation = listener.str();

    if (!explanation.empty()) {
      *result_listener << "which contains error value " << actual_value.error()
                       << ", " << explanation;
    }

    return matches;
  }

  const ::testing::Matcher<std::string> matcher_;
};

template <typename M>
class IsOkAndHoldsMatcher {
 public:
  explicit IsOkAndHoldsMatcher(M matcher) : matcher_(matcher) {}

  operator ::testing::Matcher<const NResultsOr&>() const {
    return ::testing::MakeMatcher(new IsOkAndHoldsImpl(matcher_));
  }

 private:
  M matcher_;
};

template <typename M>
class StatusIsMatcher {
 public:
  explicit StatusIsMatcher(M matcher) : matcher_(matcher) {}

  operator ::testing::Matcher<const NResultsOr&>() const {
    return ::testing::MakeMatcher(new StatusIsImpl(matcher_));
  }

 private:
  M matcher_;
};

}  // namespace internal

template <typename M>
internal::IsOkAndHoldsMatcher<M> IsOkAndHolds(M matcher) {
  return internal::IsOkAndHoldsMatcher<M>(matcher);
}

template <typename M>
internal::StatusIsMatcher<M> StatusIs(M matcher) {
  return internal::StatusIsMatcher<M>(matcher);
}

}  // namespace testing
}  // namespace deepmind::lab2d::lua

#endif  // DMLAB2D_LIB_LUA_N_RESULTS_OR_TEST_UTIL_H_
