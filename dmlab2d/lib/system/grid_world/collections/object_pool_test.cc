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

#include "dmlab2d/lib/system/grid_world/collections/object_pool.h"

#include "dmlab2d/lib/system/grid_world/collections/handle.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {

using ::testing::Eq;

struct TestHandleTag {
  static constexpr char kName[] = "TestHandle";
};
using TestHandle = Handle<struct TestHandleTag>;

TEST(ObjectPoolTest, CreateWorks) {
  ObjectPool<TestHandle, int> int_pool;
  auto ten = int_pool.Create(10);
  auto twenty = int_pool.Create(20);
  auto thirty = int_pool.Create(30);
  EXPECT_THAT(ten.Value(), Eq(0));
  EXPECT_THAT(twenty.Value(), Eq(1));
  EXPECT_THAT(thirty.Value(), Eq(2));
}

TEST(ObjectPoolTest, LookUpWorks) {
  ObjectPool<TestHandle, int> int_pool;
  auto ten = int_pool.Create(10);
  auto twenty = int_pool.Create(20);
  auto thirty = int_pool.Create(30);
  int_pool[ten] = 15;
  int_pool[twenty] = 25;
  int_pool[thirty] = 35;
  EXPECT_THAT(int_pool[ten], Eq(15));
  EXPECT_THAT(int_pool[twenty], Eq(25));
  EXPECT_THAT(int_pool[thirty], Eq(35));
}

TEST(ObjectPoolTest, ConstLookUpWorks) {
  ObjectPool<TestHandle, int> int_pool;
  auto ten = int_pool.Create(10);
  auto twenty = int_pool.Create(20);
  auto thirty = int_pool.Create(30);
  const auto& read_only_int_pool = int_pool;
  EXPECT_THAT(read_only_int_pool[ten], Eq(10));
  EXPECT_THAT(read_only_int_pool[twenty], Eq(20));
  EXPECT_THAT(read_only_int_pool[thirty], Eq(30));
}

TEST(ObjectPoolTest, ReleaseWorks) {
  ObjectPool<TestHandle, int> int_pool;
  auto ten = int_pool.Create(10);
  auto twenty = int_pool.Create(20);
  auto thirty = int_pool.Create(30);
  int_pool.Release(twenty);
  auto twenty_two = int_pool.Create(22);
  EXPECT_THAT(int_pool[twenty_two], Eq(22));
  EXPECT_THAT(twenty_two.Value(), Eq(1));
  int_pool.Release(twenty_two);
  int_pool.Release(ten);
  int_pool.Release(thirty);
}

TEST(ObjectPoolTest, RemoveAllElementsWorks) {
  ObjectPool<TestHandle, int> int_pool;
  auto ten = int_pool.Create(10);
  auto twenty = int_pool.Create(20);
  auto thirty = int_pool.Create(30);
  int_pool.Release(thirty);
  int_pool.Release(twenty);
  int_pool.Release(ten);
  ten = int_pool.Create(10);
  twenty = int_pool.Create(20);
  thirty = int_pool.Create(30);
  EXPECT_THAT(ten.Value(), Eq(0));
  EXPECT_THAT(twenty.Value(), Eq(1));
  EXPECT_THAT(thirty.Value(), Eq(2));
  int_pool.Release(thirty);
}

TEST(ObjectPoolTest, PreconditionsAreCheckedInDebugWithHole) {
  ObjectPool<TestHandle, int> int_pool;
  auto ten = int_pool.Create(10);
  auto twenty = int_pool.Create(20);
  const auto& read_only_int_pool = int_pool;
  EXPECT_THAT(read_only_int_pool[ten], Eq(10));
  int_pool.Release(ten);
#ifndef NDEBUG
  EXPECT_DEATH(int_pool[ten], "Attempting to use released handle!");
  EXPECT_DEATH(read_only_int_pool[ten], "Attempting to use released handle!");
  EXPECT_DEATH(int_pool.Release(ten), "Object removed twice!");
#endif
  EXPECT_THAT(read_only_int_pool[twenty], Eq(20));
}

TEST(ObjectPoolTest, PreconditionsAreCheckedInDebugEmpty) {
  ObjectPool<TestHandle, int> int_pool;
  auto ten = int_pool.Create(10);
  const auto& read_only_int_pool = int_pool;
  EXPECT_THAT(read_only_int_pool[ten], Eq(10));
  int_pool.Release(ten);
#ifndef NDEBUG
  EXPECT_DEATH(int_pool[ten], "Attempting to use released handle!");
  EXPECT_DEATH(read_only_int_pool[ten], "Attempting to use released handle!");
  EXPECT_DEATH(int_pool.Release(ten), "Object removed twice!");
#endif
}

}  // namespace
}  // namespace deepmind::lab2d
