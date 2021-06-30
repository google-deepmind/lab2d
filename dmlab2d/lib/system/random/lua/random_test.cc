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
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#include "dmlab2d/lib/system/random/lua/random.h"

#include <algorithm>
#include <numeric>
#include <random>
#include <sstream>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "dmlab2d/lib/lua/bind.h"
#include "dmlab2d/lib/lua/call.h"
#include "dmlab2d/lib/lua/n_results_or_test_util.h"
#include "dmlab2d/lib/lua/push_script.h"
#include "dmlab2d/lib/lua/vm.h"
#include "dmlab2d/lib/lua/vm_test_util.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {

using ::deepmind::lab2d::lua::testing::IsOkAndHolds;
using ::testing::AllOf;
using ::testing::DoubleNear;
using ::testing::Each;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Ge;
using ::testing::Gt;
using ::testing::Lt;
using ::testing::UnorderedElementsAreArray;

class LuaRandomTest : public lua::testing::TestWithVm {
 protected:
  LuaRandomTest() {
    LuaRandom::Register(L);
    vm()->AddCModuleToSearchers(
        "system.sys_random", &lua::Bind<LuaRandom::Require>,
        {&prbg_, reinterpret_cast<void*>(static_cast<std::uintptr_t>(0))});
  }

  std::mt19937_64 prbg_;
};

constexpr absl::string_view kSeedWithNumber = R"(
local sys_random = require 'system.sys_random'
sys_random:seed(999)
)";

