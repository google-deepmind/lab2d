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

#include "dmlab2d/lib/system/grid_world/collections/shuffled_set.h"

#include <array>
#include <cmath>
#include <limits>
#include <numeric>
#include <random>
#include <vector>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {

using ::testing::AllOf;
using ::testing::AnyOf;
using ::testing::Each;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::IsEmpty;
using ::testing::IsNull;
using ::testing::Lt;
using ::testing::Not;
using ::testing::Pointee;
using ::testing::UnorderedElementsAre;

TEST(ShuffledSetTest, CanInsert) {
  ShuffledSet<int> set;
  EXPECT_THAT(set.IsEmpty(), Eq(true));
  set.Insert(1);
  EXPECT_THAT(set.NumElements(), Eq(1));
  EXPECT_THAT(set.IsEmpty(), Eq(false));
  set.Insert(2);
  EXPECT_THAT(set.NumElements(), Eq(2));
}

TEST(ShuffledSetTest, CanRemove) {
  ShuffledSet<int> set;
  set.Insert(1);
  set.Insert(2);
  set.Insert(3);
  EXPECT_THAT(set.NumElements(), Eq(3));
  set.Erase(2);
  EXPECT_THAT(set.NumElements(), Eq(2));
  set.Erase(1);
  EXPECT_THAT(set.NumElements(), Eq(1));
  set.Erase(3);
  EXPECT_THAT(set.IsEmpty(), Eq(true));
}

TEST(ShuffledSetTest, CanShuffle) {
  std::mt19937_64 random;
  ShuffledSet<int> set;
  set.Insert(1);
  set.Insert(2);
  set.Insert(3);
  EXPECT_THAT(set.ShuffledElements(&random), UnorderedElementsAre(1, 2, 3));
  set.Erase(2);

  EXPECT_THAT(set.ShuffledElements(&random), UnorderedElementsAre(1, 3));
  set.Erase(1);
  set.Erase(3);

  EXPECT_THAT(set.ShuffledElements(&random), IsEmpty());
}

TEST(ShuffledSetTest, CanRandomlySelect) {
  std::mt19937_64 random;
  ShuffledSet<int> set;
  set.Insert(1);
  set.Insert(2);
  set.Insert(3);
  EXPECT_THAT(set.RandomElement(&random), AnyOf(Eq(1), Eq(2), Eq(3)));
  set.Erase(2);
  EXPECT_THAT(set.RandomElement(&random), AnyOf(Eq(1), Eq(3)));
}

TEST(ShuffledSetTest, CanShuffleWithMaxCount) {
  std::mt19937_64 random;
  ShuffledSet<int> set;
  set.Insert(1);
  set.Insert(2);
  set.Insert(3);
  EXPECT_THAT(set.ShuffledElementsWithMaxCount(&random, 0), IsEmpty());

  EXPECT_THAT(set.ShuffledElementsWithMaxCount(&random, 1),
              AnyOf(ElementsAre(1), ElementsAre(2), ElementsAre(3)));

  EXPECT_THAT(set.ShuffledElementsWithMaxCount(&random, 2),
              AnyOf(UnorderedElementsAre(1, 2), UnorderedElementsAre(1, 3),
                    UnorderedElementsAre(2, 3)));
  EXPECT_THAT(set.ShuffledElementsWithMaxCount(&random, 3),
              UnorderedElementsAre(1, 2, 3));
}

TEST(ShuffledSetTest, CanShuffleWithMaxCountFullDist) {
  std::mt19937_64 random;
  ShuffledSet<int> set;
  set.Insert(1);
  set.Insert(2);
  set.Insert(3);
  set.Insert(4);
  set.Insert(5);
  set.Insert(6);
  EXPECT_THAT(set.ShuffledElementsWithMaxCount(&random, 3),
              Not(UnorderedElementsAre(1, 2, 3)));
}

TEST(ShuffledSetTest, CanShuffleWithProbability) {
  std::mt19937_64 random;
  ShuffledSet<int> set;
  set.Insert(1);
  set.Insert(2);
  set.Insert(3);
  EXPECT_THAT(set.ShuffledElementsWithProbability(&random, 0.0), IsEmpty());
  EXPECT_THAT(set.ShuffledElementsWithProbability(&random, 1.0),
              UnorderedElementsAre(1, 2, 3));
  std::array<int, 4> counter_num_elements = {};
  std::array<int, 3> counter_num_occurances = {};
  constexpr int kNumSamples = 1000;
  constexpr double probability = 0.5;
  for (int i = 0; i < kNumSamples; ++i) {
    auto result = set.ShuffledElementsWithProbability(&random, probability);
    switch (result.size()) {
      case 0:
        break;
      case 1:
        ASSERT_THAT(result[0], AnyOf(Eq(1), Eq(2), Eq(3)));
        break;
      case 2:
        ASSERT_THAT(result, AnyOf(UnorderedElementsAre(1, 2),
                                  UnorderedElementsAre(1, 3),
                                  UnorderedElementsAre(2, 3)));
        break;
      case 3:
        ASSERT_THAT(result, UnorderedElementsAre(1, 2, 3));
        break;
      default:
        FAIL() << "Too many elements: " << result.size();
    }
    counter_num_elements[result.size()]++;
    for (int i : result) {
      counter_num_occurances[i - 1]++;
    }
  }
  EXPECT_THAT(counter_num_elements, Each(Gt(0)));
  EXPECT_THAT(counter_num_elements[1], Gt(counter_num_elements[0]));
  EXPECT_THAT(counter_num_elements[2], Gt(counter_num_elements[3]));
  int expected = set.NumElements() * kNumSamples * probability;
  int actual = std::accumulate(counter_num_occurances.begin(),
                               counter_num_occurances.end(), 0);
  int error = static_cast<int>(4 * std::sqrt(static_cast<double>(expected)));
  EXPECT_THAT(actual, AllOf(Gt(expected - error), Lt(expected + error)));
}

TEST(ShuffledSetTest, CanShuffleWithProbabilityOutOfRange) {
  std::mt19937_64 random;
  ShuffledSet<int> set;
  set.Insert(1);
  set.Insert(2);
  set.Insert(3);
  EXPECT_THAT(set.ShuffledElementsWithProbability(&random, -0.5), IsEmpty());
  EXPECT_THAT(set.ShuffledElementsWithProbability(&random, 1.5),
              UnorderedElementsAre(1, 2, 3));
}

TEST(ShuffledSetTest, CanShuffledElementsFind) {
  std::mt19937_64 random;
  ShuffledSet<int> set;
  set.Insert(1);
  set.Insert(2);
  set.Insert(3);
  set.Insert(4);
  set.Insert(5);
  set.Insert(6);

  std::vector<int> elements_seen;
  EXPECT_THAT(set.ShuffledElementsFind(&random,
                                       [&elements_seen](int val) {
                                         elements_seen.push_back(val);
                                         return false;
                                       }),
              IsNull());
  EXPECT_THAT(elements_seen, UnorderedElementsAre(1, 2, 3, 4, 5, 6));
  EXPECT_THAT(elements_seen, Not(ElementsAre(1, 2, 3, 4, 5, 6)));
  EXPECT_THAT(
      set.ShuffledElementsFind(&random, [](int val) { return val < 3; }),
      Pointee(Lt(3)));
  EXPECT_THAT(
      set.ShuffledElementsFind(&random, [](int val) { return val > 3; }),
      Pointee(Gt(3)));
}

}  // namespace deepmind::lab2d
