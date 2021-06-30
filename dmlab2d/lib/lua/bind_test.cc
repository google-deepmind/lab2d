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

#include "dmlab2d/lib/lua/bind.h"

#include "dmlab2d/lib/lua/push.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/lua/vm_test_util.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d::lua {
namespace {

constexpr char kNegativeError[] = "Value was negative!";
constexpr char kFailedToConvertError[] = "Value was not a number";

// Returns value passed in if >=0 or an error message.
NResultsOr NonNegativeOrError(lua_State* L) {
  double value;
  if (Read(L, 1, &value)) {
    if (value >= 0) {
      return 1;
    } else {
      return kNegativeError;
    }
  }
  return kFailedToConvertError;
}

using BindTest = testing::TestWithVm;

TEST_F(BindTest, Success) {
  lua_pushcfunction(L, &Bind<NonNegativeOrError>);
  Push(L, 10.0);
  ASSERT_EQ(0, lua_pcall(L, 1, 1, 0)) << ToString(L, -1);
  double value_out;
  ASSERT_TRUE(IsFound(Read(L, -1, &value_out))) << "Result was not a double.";
  EXPECT_EQ(10.0, value_out);
}

TEST_F(BindTest, FailLessThanZero) {
  lua_pushcfunction(L, &Bind<NonNegativeOrError>);
  Push(L, -10.0);
  ASSERT_NE(0, lua_pcall(L, 1, 1, 0)) << "No error message!";
  std::string error;
  ASSERT_TRUE(IsFound(Read(L, -1, &error))) << "Error was not available.";
  EXPECT_EQ(kNegativeError, error);
}

TEST_F(BindTest, FailNotADouble) {
  lua_pushcfunction(L, &Bind<NonNegativeOrError>);
  Push(L, "-10.0");
  ASSERT_NE(0, lua_pcall(L, 1, 1, 0)) << "No error message!";
  std::string error;
  ASSERT_TRUE(IsFound(Read(L, -1, &error))) << "Error was not available.";
  EXPECT_EQ(kFailedToConvertError, error);
}

}  // namespace
}  // namespace deepmind::lab2d::lua
