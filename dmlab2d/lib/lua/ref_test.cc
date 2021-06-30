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

#include "dmlab2d/lib/lua/ref.h"

#include <array>
#include <string>

#include "absl/strings/string_view.h"
#include "dmlab2d/lib/lua/n_results_or_test_util.h"
#include "dmlab2d/lib/lua/push.h"
#include "dmlab2d/lib/lua/push_script.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/lua/vm_test_util.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d::lua {
namespace {

using RefTest = testing::TestWithVm;
using ::deepmind::lab2d::lua::testing::IsOkAndHolds;
using ::testing::ElementsAre;

TEST_F(RefTest, TestToValue) {
  Ref ref;
  {
    Push(L, "Hello");
    ASSERT_TRUE(IsFound(Read(L, 1, &ref)));
    lua_settop(L, 0);
  }
  std::string value;
  ASSERT_TRUE(IsFound(ref.ToValue(&value)));
  EXPECT_EQ(value, "Hello");
}

TEST_F(RefTest, TestFriendRead) {
  Ref ref;
  {
    Push(L, "Hello");
    ASSERT_TRUE(IsFound(Read(L, 1, &ref)));
    lua_settop(L, 0);
  }
  Push(L, ref);
  std::string value;
  ASSERT_TRUE(IsFound(Read(L, 1, &value)));
  EXPECT_EQ(value, "Hello");
}

TEST_F(RefTest, TestFriendPush) {
  Ref ref = Ref::Create(L, "Hello");
  Push(L, ref);
  std::string value;
  ASSERT_TRUE(IsFound(Read(L, 1, &value)));
  EXPECT_EQ(value, "Hello");
  lua_settop(L, 0);
}

TEST_F(RefTest, TestEqual) {
  Ref ref0 = Ref::Create(L, "Hello");
  EXPECT_EQ(ref0, ref0);
  Ref ref1 = Ref::Create(L, "Hello");
  EXPECT_EQ(ref0, ref1);
  Ref ref2 = ref1;
  EXPECT_EQ(ref0, ref2);
  Ref ref3 = std::move(ref1);
  EXPECT_EQ(ref0, ref3);
  ref0 = Ref::Create(L, "World");
  ref1 = Ref::Create(L, "World");
  EXPECT_EQ(ref0, ref1);
  Ref empty0;
  Ref empty1;
  EXPECT_EQ(empty0, empty1);
}

TEST_F(RefTest, TestNotEqual) {
  Ref ref0 = Ref::Create(L, "Hello");
  Ref ref1 = Ref::Create(L, "World");
  EXPECT_NE(ref0, ref1);
  Ref ref2;
  EXPECT_NE(ref0, ref2);
}

TEST_F(RefTest, TestNotFound) {
  Ref ref;
  EXPECT_TRUE(IsNotFound(Read(L, 1, &ref)));
  lua_pushnil(L);
  EXPECT_TRUE(IsNotFound(Read(L, 1, &ref)));
  lua_settop(L, 0);
}

constexpr absl::string_view kCanCall = R"(
return ...
)";

TEST_F(RefTest, CanCallEmptyArgs) {
  PushScript(L, kCanCall, "kCanCall");
  Ref ref;
  ASSERT_TRUE(IsFound(Read(L, 1, &ref)));
  lua_settop(L, 0);
  EXPECT_THAT(ref.Call(), IsOkAndHolds(0));
}

TEST_F(RefTest, CanCallThreeArgs) {
  PushScript(L, kCanCall, "kCanCall");
  Ref ref;
  ASSERT_TRUE(IsFound(Read(L, 1, &ref)));
  lua_settop(L, 0);
  EXPECT_THAT(ref.Call(1, "two", std::array<int, 3>{1, 2, 3}), IsOkAndHolds(3));
  int val1;
  ASSERT_TRUE(IsFound(Read(L, 1, &val1)));
  EXPECT_EQ(val1, 1);
  absl::string_view val2;

  ASSERT_TRUE(IsFound(Read(L, 2, &val2)));
  EXPECT_EQ(val2, "two");
  std::array<int, 3> val3;
  ASSERT_TRUE(IsFound(Read(L, 3, &val3)));
  EXPECT_THAT(val3, ElementsAre(1, 2, 3));
}

}  // namespace
}  // namespace deepmind::lab2d::lua
