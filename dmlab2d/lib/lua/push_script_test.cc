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

#include "dmlab2d/lib/lua/push_script.h"

#include "dmlab2d/lib/lua/n_results_or_test_util.h"
#include "dmlab2d/lib/lua/push.h"
#include "dmlab2d/lib/lua/vm_test_util.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d::lua {
namespace {

using ::deepmind::lab2d::lua::testing::IsOkAndHolds;
using ::deepmind::lab2d::lua::testing::StatusIs;
using ::testing::AllOf;
using ::testing::HasSubstr;

using PushScriptTest = testing::TestWithVm;

constexpr char kTestGoodScript[] = R"(
local args = {...}
local function TestFunction(is_success)
  assert(is_success, "Random Error Message!")
end
TestFunction(args[1])
return "Success"
)";

TEST_F(PushScriptTest, GoodScript) {
  EXPECT_THAT(PushScript(L, kTestGoodScript, "kTestGoodScript"),
              IsOkAndHolds(1));
}

constexpr char kTestBadScript[] = R"(
local function TestFunction(is_success)
  -- Assert is missing trailing bracket.
  assert(is_success, "Random Error Message!"
end
TestFunction(args[1])
return "Success"
)";

TEST_F(PushScriptTest, BadScript) {
  EXPECT_THAT(PushScript(L, kTestBadScript, "kTestBadScript"),
              StatusIs(AllOf(HasSubstr("')' expected (to close '('"),
                             HasSubstr("kTestBadScript"))));
}

TEST_F(PushScriptTest, FileMissing) {
  const std::string filename = "Error";
  EXPECT_THAT(PushScriptFile(L, filename), StatusIs(HasSubstr("open")));
}
}  // namespace
}  // namespace deepmind::lab2d::lua
