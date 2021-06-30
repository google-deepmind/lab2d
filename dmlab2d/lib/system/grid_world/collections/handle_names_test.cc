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

#include "dmlab2d/lib/system/grid_world/collections/handle_names.h"

#include <iterator>
#include <vector>

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

TEST(HandleNames, ToHandleWorks) {
  HandleNames<TestHandle> handle_names({
      "Name0",
      "Name1",
      "Name2",
  });
  EXPECT_THAT(handle_names.ToHandle("Name0"), Eq(TestHandle(0)));
  EXPECT_THAT(handle_names.ToHandle("Name1"), Eq(TestHandle(1)));
  EXPECT_THAT(handle_names.ToHandle("Name2"), Eq(TestHandle(2)));
  EXPECT_THAT(handle_names.ToHandle("Missing"), Eq(TestHandle()));
}

TEST(HandleNames, ToHandlesWorks) {
  HandleNames<TestHandle> handle_names({
      "Cat0",
      "Bat1",
      "Rat2",
  });
  EXPECT_THAT(handle_names.ToHandles({"Rat2", "Cat0", "Rat2"}),
              ElementsAre(TestHandle(0), TestHandle(2)));
  EXPECT_THAT(handle_names.ToHandles({"Rat2", "Missing", ""}),
              ElementsAre(TestHandle(2)));
}

TEST(HandleNames, NumElementsWorks) {
  HandleNames<TestHandle> handle_names({
      "Cat0",
      "Bat1",
      "Rat2",
  });
  EXPECT_THAT(handle_names.NumElements(), Eq(3));
}

TEST(HandleNames, ForWorks) {
  std::vector<std::string> names = {
      "Cat0",
      "Bat1",
      "Rat2",
  };
  HandleNames<TestHandle> handle_names(names);
  int i = 0;
  for (auto [handle, name] : handle_names) {
    EXPECT_THAT(handle, Eq(TestHandle(i)));
    EXPECT_THAT(name, Eq(names[i]));
    ++i;
  }
}

TEST(HandleNames, ToNameWorks) {
  HandleNames<TestHandle> handle_names({
      "Cat0",
      "Bat1",
      "Rat2",
  });
  EXPECT_THAT(handle_names.ToName(TestHandle(0)), Eq("Cat0"));
  EXPECT_THAT(handle_names.ToName(TestHandle(1)), Eq("Bat1"));
  EXPECT_THAT(handle_names.ToName(TestHandle(2)), Eq("Rat2"));
}

TEST(HandleNames, NamesWorks) {
  HandleNames<TestHandle> handle_names({
      "Cat0",
      "Bat1",
      "Rat2",
  });
  EXPECT_THAT(handle_names.Names(), ElementsAre("Cat0", "Bat1", "Rat2"));
}

}  // namespace
}  // namespace deepmind::lab2d
