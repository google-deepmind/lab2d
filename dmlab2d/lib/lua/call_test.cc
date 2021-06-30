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

#include "dmlab2d/lib/lua/call.h"

#include <string>

#include "dmlab2d/lib/lua/bind.h"
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
using ::testing::AllOf;
using ::testing::HasSubstr;
using ::testing::Not;

constexpr char kTestAssert[] = R"(
local args = {...}
local function TestFunction(is_success)
  assert(is_success, "Random Error Message!")
end
TestFunction(args[1])
return "Success"
)";

using CallTest = testing::TestWithVm;

TEST_F(CallTest, CallsFunction) {
  int top = lua_gettop(L);

  NResultsOr n_or = PushScript(L, kTestAssert, "kTestAssert");
  ASSERT_THAT(n_or, IsOkAndHolds(lua_gettop(L) - top));

  Push(L, true);

  n_or = Call(L, 1);
  ASSERT_THAT(n_or, IsOkAndHolds(lua_gettop(L) - top));

  std::string result;
  ASSERT_TRUE(IsFound(Read(L, -1, &result)));
  EXPECT_EQ(result, "Success");
}

TEST_F(CallTest, FunctionErrors) {
  int top = lua_gettop(L);

  NResultsOr n_or = PushScript(L, kTestAssert, "kTestAssert");
  ASSERT_THAT(n_or, IsOkAndHolds(lua_gettop(L) - top));

  Push(L, false);

  EXPECT_THAT(Call(L, 1), StatusIs(AllOf(HasSubstr("Random Error Message!"),
                                         HasSubstr("TestFunction"),
                                         HasSubstr("kTestAssert"))));

  EXPECT_EQ(lua_gettop(L), top);
}

TEST_F(CallTest, FunctionErrorsNoStack) {
  int top = lua_gettop(L);

  NResultsOr n_or = PushScript(L, kTestAssert, "kTestAssert");
  ASSERT_THAT(n_or, IsOkAndHolds(lua_gettop(L) - top));

  Push(L, false);

  EXPECT_THAT(Call(L, 1, false),
              StatusIs(AllOf(HasSubstr("Random Error Message!"),
                             Not(HasSubstr("TestFunction")))));

  EXPECT_EQ(lua_gettop(L), top);
}

NResultsOr TestCFuntion(lua_State* L) {
  bool should_be_success = false;
  if (IsTypeMismatch(Read(L, 1, &should_be_success))) {
    return "Type missmatch!";
  }
  Push(L, "What happens?");
  if (should_be_success) {
    return 1;
  }
  return "My error message!";
}

TEST_F(CallTest, FunctionBindErrors) {
  int top = lua_gettop(L);

  lua_pushcfunction(L, &Bind<TestCFuntion>);
  EXPECT_EQ(lua_gettop(L), top + 1);
  Push(L, false);

  auto call_result = Call(L, 1);
  EXPECT_THAT(call_result, StatusIs(HasSubstr("My error message!")));

  EXPECT_EQ(lua_gettop(L), top + call_result.n_results());
}

TEST_F(CallTest, FunctionBindSuccess) {
  int top = lua_gettop(L);

  lua_pushcfunction(L, &Bind<TestCFuntion>);
  EXPECT_EQ(lua_gettop(L), top + 1);
  Push(L, true);

  auto call_result = Call(L, 1);
  ASSERT_THAT(call_result, IsOkAndHolds(1));

  std::string message;
  ASSERT_TRUE(IsFound(Read(L, -1, &message)));
  EXPECT_EQ("What happens?", message);

  EXPECT_EQ(lua_gettop(L), top + call_result.n_results());
}

}  // namespace
}  // namespace deepmind::lab2d::lua
