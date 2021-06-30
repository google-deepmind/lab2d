// Copyright (C) 2017-2019 The DMLab2D Authors.
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

#ifndef DMLAB2D_LIB_ENV_LUA_API_OBSERVATIONS_H_
#define DMLAB2D_LIB_ENV_LUA_API_OBSERVATIONS_H_

#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "dmlab2d/lib/lua/lua.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/table_ref.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind::lab2d {

class Observations {
 public:
  // Reads the observation spec from the table passed in.
  // Keeps a reference to the table for further calls.
  lua::NResultsOr BindApi(lua::TableRef script_table_ref);

  // Script observation count.
  int Count() const { return infos_.size(); }

  // Script observation name. `idx` shall be in [0, Count()).
  const char* Name(int idx) const { return infos_[idx].name.c_str(); }

  // Script observation spec. `idx` shall be in [0, Count()).
  void Spec(int idx, EnvCApi_ObservationSpec* spec) const;

  // Script observation. `idx` shall be in [0, Count()).
  void Observation(int idx, EnvCApi_Observation* observation);

 private:
  struct SpecInfo {
    std::string name;
    EnvCApi_ObservationType type;
    std::vector<int> shape;
  };

  lua::TableRef script_table_ref_;

  // Storage of supplementary observation types from script.
  std::vector<SpecInfo> infos_;

  // Used to hold the EnvCApi_ObservationSpec::shape values until the next call
  // of observation.
  std::vector<int> tensor_shape_;

  // Used to hold a reference to the observation tensor until the next call of
  // observation.
  lua::TableRef tensor_;

  // Used to store the observation until the next call of observation.
  std::string string_;
  double double_;
  std::int64_t int64_;
  std::int32_t int32_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_ENV_LUA_API_OBSERVATIONS_H_
