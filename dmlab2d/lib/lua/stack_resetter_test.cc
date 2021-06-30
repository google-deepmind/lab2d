// Copyright (C) 2018-2019 The DMLab2D Authors.
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

#include "dmlab2d/lib/lua/stack_resetter.h"

#include <string>

#include "dmlab2d/lib/lua/call.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/n_results_or_test_util.h"
#include "dmlab2d/lib/lua/push.h"
#include "dmlab2d/lib/lua/push_script.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/lua/vm_test_util.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d::lua {
namespace {

using ::deepmind::lab2d::lua::testing::IsOkAndHolds;
using ::deepmind::lab2d::lua::testing::StatusIs;
using ::testing::HasSubstr;

using StackResetterTest = testing::TestWithVm;

TEST_F(StackResetterTest, Reset) {
  ASSERT_EQ(lua_gettop(L), 0);
  {
    StackResetter stack_resetter(L);
    Push(L, 10);
    {
      StackResetter stack_resetter(L);
      Push(L, 20);
      EXPECT_EQ(lua_gettop(L), 2);
    }
    EXPECT_EQ(lua_gettop(L), 1);
  }
  EXPECT_EQ(lua_gettop(L), 0);
}

constexpr const char kFunctionScript[] = R"(
local arg1 = ...
if not arg1 then
  error("No arg1")
end
return arg1
)";

TEST_F(StackResetterTest, ResetOverError) {
  ASSERT_EQ(lua_gettop(L), 0);
  {
    StackResetter stack_resetter(L);
    EXPECT_THAT(PushScript(L, kFunctionScript, "kFunctionScript"),
                IsOkAndHolds(1));
    EXPECT_THAT(Call(L, 0), StatusIs(HasSubstr("No arg1")));
  }
  EXPECT_EQ(lua_gettop(L), 0);
}

TEST_F(StackResetterTest, ResetWithReturnValues) {
  ASSERT_EQ(lua_gettop(L), 0);
  {
    StackResetter stack_resetter(L);
    EXPECT_THAT(PushScript(L, kFunctionScript, "kFunctionScript"),
                IsOkAndHolds(1));
    Push(L, 10);
    EXPECT_THAT(Call(L, 1), IsOkAndHolds(1));
    int value = 0;
    EXPECT_TRUE(IsFound(Read(L, 1, &value)));
    EXPECT_EQ(value, 10);
    EXPECT_EQ(lua_gettop(L), 1);
  }
  EXPECT_EQ(lua_gettop(L), 0);
}

}  // namespace
}  // namespace deepmind::lab2d::lua
