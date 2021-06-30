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

#ifndef DMLAB2D_LIB_ENV_LUA_API_ENV_LUA_API_H_
#define DMLAB2D_LIB_ENV_LUA_API_ENV_LUA_API_H_

#include <array>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <random>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "dmlab2d/lib/env_lua_api/actions.h"
#include "dmlab2d/lib/env_lua_api/episode.h"
#include "dmlab2d/lib/env_lua_api/events.h"
#include "dmlab2d/lib/env_lua_api/observations.h"
#include "dmlab2d/lib/env_lua_api/properties.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/table_ref.h"
#include "dmlab2d/lib/lua/vm.h"
#include "dmlab2d/lib/system/file_system/file_system.h"
#include "dmlab2d/lib/util/file_reader_types.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind::lab2d {

// This is the userdata in DeepmindContext. It contains the Lua VM and
// methods for handling callbacks from DeepMind Lab.
class EnvLuaApi {
 public:
  // Constructed with a Lua VM.
  // 'executable_runfiles' path to where DeepMind Lab assets are stored.
  explicit EnvLuaApi(std::string executable_runfiles);

  // If key is 'levelName' then calls SetLevelName with value.
  // If key is 'mixerSeed' then all built-in random number generators will
  // generate a differnt sequence when given the same seed. Otherwise inserts
  // 'key' 'value' into settings_ ready to be processed by init call in Lua.
  // Must be called before Init.
  int AddSetting(absl::string_view key, absl::string_view value);

  // Runs the script named level_name_ and stores the result in
  // script_table_ref_.
  // Calls "init" member function on the script_table_ref_ with settings_.
  // Returns zero if successful and non-zero on error.
  int Init();

  // Calls "start" member function on the script_table_ref_ with episode and
  // seed.
  // Returns zero if successful and non-zero on error.
  int Start(int episode, int seed);

  // Returns the status of the environment.
  EnvCApi_EnvironmentStatus Advance(int number_of_steps, double* reward);

  // Path to where DeepMind Lab assets are stored.
  const std::string& ExecutableRunfiles() const { return executable_runfiles_; }

  // Returns a new random seed on each call. Internally uses 'engine_prbg_'
  // to generate new positive integers.
  int MakeRandomSeed();

  std::uint32_t MixerSeed() const { return mixer_seed_; }

  std::mt19937_64* UserPrbg() { return &user_prbg_; }

  std::mt19937_64* EnginePrbg() { return &engine_prbg_; }

  const char* ErrorMessage() const { return error_message_.c_str(); }

  // Sets current error message. 'message' shall be a null terminated string.
  void SetErrorMessage(absl::string_view message) {
    error_message_ = std::string(message);
  }

  const FileSystem& GetFileSystem() const { return file_system_; }
  FileSystem* MutableFileSystem() { return &file_system_; }

  const Events& GetEvents() const { return events_; }
  Events* MutableEvents() { return &events_; }

  const Episode& GetEpisode() const { return episode_; }
  Episode* MutableEpisode() { return &episode_; }

  const Observations& GetObservations() const { return observations_; }
  Observations* MutableObservations() { return &observations_; }

  const Properties& GetProperties() const { return properties_; }
  Properties* MutableProperties() { return &properties_; }

  const Actions& GetActions() const { return actions_; }
  Actions* MutableActions() { return &actions_; }

  lua::Vm* mutable_lua_vm() { return &lua_vm_; }

 private:
  // 'level_name': name of a Lua file; this script is run during first call to
  // Init.
  // Must be called before Init.
  void SetLevelName(absl::string_view level_name);

  // Specifies a mixer value to be combined with all the seeds passed to this
  // environment, before using them with the internal PRBGs. This is done in
  // a way which guarantees that the resulting seeds span disjoint subsets of
  // the integers in [0, 2^64) for each different mixer value. However, the
  // sequences produced by the environment's PRBGs are not necessarily disjoint.
  void SetMixerSeed(std::uint32_t s) { mixer_seed_ = s; }

  // 'level_directory': Sets the directory to find level scripts in. If a local
  // path is used it will be relative to the 'games_scripts' directory. (Default
  // is 'levels'.)
  void SetLevelDirectory(std::string level_directory);

  // Calls Lua script 'init' function with settings dictionary.
  // An optional return value may be returned from the script. This value
  // is ignored if error_value is 0 or ther is no error.
  // Must be called with an empty Lua stack.
  lua::NResultsOr ApiInit(int* error_value);

  lua::NResultsOr PushLevelScriptAndName();

  // If there is an error in `result`, the error is stored and returns true.
  // Otherwise returns false.
  bool StoreError(const lua::NResultsOr& result) {
    if (!result.ok()) {
      SetErrorMessage(result.error());
      return true;
    }
    return false;
  }

  EnvLuaApi(const EnvLuaApi&) = delete;
  EnvLuaApi& operator=(const EnvLuaApi&) = delete;

  // Returns the path to the level directory.
  std::string GetLevelDirectory();

  // The context's Lua VM. The top of the stack of the VM is zero before and
  // after any call.
  lua::Vm lua_vm_;

  // Path to the executable's assets.
  std::string executable_runfiles_;

  // The settings to run the script with.
  absl::flat_hash_map<std::string, std::string> settings_;

  // When a levelName is set without the suffix '.lua' the level is found
  // relative to this directory.
  std::string level_directory_;

  std::vector<std::string> require_order_;

  // The name of the script to run on first Init.
  std::string level_name_;

  // The name of the script to run on first Init.
  std::string sub_level_name_;

  // The content of a level script passed by a setting.
  std::string level_script_content_;

  // Last error message.
  std::string error_message_;

  // The result of the script that was run when Init was first called.
  lua::TableRef script_table_ref_;

  // A pseudo-random-bit generator for exclusive use by users.
  std::mt19937_64 user_prbg_;

  // A pseudo-random-bit generator for exclusive use of the EnvCApi. Seeded
  // each episode with the episode start seed.
  std::mt19937_64 engine_prbg_;

  // An object for storing location of `runfiles` directory.
  FileSystem file_system_;

  // An object for storing and retrieving events.
  Events events_;

  // An object for starting and advancing an episode.
  Episode episode_;

  // An object for storing and retrieving custom observations.
  Observations observations_;

  // An object for storing and retrieving custom properties.
  Properties properties_;

  // An object for storing and applying custom actions.
  Actions actions_;

  // Stores the mixer seed for the random bit generators.
  std::uint32_t mixer_seed_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_ENV_LUA_API_ENV_LUA_API_H_
