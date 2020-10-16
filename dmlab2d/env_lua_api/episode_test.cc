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

#include "dmlab2d/env_lua_api/episode.h"

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
using ::deepmind::lab2d::lua::testing::StatusIs;
using ::testing::Eq;
using ::testing::HasSubstr;

using EpisodeTest = lua::testing::TestWithVm;

constexpr char kEpisodeApi[] = R"(
local api = {
  _episode = -1,
  _seed = -1,
  _step = 0,
}

function api:start(episode, seed)
  self._episode = episode
  self._seed = seed
end

function api:advance(step)
  self._step = step
  return step < 3, 2.0
end

return api
)";

TEST_F(EpisodeTest, EpisodeApi) {
  ASSERT_THAT(lua::PushScript(L, kEpisodeApi, "kEpisodeApi"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef table;
  ASSERT_TRUE(IsFound(Read(L, 1, &table)));
  lua_settop(L, 0);
  Episode episode;
  ASSERT_THAT(episode.BindApi(table), IsOkAndHolds(0));
  EXPECT_THAT(episode.Start(/*episode=*/10, /*seed=*/15), IsOkAndHolds(0));

  int episode_index = 0;
  EXPECT_TRUE(IsFound(table.LookUp("_episode", &episode_index)));
  EXPECT_THAT(episode_index, Eq(10));

  int seed = 0;
  EXPECT_TRUE(IsFound(table.LookUp("_seed", &seed)));
  EXPECT_THAT(seed, Eq(15));

  int step = 0;
  EXPECT_TRUE(IsFound(table.LookUp("_step", &step)));
  EXPECT_THAT(step, Eq(0));

  EnvCApi_EnvironmentStatus status;
  double reward = 0;

  EXPECT_THAT(episode.Advance(&status, &reward), IsOkAndHolds(0));
  EXPECT_THAT(status, Eq(EnvCApi_EnvironmentStatus_Running));
  EXPECT_THAT(reward, Eq(2.0));
  EXPECT_TRUE(IsFound(table.LookUp("_step", &step)));
  EXPECT_THAT(step, Eq(1));

  EXPECT_THAT(episode.Advance(&status, &reward), IsOkAndHolds(0));
  EXPECT_THAT(status, Eq(EnvCApi_EnvironmentStatus_Running));
  EXPECT_THAT(reward, Eq(2.0));
  EXPECT_TRUE(IsFound(table.LookUp("_step", &step)));
  EXPECT_THAT(step, Eq(2));

  EXPECT_THAT(episode.Advance(&status, &reward), IsOkAndHolds(0));
  EXPECT_THAT(status, Eq(EnvCApi_EnvironmentStatus_Terminated));
  EXPECT_THAT(reward, Eq(2.0));
  EXPECT_TRUE(IsFound(table.LookUp("_step", &step)));
  EXPECT_THAT(step, Eq(3));
}

constexpr char kEpisodeApiError[] = R"(
local api = {
  _episode = -1,
  _seed = -1,
  _step = 0,
}

function api:start(episode, seed)
  assert(seed >= 0,
      "Seed must be greater than 0; actual: " .. seed)
  self._episode = episode
  self._seed = seed
end

function api:advance(step)
  assert(step ~= 2, "Another error")
  self._step = step
  return step < 3, 2.0
end

return api
)";

TEST_F(EpisodeTest, EpisodeApiError) {
  ASSERT_THAT(lua::PushScript(L, kEpisodeApiError, "kEpisodeApiError"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef table;
  ASSERT_TRUE(IsFound(Read(L, 1, &table)));
  lua_settop(L, 0);
  Episode episode;
  ASSERT_THAT(episode.BindApi(table), IsOkAndHolds(0));
  EXPECT_THAT(episode.Start(/*episode=*/1, /*seed=*/-5),
              StatusIs(HasSubstr("Seed must be greater than 0; actual: -5")));

  EXPECT_THAT(episode.Start(/*episode=*/1, /*seed=*/5), IsOkAndHolds(0));
  EnvCApi_EnvironmentStatus status;
  double reward = 0;
  EXPECT_THAT(episode.Advance(&status, &reward), IsOkAndHolds(0));
  EXPECT_THAT(episode.Advance(&status, &reward),
              StatusIs(HasSubstr("Another error")));
}

}  // namespace
}  // namespace deepmind::lab2d
