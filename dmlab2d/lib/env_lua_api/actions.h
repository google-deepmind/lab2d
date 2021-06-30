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

#ifndef DMLAB2D_LIB_ENV_LUA_API_ACTIONS_H_
#define DMLAB2D_LIB_ENV_LUA_API_ACTIONS_H_

#include <memory>
#include <string>
#include <vector>

#include "dmlab2d/lib/lua/lua.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/table_ref.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind::lab2d {

class Actions {
 public:
  // Reads the custom action spec from the table passed in.
  // Keeps a reference to the table for further calls.
  lua::NResultsOr BindApi(lua::TableRef script_table_ref);

  // Discrete action count.
  int DiscreteCount() const { return discrete_action_infos_.size(); }

  // Discrete action name. `idx` shall be in `[0, DiscreteCount())`.
  const char* DiscreteName(int idx) const {
    return discrete_action_infos_[idx].name.c_str();
  }

  //  Discrete action spec. `idx` shall be in `[0, DiscreteCount())`.
  void DiscreteBounds(int idx, int* min_value_out, int* max_value_out) const;

  // Calls api discrete action function with an array `DiscreteCount()`
  // integers.
  void DiscreteApply(const int* actions);

  // Continuous action count.
  int ContinuousCount() const { return continuous_action_infos_.size(); }

  // Continuous action name. `idx` shall be in `[0, ContinuousCount())`.
  const char* ContinuousName(int idx) const {
    return continuous_action_infos_[idx].name.c_str();
  }

  // Continuous action spec. `idx` shall be in [0, ContinuousCount()).
  void ContinuousBounds(int idx, double* min_value_out,
                        double* max_value_out) const;

  // Calls api discrete action function with an array `DiscreteCount()`
  // doubles.
  void ContinuousApply(const double* actions);

  // Continuous action count.
  int TextCount() const { return text_actions_.size(); }

  // Text action name. `idx` shall be in `[0, TextCount())`.
  const char* TextName(int idx) const { return text_actions_[idx].c_str(); }

  // Calls api discrete action function with an array `DiscreteCount()` doubles.
  void TextApply(const EnvCApi_TextAction* actions);

 private:
  // Entry for a custom action spec.
  template <typename T>
  struct ActionInfo {
    std::string name;
    T min_value;
    T max_value;
  };

  lua::TableRef script_table_ref_;

  // Storage of discrete action types from script.
  std::vector<ActionInfo<int>> discrete_action_infos_;

  // Storage of discrete action types from script.
  std::vector<ActionInfo<double>> continuous_action_infos_;

  // Storage of discrete action types from script.
  std::vector<std::string> text_actions_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_ENV_LUA_API_ACTIONS_H_