TEST_F(LuaRandomTest, SeedWithNumber) {
  std::ostringstream expected_state, actual_state;

  prbg_.seed(999);
  expected_state << prbg_;
  prbg_.seed(0);

  ASSERT_THAT(lua::PushScript(L, kSeedWithNumber, "kSeedWithNumber"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
  actual_state << prbg_;

  EXPECT_EQ(expected_state.str(), actual_state.str());
}

constexpr absl::string_view kSeedWithString = R"(
local sys_random = require 'system.sys_random'
sys_random:seed("888")
)";

TEST_F(LuaRandomTest, SeedWithString) {
  std::ostringstream expected_state, actual_state;

  prbg_.seed(888);
  expected_state << prbg_;

  prbg_.seed(0);

  ASSERT_THAT(lua::PushScript(L, kSeedWithString, "kSeedWithString"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
  actual_state << prbg_;

  EXPECT_EQ(expected_state.str(), actual_state.str());
}

constexpr absl::string_view kUniformInt = R"(
local sys_random = require 'system.sys_random'
local from, to = ...
local results = {}
for i = 1, 1000 do
  results[#results + 1] = sys_random:uniformInt(from, to)
end
return results
)";

TEST_F(LuaRandomTest, UniformInt) {
  ASSERT_THAT(lua::PushScript(L, kUniformInt, "kUniformInt"), IsOkAndHolds(1));
  lua::Push(L, 1);
  lua::Push(L, 3);
  ASSERT_THAT(lua::Call(L, 2), IsOkAndHolds(1));
  std::vector<int> result;
  ASSERT_TRUE(IsFound(lua::Read(L, 1, &result)));
  int sum = std::accumulate(result.begin(), result.end(), 0);
  ASSERT_THAT(result.size(), Eq(1000));
  double mean = static_cast<double>(sum) / result.size();
  EXPECT_THAT(mean, DoubleNear(2.0, 0.1));
  std::sort(result.begin(), result.end());
  result.erase(std::unique(result.begin(), result.end()), result.end());
  EXPECT_THAT(result, ElementsAre(1, 2, 3));
}

TEST_F(LuaRandomTest, UniformIntNegative) {
  ASSERT_THAT(lua::PushScript(L, kUniformInt, "kUniformInt"), IsOkAndHolds(1));
  lua::Push(L, -5);
  lua::Push(L, -3);
  ASSERT_THAT(lua::Call(L, 2), IsOkAndHolds(1));
  std::vector<int> result;
  ASSERT_TRUE(IsFound(lua::Read(L, 1, &result)));
  std::sort(result.begin(), result.end());
  result.erase(std::unique(result.begin(), result.end()), result.end());
  EXPECT_THAT(result, ElementsAre(-5, -4, -3));
}

constexpr absl::string_view kUniformReal = R"(
local sys_random = require 'system.sys_random'
local from, to = ...
local results = {}
for i = 1, 1000 do
  results[#results + 1] = sys_random:uniformReal(from, to)
end
return results
)";

TEST_F(LuaRandomTest, UniformReal) {
  ASSERT_THAT(lua::PushScript(L, kUniformReal, "kUniformReal"),
              IsOkAndHolds(1));
  prbg_.seed(0);
  lua::Push(L, 0.0);
  lua::Push(L, 1.0);
  ASSERT_THAT(lua::Call(L, 2), IsOkAndHolds(1));
  std::vector<double> result;
  ASSERT_TRUE(IsFound((lua::Read(L, 1, &result))));
  ASSERT_THAT(result.size(), Eq(1000));
  double sum = std::accumulate(result.begin(), result.end(), 0.0);
  double mean = sum / result.size();
  EXPECT_THAT(mean, DoubleNear(0.5, 0.1));
  auto half_it = result.begin() + result.size() / 2;
  std::nth_element(result.begin(), half_it, result.end());
  double sum_first = std::accumulate(result.begin(), half_it, 0.0);
  double mean_first = sum_first / std::distance(result.begin(), half_it);
  EXPECT_THAT(mean_first, DoubleNear(0.25, 0.1));
}

constexpr absl::string_view kPoisson = R"(
local sys_random = require 'system.sys_random'
local frequency = ...
local results = {}
for i = 1, 1000 do
  results[#results + 1] = sys_random:poissonDistribution(frequency)
end
return results
)";

TEST_F(LuaRandomTest, Poisson) {
  ASSERT_THAT(lua::PushScript(L, kPoisson, "kPoisson"), IsOkAndHolds(1));
  prbg_.seed(0);
  lua::Push(L, 4.0);
  ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(1));
  std::vector<int> result;
  ASSERT_TRUE(IsFound((lua::Read(L, 1, &result))));
  ASSERT_THAT(result.size(), Eq(1000));
  EXPECT_THAT(result, Each(Ge(0)));
  EXPECT_THAT(result, Each(Lt(20)));
}

constexpr absl::string_view kShuffleInPlace = R"(
local sys_random = require 'system.sys_random'
local array = ...
sys_random:shuffleInPlace(array)
return array
)";

TEST_F(LuaRandomTest, ShuffleInPlace) {
  ASSERT_THAT(lua::PushScript(L, kShuffleInPlace, "kShuffleInPlace"),
              IsOkAndHolds(1));
  prbg_.seed(0);
  std::vector<int> elements(10);
  std::iota(elements.begin(), elements.end(), 0);
  lua::Push(L, elements);
  ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(1));
  std::vector<int> results;
  ASSERT_TRUE(IsFound((lua::Read(L, 1, &results))));
  ASSERT_FALSE(std::is_sorted(results.begin(), results.end()));
  EXPECT_THAT(results, UnorderedElementsAreArray(elements));
}

TEST_F(LuaRandomTest, ShuffleInPlace3Numbers) {
  constexpr int kNumElements = 3;
  std::vector<int> elements(kNumElements);
  std::iota(elements.begin(), elements.end(), 1);
  ASSERT_THAT(lua::PushScript(L, kShuffleInPlace, "kShuffleInPlace"),
              IsOkAndHolds(1));
  absl::flat_hash_map<std::array<int, kNumElements>, int> counters;
  for (int i = 1; i < 6000; ++i) {
    prbg_.seed(i - 1);
    lua_pushvalue(L, 1);
    lua::Push(L, elements);
    ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(1));
    std::array<int, kNumElements> results;
    ASSERT_TRUE(IsFound((lua::Read(L, 2, &results))));
    ASSERT_THAT(results, UnorderedElementsAreArray(elements));
    ++counters.emplace(results, 0).first->second;
    lua_pop(L, 1);
  }

  ASSERT_THAT(counters.size(), Eq(6));
  for (const auto& element : counters) {
    EXPECT_THAT(element.second, AllOf(Gt(1000 - 100), Lt(1000 + 100)));
  }
}

