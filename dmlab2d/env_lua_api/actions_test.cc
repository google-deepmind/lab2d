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
#include "dmlab2d/env_lua_api/actions.h"

#include <array>

#include "dmlab2d/lua/call.h"
#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/n_results_or_test_util.h"
#include "dmlab2d/lua/push_script.h"
#include "dmlab2d/lua/table_ref.h"
#include "dmlab2d/lua/vm_test_util.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind::lab2d {
namespace {

using ::deepmind::lab2d::lua::testing::IsOkAndHolds;
using ::testing::Eq;
using ::testing::StrEq;

using ActionsTest = lua::testing::TestWithVm;

constexpr char kDiscreteSpec[] = R"(
return {
    discreteActionSpec = function(_)
      return {
          { name = '1.ACT', min = -1, max = 1 },
          { name = '2.ACT', min = -2, max = 2 },
          { name = '3.ACT', min = -3, max = 3 },
      }
    end,
    discreteActions = function(_, actions)
      assert(#actions == 3)
      for i, val in ipairs(actions) do
        assert(actions[i] == i)
      end
    end
})";

TEST_F(ActionsTest, ReadDiscreteSpec) {
  ASSERT_THAT(lua::PushScript(L, kDiscreteSpec, "kDiscreteSpec"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef table;
  ASSERT_TRUE(IsFound(Read(L, 1, &table)));
  lua_settop(L, 0);
  Actions actions;
  ASSERT_THAT(actions.BindApi(table), IsOkAndHolds(0));

  ASSERT_THAT(actions.DiscreteCount(), Eq(3));
  EXPECT_THAT(actions.DiscreteName(0), StrEq("1.ACT"));
  EXPECT_THAT(actions.DiscreteName(1), StrEq("2.ACT"));
  EXPECT_THAT(actions.DiscreteName(2), StrEq("3.ACT"));
  int min_value, max_value;
  actions.DiscreteBounds(0, &min_value, &max_value);
  EXPECT_THAT(min_value, Eq(-1));
  EXPECT_THAT(max_value, Eq(1));

  actions.DiscreteBounds(1, &min_value, &max_value);
  EXPECT_THAT(min_value, Eq(-2));
  EXPECT_THAT(max_value, Eq(2));

  actions.DiscreteBounds(2, &min_value, &max_value);
  EXPECT_THAT(min_value, Eq(-3));
  EXPECT_THAT(max_value, Eq(3));
  std::array<int, 3> values = {1, 2, 3};
  actions.DiscreteApply(values.data());
}

constexpr char kContinuousSpec[] = R"(
return {
    continuousActionSpec = function(_)
      return {
          { name = '1.ACT', min = -1, max = 1 },
          { name = '2.ACT', min = -2, max = 2 },
          { name = '3.ACT', min = -3, max = 3 },
      }
    end,
    continuousActions = function(_, actions)
      assert(#actions == 3)
      for i, val in ipairs(actions) do
        assert(actions[i] == i)
      end
    end
})";

TEST_F(ActionsTest, ReadContinuousSpec) {
  ASSERT_THAT(lua::PushScript(L, kContinuousSpec, "kContinuousSpec"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef table;
  ASSERT_TRUE(IsFound(Read(L, 1, &table)));
  lua_settop(L, 0);
  Actions actions;
  ASSERT_THAT(actions.BindApi(table), IsOkAndHolds(0));

  ASSERT_THAT(actions.ContinuousCount(), Eq(3));
  EXPECT_THAT(actions.ContinuousName(0), StrEq("1.ACT"));
  EXPECT_THAT(actions.ContinuousName(1), StrEq("2.ACT"));
  EXPECT_THAT(actions.ContinuousName(2), StrEq("3.ACT"));
  double min_value, max_value;
  actions.ContinuousBounds(0, &min_value, &max_value);
  EXPECT_THAT(min_value, Eq(-1));
  EXPECT_THAT(max_value, Eq(1));

  actions.ContinuousBounds(1, &min_value, &max_value);
  EXPECT_THAT(min_value, Eq(-2));
  EXPECT_THAT(max_value, Eq(2));

  actions.ContinuousBounds(2, &min_value, &max_value);
  EXPECT_THAT(min_value, Eq(-3));
  EXPECT_THAT(max_value, Eq(3));
  std::array<double, 3> values = {1, 2, 3};
  actions.ContinuousApply(values.data());
}

constexpr char kTextSpec[] = R"(
return {
    textActionSpec = function(_)
      return {
          '1.ACT',
          '2.ACT',
          '3.ACT',
      }
    end,
    textActions = function(_, actions)
      for i, ac in ipairs(actions) do
        print(i .. ' ' .. ac)
      end
      io.flush()
      assert(#actions == 3, '#actions == 3')
      assert(actions[1] == '0', actions[1])
      assert(actions[2] == '11', actions[2])
      assert(actions[3] == '222', actions[3])
    end
})";

TEST_F(ActionsTest, ReadTextSpec) {
  ASSERT_THAT(lua::PushScript(L, kTextSpec, "kTextSpec"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef table;
  ASSERT_TRUE(IsFound(Read(L, 1, &table)));
  lua_settop(L, 0);
  Actions actions;
  ASSERT_THAT(actions.BindApi(table), IsOkAndHolds(0));
  ASSERT_THAT(actions.TextCount(), Eq(3));
  EXPECT_THAT(actions.TextName(0), StrEq("1.ACT"));
  EXPECT_THAT(actions.TextName(1), StrEq("2.ACT"));
  EXPECT_THAT(actions.TextName(2), StrEq("3.ACT"));
  std::array<EnvCApi_TextAction, 3> values = {
      EnvCApi_TextAction{"0", std::strlen("0")},
      EnvCApi_TextAction{"11", std::strlen("11")},
      EnvCApi_TextAction{"222", std::strlen("222")},
  };
  actions.TextApply(values.data());
}

}  // namespace
}  // namespace deepmind::lab2d
