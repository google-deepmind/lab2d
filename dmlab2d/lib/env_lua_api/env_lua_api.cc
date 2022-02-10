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

#include "dmlab2d/lib/env_lua_api/env_lua_api.h"

#include <cstdint>
#include <istream>
#include <string>
#include <utility>

#include "absl/strings/match.h"
#include "absl/strings/numbers.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "dmlab2d/lib/env_lua_api/properties.h"
#include "dmlab2d/lib/lua/bind.h"
#include "dmlab2d/lib/lua/call.h"
#include "dmlab2d/lib/lua/lua.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/push.h"
#include "dmlab2d/lib/lua/push_script.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/lua/stack_resetter.h"
#include "dmlab2d/lib/lua/vm.h"
#include "dmlab2d/lib/system/image/lua_image.h"
#include "dmlab2d/lib/system/random/lua/random.h"
#include "dmlab2d/lib/system/tensor/lua/tensor.h"
#include "dmlab2d/lib/util/default_read_only_file_system.h"
#include "dmlab2d/lib/util/file_reader_types.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind::lab2d {

constexpr char kGameScriptPath[] =
    "/org_deepmind_lab2d/dmlab2d/lib/game_scripts";
constexpr char kLevelDirectory[] = "levels";
constexpr char kScriptFromSetting[] = "<script from setting>";

EnvLuaApi::EnvLuaApi(std::string executable_runfiles)
    : lua_vm_(lua::CreateVm()),
      executable_runfiles_(std::move(executable_runfiles)),
      file_system_(executable_runfiles_, util::DefaultReadOnlyFileSystem()),
      mixer_seed_(0) {}

int EnvLuaApi::AddSetting(absl::string_view key, absl::string_view value) {
  if (key == "levelName") {
    SetLevelName(std::string(value));
    return 0;
  }
  if (key == "mixerSeed") {
    std::uint32_t mixer_seed;
    if (absl::SimpleAtoi(value, &mixer_seed)) {
      SetMixerSeed(mixer_seed);
      return 0;
    } else {
      SetErrorMessage(absl::StrCat("Invalid settings 'mixerSeed' : ", value));
      return 1;
    }
  }
  if (key == "levelDirectory") {
    SetLevelDirectory(std::string(value));
    return 0;
  }
  settings_.emplace(key, value);
  return 0;
}

void EnvLuaApi::SetLevelName(absl::string_view level_name) {
  if (!level_name.empty() && level_name.front() == '=') {
    level_script_content_ = level_name.substr(1);
  }
  if (auto sep = level_name.find_last_of(':'); sep != absl::string_view::npos) {
    level_name_ = level_name.substr(0, sep);
    sub_level_name_ = level_name.substr(sep + 1);
  } else {
    level_name_ = level_name;
    sub_level_name_.clear();
  }
}

void EnvLuaApi::SetLevelDirectory(std::string level_directory) {
  level_directory_ = std::move(level_directory);
}

std::string EnvLuaApi::GetLevelDirectory() {
  if (!level_directory_.empty()) {
    if (level_directory_[0] == '/') {
      return level_directory_;
    } else {
      return absl::StrCat(ExecutableRunfiles(), "/", level_directory_);
    }
  }
  return absl::StrCat(ExecutableRunfiles(), kGameScriptPath, "/",
                      kLevelDirectory);
}

lua::NResultsOr EnvLuaApi::PushLevelScriptAndName() {
  lua_State* L = lua_vm_.get();
  if (!level_script_content_.empty()) {
    if (!level_directory_.empty()) {
      lua_vm_.AddPathToSearchers(level_directory_);
    }
    if (auto result =
            lua::PushScript(L, level_script_content_, kScriptFromSetting);
        !result.ok()) {
      return result;
    }
    lua::Push(L, kGameScriptPath);
    return 2;
  }
  if (absl::EndsWith(level_name_, ".lua")) {
    if (auto pos = level_name_.find_last_of('/'); pos != std::string::npos) {
      lua_vm_.AddPathToSearchers(level_name_.substr(0, pos));
    }
    if (!level_directory_.empty()) {
      lua_vm_.AddPathToSearchers(level_directory_);
    }
    if (auto result = lua::PushScriptFile(L, level_name_); !result.ok()) {
      return result;
    }
    lua::Push(L, level_name_);
    return 2;
  }
  if (level_name_.empty()) {
    return "Missing level script! Must set setting 'levelName'!";
  }

  auto level_directory = GetLevelDirectory();
  auto level_path = absl::StrCat(level_directory, "/", level_name_, ".lua");
  if (std::ifstream f(level_path.c_str()); f.good()) {
    auto last_sep = level_path.find_last_of('/');
    auto level_root = level_path.substr(0, last_sep);
    if (level_root != level_directory) {
      lua_vm_.AddPathToSearchers(level_root);
    }
  } else {
    auto level_root = absl::StrCat(level_directory, "/", level_name_);
    level_path = absl::StrCat(level_root, "/init.lua");
    lua_vm_.AddPathToSearchers(level_root);
  }
  lua_vm_.AddPathToSearchers(level_directory);
  lua_vm_.AddPathToSearchers(ExecutableRunfiles());

  if (auto result = lua::PushScriptFile(L, level_path); !result.ok()) {
    return result;
  }
  lua::Push(L, level_path);
  return 2;
}