constexpr absl::string_view kShuffleCopy = R"(
local sys_random = require 'system.sys_random'
local array = ...
local result = sys_random:shuffle(array)
assert(result ~= array)
return result
)";

TEST_F(LuaRandomTest, ShuffleCopy) {
  ASSERT_THAT(lua::PushScript(L, kShuffleCopy, "kShuffleCopy"),
              IsOkAndHolds(1));
  prbg_.seed(0);
  std::vector<int> elements(10);
  std::iota(elements.begin(), elements.end(), 0);
  lua::Push(L, elements);
  ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(1));
  std::vector<int> results;
  ASSERT_TRUE(IsFound((lua::Read(L, 1, &results))));
  ASSERT_FALSE(std::is_sorted(results.begin(), results.end()));
  EXPECT_THAT(results, UnorderedElementsAreArray(elements));
}

TEST_F(LuaRandomTest, ShuffleCopy3Numbers) {
  constexpr int kNumElements = 3;
  std::vector<int> elements(kNumElements);
  std::iota(elements.begin(), elements.end(), 1);
  ASSERT_THAT(lua::PushScript(L, kShuffleCopy, "kShuffleCopy"),
              IsOkAndHolds(1));
  absl::flat_hash_map<std::array<int, kNumElements>, int> counters;
  for (int i = 1; i < 6000; ++i) {
    prbg_.seed(i - 1);
    lua_pushvalue(L, 1);
    lua::Push(L, elements);
    ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(1));
    std::array<int, kNumElements> results;
    ASSERT_TRUE(IsFound((lua::Read(L, 2, &results))));
    ASSERT_THAT(results, UnorderedElementsAreArray(elements));
    ++counters.emplace(results, 0).first->second;
    lua_pop(L, 1);
  }

  ASSERT_THAT(counters.size(), Eq(6));
  for (const auto& element : counters) {
    EXPECT_THAT(element.second, AllOf(Gt(1000 - 100), Lt(1000 + 100)));
  }
}

constexpr absl::string_view kChoice = R"(
local sys_random = require 'system.sys_random'
local array = ...
return sys_random:choice(array)
)";

TEST_F(LuaRandomTest, Choice) {
  ASSERT_THAT(lua::PushScript(L, kChoice, "kChoice"), IsOkAndHolds(1));
  constexpr int kNumElements = 4;
  std::vector<int> elements(kNumElements);
  std::iota(elements.begin(), elements.end(), 0);
  std::array<int, kNumElements> histogram = {};
  for (int i = 0; i < 1000; ++i) {
    // Function gets popped in call. Push it again so it remains at top of stack
    // for next iteration.
    lua_pushvalue(L, 1);
    lua::Push(L, elements);
    ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(1));
    int result;
    ASSERT_TRUE(IsFound((lua::Read(L, 2, &result))));
    ASSERT_THAT(result, AllOf(Ge(0), Lt(kNumElements)));
    ++histogram[result];
    // Pop result.
    lua_pop(L, 1);
  }
  // Pop original function.
  lua_pop(L, 1);
  ASSERT_THAT(histogram, Each(Gt(0)));
}

TEST_F(LuaRandomTest, ChoiceNil) {
  ASSERT_THAT(lua::PushScript(L, kChoice, "kChoice"), IsOkAndHolds(1));
  lua::Push(L, std::vector<int>());
  ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(1));
  ASSERT_TRUE(lua_isnil(L, 1));
  lua_pop(L, 1);
}

}  // namespace
}  // namespace deepmind::lab2d
