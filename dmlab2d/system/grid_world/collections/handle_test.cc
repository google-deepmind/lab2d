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

#include "dmlab2d/system/grid_world/collections/handle.h"

#include "absl/hash/hash_testing.h"
#include "absl/strings/str_format.h"
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

TEST(HandleTest, ValueWorks) {
  TestHandle handle(5);
  EXPECT_THAT(handle.Value(), Eq(5));
}

TEST(HandleTest, IsEmptyWorks) {
  TestHandle handle(5);
  EXPECT_THAT(handle.IsEmpty(), Eq(false));
  TestHandle empty_handle;
  EXPECT_THAT(empty_handle.IsEmpty(), Eq(true));
}

TEST(HandleTest, ToStream) {
  TestHandle handle_empty;
  EXPECT_THAT(absl::StrFormat("%s", absl::FormatStreamed(handle_empty)),
              Eq("TestHandle(<empty>)"));
  TestHandle handle_5(5);
  EXPECT_THAT(absl::StrFormat("%s", absl::FormatStreamed(handle_5)),
              Eq("TestHandle(5)"));
}

TEST(HandleTest, HandlesAreComparable) {
  TestHandle handle5(5), handle4(4);
  EXPECT_THAT(handle4 == handle4, Eq(true));
  EXPECT_THAT(handle4 <= handle4, Eq(true));
  EXPECT_THAT(handle4 >= handle4, Eq(true));
  EXPECT_THAT(handle4 < handle4, Eq(false));
  EXPECT_THAT(handle4 > handle4, Eq(false));
  EXPECT_THAT(handle4 == handle5, Eq(false));
  EXPECT_THAT(handle4 <= handle5, Eq(true));
  EXPECT_THAT(handle4 >= handle5, Eq(false));
  EXPECT_THAT(handle4 < handle5, Eq(true));
  EXPECT_THAT(handle4 > handle5, Eq(false));
  EXPECT_THAT(handle5 == handle4, Eq(false));
  EXPECT_THAT(handle5 <= handle4, Eq(false));
  EXPECT_THAT(handle5 >= handle4, Eq(true));
  EXPECT_THAT(handle5 < handle4, Eq(false));
  EXPECT_THAT(handle5 > handle4, Eq(true));
}

TEST(HandleTest, HandlesAreSortable) {
  TestHandle handles[] = {TestHandle(5), TestHandle(2), TestHandle(4)};
  std::sort(std::begin(handles), std::end(handles));
  EXPECT_THAT(handles,
              ElementsAre(TestHandle(2), TestHandle(4), TestHandle(5)));
}

TEST(HandleTest, HandlesImplementsAbslHashCorrectly) {
  std::vector<TestHandle> handles;
  handles.reserve(20);
  handles.emplace_back();
  for (int i = 0; i < 20; ++i) {
    if (i == 10) {
      handles.emplace_back();
    }
    handles.emplace_back(i);
  }

  EXPECT_TRUE(absl::VerifyTypeImplementsAbslHashCorrectly(handles));
}

}  // namespace
}  // namespace deepmind::lab2d