int EnvLuaApi::Init() {
  lua_State* L = lua_vm_.get();
  lua::StackResetter stack_resetter(L);
  tensor::LuaTensorRegister(L);
  LuaRandom::Register(L);
  if (StoreError(PushLevelScriptAndName())) {
    return 1;
  }

  lua_vm_.AddPathToSearchers(
      absl::StrCat(ExecutableRunfiles(), kGameScriptPath));

  void* readonly_fs = const_cast<DeepMindReadOnlyFileSystem*>(
      GetFileSystem().ReadOnlyFileSystem());
  lua_vm_.AddCModuleToSearchers("system.image", LuaImageRequire, {readonly_fs});
  lua_vm_.AddCModuleToSearchers("system.tensor", tensor::LuaTensorConstructors,
                                {readonly_fs});

  lua_vm_.AddCModuleToSearchers("system.events", &lua::Bind<Events::Module>,
                                {MutableEvents()});
  lua_vm_.AddCModuleToSearchers(
      "system.random", &lua::Bind<LuaRandom::Require>,
      {UserPrbg(),
       reinterpret_cast<void*>(static_cast<std::uintptr_t>(mixer_seed_))});

  lua_vm_.AddCModuleToSearchers("system.properties",
                                &lua::Bind<Properties::Module>);

  if (auto result = lua::Call(L, 1); StoreError(result)) {
    return 1;
  } else if (result.n_results() != 1) {
    error_message_ = "Lua script function must return a table.";
    return 1;
  }

  if (lua_isfunction(L, -1)) {
    lua::Push(L, sub_level_name_);
    if (auto result = lua::Call(L, 1); StoreError(result)) {
      return 1;
    } else if (result.n_results() != 1) {
      error_message_ = "Lua script must return only a table or function.";
      return 1;
    }
  }

  if (!IsFound(lua::Read(L, -1, &script_table_ref_))) {
    error_message_ = absl::StrCat(
        "Lua script must return a table or function, Actually returned : '",
        lua::ToString(L, -1), "'");
    return 1;
  }

  lua_settop(L, 0);  // ApiInit expects an empty Lua stack.

  int error_value = 0;
  if (StoreError(ApiInit(&error_value)) ||
      StoreError(MutableObservations()->BindApi(script_table_ref_)) ||
      StoreError(MutableActions()->BindApi(script_table_ref_)) ||
      StoreError(MutableProperties()->BindApi(script_table_ref_)) ||
      StoreError(MutableEpisode()->BindApi(script_table_ref_))) {
    return error_value != 0 ? error_value : 1;
  }
  return 0;
}

int EnvLuaApi::MakeRandomSeed() {
  return std::uniform_int_distribution<int>(
      1, std::numeric_limits<int>::max())(*EnginePrbg());
}

int EnvLuaApi::Start(int episode, int seed) {
  MutableEvents()->Clear();
  EnginePrbg()->seed(static_cast<std::uint64_t>(seed) ^
                     (static_cast<std::uint64_t>(mixer_seed_) << 32));
  return StoreError(MutableEpisode()->Start(episode, MakeRandomSeed())) ? 1 : 0;
}

lua::NResultsOr EnvLuaApi::ApiInit(int* error_value) {
  lua_State* L = lua_vm_.get();
  lua::StackResetter stack_resetter(L);
  script_table_ref_.PushMemberFunction("init");
  if (lua_isnil(L, -2)) {
    return 0;
  }
  lua::Push(L, settings_);
  auto result = lua::Call(L, 2);
  if (!result.ok()) {
    return result;
  }

  bool correct_args =
      result.n_results() == 0 || (result.n_results() == 1 && lua_isnil(L, 1)) ||
      (result.n_results() <= 2 && IsFound(lua::Read(L, 1, error_value)));
  if (*error_value != 0) {
    if (result.n_results() == 2) {
      return lua::ToString(L, 2);
    } else {
      return "[init] - Script returned non zero.";
    }
  }
  if (!correct_args) {
    return "[init] - Must return none, nil, or integer and message";
  }
  return 0;
}

EnvCApi_EnvironmentStatus EnvLuaApi::Advance(int number_of_steps,
                                             double* reward) {
  if (number_of_steps != 1) {
    SetErrorMessage("DeepMind Lab2d does not support frame skip.");
    return EnvCApi_EnvironmentStatus_Error;
  }
  MutableEvents()->Clear();
  EnvCApi_EnvironmentStatus status;
  if (StoreError(MutableEpisode()->Advance(&status, reward))) {
    return EnvCApi_EnvironmentStatus_Error;
  }
  return status;
}

}  // namespace deepmind::lab2d
