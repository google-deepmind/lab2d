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

#include "dmlab2d/lib/env_lua_api/episode.h"

#include "dmlab2d/lib/lua/call.h"
#include "dmlab2d/lib/lua/lua.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/push.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/lua/stack_resetter.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind::lab2d {

lua::NResultsOr Episode::Start(int episode, int seed) {
  num_elapsed_frames_ = 0;
  lua_State* L = script_table_ref_.LuaState();
  lua_getglobal(L, "collectgarbage");
  lua_pushvalue(L, -1);
  lua_call(L, 0, 0);
  lua_call(L, 0, 0);
  lua::StackResetter stack_resetter(L);
  script_table_ref_.PushMemberFunction("start");
  if (!lua_isnil(L, -2)) {
    lua::Push(L, episode);
    lua::Push(L, seed);
    auto result = lua::Call(L, 3);
    if (!result.ok()) {
      return result;
    }
  }
  return 0;
}

lua::NResultsOr Episode::Advance(EnvCApi_EnvironmentStatus* status,
                                 double* reward) {
  num_elapsed_frames_ += 1;
  lua_State* L = script_table_ref_.LuaState();
  lua::StackResetter stack_resetter(L);
  script_table_ref_.PushMemberFunction("advance");
  if (lua_isnil(L, -2)) {
    *status = EnvCApi_EnvironmentStatus_Terminated;
    return 0;
  }
  lua::Push(L, num_elapsed_frames_);
  auto result = lua::Call(L, 2);
  if (!result.ok()) {
    return absl::StrCat("[advance] - ", result.error());
  }

  bool continue_episode = false;
  *reward = 0;
  if (!IsFound(lua::Read(L, 1, &continue_episode)) ||
      IsTypeMismatch(lua::Read(L, 2, reward))) {
    return "[advance] - Expect boolean return value of whether the episode has "
           "ended, and an optional number value for the reward.";
  }

  *status = continue_episode ? EnvCApi_EnvironmentStatus_Running
                             : EnvCApi_EnvironmentStatus_Terminated;
  return 0;
}

}  // namespace deepmind::lab2d
