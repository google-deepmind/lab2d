// Copyright (C) 2019 The DMLab2D Authors.
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

#include "dmlab2d/lib/system/grid_world/collections/fixed_handle_map.h"

#include <utility>

#include "dmlab2d/lib/system/grid_world/collections/handle.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {

using ::testing::ElementsAre;
using ::testing::Eq;

struct TestHandleTag {
  static constexpr char kName[] = "TestHandle";
};
using TestHandle = Handle<TestHandleTag>;

TEST(FixedHandleMapTest, SizeWorks) {
  std::vector<int> input_values_empty;
  FixedHandleMap<TestHandle, int> values_empty(input_values_empty);
  const auto& const_values_empty = values_empty;
  EXPECT_THAT(const_values_empty.size(), Eq(0));
  EXPECT_THAT(const_values_empty.empty(), Eq(true));

  std::vector<int> input_values = {0, 3, 5};
  FixedHandleMap<TestHandle, int> values(std::move(input_values));
  const auto& const_values = values;
  EXPECT_THAT(const_values.size(), Eq(3));
  EXPECT_THAT(const_values.empty(), Eq(false));
}

TEST(FixedHandleMapTest, CanLookup) {
  std::vector<int> input_values = {0, 3, 5};
  FixedHandleMap<TestHandle, int> values(std::move(input_values));
  EXPECT_THAT(values[TestHandle(2)], Eq(5));
}

TEST(FixedHandleMapTest, CanLookupConst) {
  std::vector<int> input_values = {0, 3, 5};
  FixedHandleMap<TestHandle, int> values(std::move(input_values));
  const auto& const_values = values;
  EXPECT_THAT(const_values[TestHandle(1)], Eq(3));
}

TEST(FixedHandleMapTest, CanAssign) {
  FixedHandleMap<TestHandle, int> values(3);
  EXPECT_THAT(values[TestHandle(0)], Eq(0));
  values[TestHandle(0)] = 15;
  EXPECT_THAT(values[TestHandle(0)], Eq(15));
}

TEST(FixedHandleMapTest, CanIterate) {
  std::vector<int> input_values = {0, 3, 5};
  FixedHandleMap<TestHandle, int> values(std::move(input_values));

  std::vector<int> forwards(values.begin(), values.end());
  EXPECT_THAT(forwards, ElementsAre(0, 3, 5));

  std::vector<int> reverse(values.rbegin(), values.rend());
  EXPECT_THAT(reverse, ElementsAre(5, 3, 0));

  std::vector<int> const_forwards(values.cbegin(), values.cend());
  EXPECT_THAT(const_forwards, ElementsAre(0, 3, 5));

  std::vector<int> const_reverse(values.crbegin(), values.crend());
  EXPECT_THAT(const_reverse, ElementsAre(5, 3, 0));

  const auto& const_values = values;
  std::vector<int> const_v_forwards(const_values.begin(), const_values.end());
  EXPECT_THAT(const_v_forwards, ElementsAre(0, 3, 5));

  std::vector<int> const_v_reverse(const_values.rbegin(), const_values.rend());
  EXPECT_THAT(const_v_reverse, ElementsAre(5, 3, 0));
}

}  // namespace
}  // namespace deepmind::lab2d
