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

#include "dmlab2d/lib/env_lua_api/actions.h"

#include <cstddef>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "absl/log/check.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "dmlab2d/lib/lua/call.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/push.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/lua/stack_resetter.h"
#include "dmlab2d/lib/system/tensor/lua/tensor.h"

namespace deepmind::lab2d {

constexpr absl::string_view kDiscreteActionSpec = "discreteActionSpec";
constexpr absl::string_view kDiscreteActions = "discreteActions";
constexpr absl::string_view kContinuousActionSpec = "continuousActionSpec";
constexpr absl::string_view kContinuousActions = "continuousActions";
constexpr absl::string_view kTextActionSpec = "textActionSpec";
constexpr absl::string_view kTextActions = "textActions";

namespace {

template <typename T, typename ActionInfo>
lua::NResultsOr ReadActionSpec(absl::string_view action_spec_name,
                               lua::TableRef* script_table_ref,
                               std::vector<ActionInfo>* infos) {
  lua_State* L = script_table_ref->LuaState();
  lua::StackResetter stack_resetter(L);
  script_table_ref->PushMemberFunction(action_spec_name);
  if (lua_isnil(L, -2)) {
    return 0;
  }
  auto result = lua::Call(L, 1);
  if (!result.ok()) {
    return result;
  }
  lua::TableRef actions;
  if (!IsFound(lua::Read(L, -1, &actions))) {
    return absl::StrCat("[", action_spec_name,
                        "] - Must return a action spec table.");
  }
  auto action_count = actions.ArraySize();
  infos->clear();
  infos->reserve(action_count);
  for (std::size_t i = 0, c = action_count; i != c; ++i) {
    lua::TableRef info;
    if (!IsFound(actions.LookUp(i + 1, &info))) {
      return absl::StrCat("[", action_spec_name,
                          "] - Missing table argument.\n");
    }
    ActionInfo action_info;
    if (!info.LookUp("name", &action_info.name)) {
      return absl::StrCat("[", action_spec_name,
                          "] - Missing 'name = <string>'.\n");
    }
    if (!IsFound(info.LookUp("min", &action_info.min_value))) {
      return absl::StrCat("[", action_spec_name,
                          "] - Missing 'min = <number>'.\n");
    }

    if (!IsFound(info.LookUp("max", &action_info.max_value))) {
      return absl::StrCat("[", action_spec_name,
                          "] - Missing 'max = <number>'.\n");
    }
    infos->push_back(std::move(action_info));
  }
  return 0;
}

lua::NResultsOr ReadTextActionSpec(absl::string_view action_spec_name,
                                   lua::TableRef* script_table_ref,
                                   std::vector<std::string>* text_actions) {
  lua_State* L = script_table_ref->LuaState();
  lua::StackResetter stack_resetter(L);
  script_table_ref->PushMemberFunction(action_spec_name);
  if (lua_isnil(L, -2)) {
    return 0;
  }
  auto result = lua::Call(L, 1);
  if (!result.ok()) {
    return result;
  }

  if (!IsFound(lua::Read(L, -1, text_actions))) {
    return absl::StrCat("[", action_spec_name,
                        "] - Must return an array of text action names.");
  }
  return 0;
}

template <typename T, typename ActionInfo>
void ActionApply(absl::string_view action_name, const T* actions,
                 const std::vector<ActionInfo>& infos,
                 lua::TableRef* script_table_ref) {
  if (infos.empty()) {
    return;
  }
  lua_State* L = script_table_ref->LuaState();
  lua::StackResetter stack_resetter(L);
  script_table_ref->PushMemberFunction(action_name);
  // Function must exist.
  CHECK(!lua_isnil(L, -2)) << "[" << action_name << "] - API function missing";
  lua::Push(L, absl::MakeConstSpan(actions, infos.size()));
  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << "[" << action_name << "] - " << result.error();
}

void PushTextActions(lua_State* L,
                     absl::Span<const EnvCApi_TextAction> values) {
  lua_createtable(L, values.size(), 0);
  for (std::size_t i = 0; i < values.size(); ++i) {
    lua::Push(L, i + 1);
    lua_pushlstring(L, values[i].data, values[i].len);
    lua_settable(L, -3);
  }
}

}  // namespace

lua::NResultsOr Actions::BindApi(lua::TableRef script_table_ref) {
  script_table_ref_ = std::move(script_table_ref);
  if (auto result = ReadActionSpec<int>(kDiscreteActionSpec, &script_table_ref_,
                                        &discrete_action_infos_);
      !result.ok()) {
    return result;
  }

  if (auto result = ReadActionSpec<double>(
          kContinuousActionSpec, &script_table_ref_, &continuous_action_infos_);
      !result.ok()) {
    return result;
  }

  return ReadTextActionSpec(kTextActionSpec, &script_table_ref_,
                            &text_actions_);
}

void Actions::DiscreteApply(const int* actions) {
  ActionApply(kDiscreteActions, actions, discrete_action_infos_,
              &script_table_ref_);
}

void Actions::DiscreteBounds(int idx, int* min_value_out,
                             int* max_value_out) const {
  const auto& info = discrete_action_infos_[idx];
  *min_value_out = info.min_value;
  *max_value_out = info.max_value;
}

void Actions::ContinuousApply(const double* actions) {
  ActionApply(kContinuousActions, actions, continuous_action_infos_,
              &script_table_ref_);
}

void Actions::ContinuousBounds(int idx, double* min_value_out,
                               double* max_value_out) const {
  const auto& info = continuous_action_infos_[idx];
  *min_value_out = info.min_value;
  *max_value_out = info.max_value;
}

void Actions::TextApply(const EnvCApi_TextAction* actions) {
  lua_State* L = script_table_ref_.LuaState();
  lua::StackResetter stack_resetter(L);
  script_table_ref_.PushMemberFunction(kTextActions);
  // Function must exist.
  CHECK(!lua_isnil(L, -2)) << "[" << kTextActions << "] - API function missing";
  PushTextActions(L, absl::MakeConstSpan(actions, text_actions_.size()));
  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << "[" << kTextActions << "] - " << result.error();
}

}  // namespace deepmind::lab2d
