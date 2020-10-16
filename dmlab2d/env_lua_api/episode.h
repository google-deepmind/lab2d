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

#ifndef DMLAB2D_ENV_LUA_API_EPISODE_H_
#define DMLAB2D_ENV_LUA_API_EPISODE_H_

#include <utility>

#include "dmlab2d/lua/n_results_or.h"
#include "dmlab2d/lua/table_ref.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind::lab2d {

class Episode {
 public:
  // Binds episode control to the Lua API.
  // Keeps a reference to the table for further calls.
  lua::NResultsOr BindApi(lua::TableRef script_table_ref) {
    script_table_ref_ = std::move(script_table_ref);
    return 0;
  }
  // Calls "start" member function on the script_table_ref_ with episode and
  // seed.
  lua::NResultsOr Start(int episode, int seed);

  // Calls "advance" member function on the script_table_ref_ with currnet step.
  // Advance the episode `number_of_steps`. Calculates the accumulated reward.
  lua::NResultsOr Advance(EnvCApi_EnvironmentStatus* status, double* reward);

 private:
  lua::TableRef script_table_ref_;
  std::size_t num_elapsed_frames_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_ENV_LUA_API_EPISODE_H_
