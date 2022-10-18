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

#include "dmlab2d/lib/dmlab2d.h"

#include <memory>
#include <utility>

#include "absl/memory/memory.h"
#include "dmlab2d/lib/env_lua_api/env_lua_api.h"
#include "dmlab2d/lib/lua/bind.h"
#include "dmlab2d/lib/system/file_system/lua/file_system.h"
#include "dmlab2d/lib/system/generators/pushbox/lua/pushbox.h"
#include "dmlab2d/lib/system/grid_world/lua/lua_world.h"
#include "dmlab2d/lib/system/tile/lua/tile.h"
#include "third_party/rl_api/env_c_api.h"
#include "third_party/rl_api/env_c_api_bind.h"

namespace deepmind::lab2d {
namespace {

// Forward EnvCApi calls to EnvLuaApi.
class Lab2D {
 public:
  Lab2D(const DeepMindLab2DLaunchParams* params) : env_(params->runfiles_root) {
    env_.mutable_lua_vm()->AddCModuleToSearchers("system.tile", LuaTileModule);
    env_.mutable_lua_vm()->AddCModuleToSearchers("system.grid_world",
                                                 LuaWorld::Module);
    env_.mutable_lua_vm()->AddCModuleToSearchers("system.generators.pushbox",
                                                 LuaPushboxRequire);
    env_.mutable_lua_vm()->AddCModuleToSearchers(
        "system.file_system", &lua::Bind<LuaFileSystemRequire>,
        {env_.MutableFileSystem()});
  }

  int Setting(const char* key, const char* value) {
    return env_.AddSetting(key, value);
  }

  int Init() { return env_.Init(); }

  int Start(int episode_id, int seed) { return env_.Start(episode_id, seed); }

  const char* ErrorMessage() const { return env_.ErrorMessage(); }

  const char* EnvironmentName() const { return "dmlab2d"; }

  int ActionDiscreteCount() const { return env_.GetActions().DiscreteCount(); }

  const char* ActionDiscreteName(int discrete_idx) const {
    return env_.GetActions().DiscreteName(discrete_idx);
  }

  void ActionDiscreteBounds(int discrete_idx, int* min_value,
                            int* max_value) const {
    return env_.GetActions().DiscreteBounds(discrete_idx, min_value, max_value);
  }

  int ActionContinuousCount() const {
    return env_.GetActions().ContinuousCount();
  }

  const char* ActionContinuousName(int discrete_idx) const {
    return env_.GetActions().ContinuousName(discrete_idx);
  }

  void ActionContinuousBounds(int discrete_idx, double* min_value,
                              double* max_value) const {
    return env_.GetActions().ContinuousBounds(discrete_idx, min_value,
                                              max_value);
  }

  int ActionTextCount() const { return env_.GetActions().TextCount(); }

  const char* ActionTextName(int text_idx) const {
    return env_.GetActions().TextName(text_idx);
  }

  int ObservationCount() const { return env_.GetObservations().Count(); }
  const char* ObservationName(int observation_idx) {
    return env_.GetObservations().Name(observation_idx);
  }

  void ObservationSpec(int observation_idx,
                       EnvCApi_ObservationSpec* spec) const {
    return env_.GetObservations().Spec(observation_idx, spec);
  }

  void Observation(int observation_idx, EnvCApi_Observation* observation) {
    return env_.MutableObservations()->Observation(observation_idx,
                                                   observation);
  }

  int EventTypeCount() const { return env_.GetEvents().TypeCount(); }
  const char* EventTypeName(int event_type_idx) const {
    return env_.GetEvents().TypeName(event_type_idx);
  }

  int EventCount() { return env_.GetEvents().Count(); }

  void Event(int event_idx, EnvCApi_Event* event) {
    return env_.MutableEvents()->Export(event_idx, event);
  }

  void ActDiscrete(const int actions_discrete[]) {
    if (actions_discrete != nullptr) {
      env_.MutableActions()->DiscreteApply(actions_discrete);
    }
  }

  void ActContinuous(const double actions_continuous[]) {
    if (actions_continuous != nullptr) {
      env_.MutableActions()->ContinuousApply(actions_continuous);
    }
  }

  void ActText(const EnvCApi_TextAction actions_text[]) {
    if (actions_text != nullptr) {
      env_.MutableActions()->TextApply(actions_text);
    }
  }

  EnvCApi_EnvironmentStatus Advance(int num_steps, double* reward) {
    return env_.Advance(num_steps, reward);
  }

  EnvCApi_PropertyResult WriteProperty(const char* key, const char* value) {
    return env_.MutableProperties()->WriteProperty(key, value);
  }

  EnvCApi_PropertyResult ReadProperty(const char* key, const char** value) {
    return env_.MutableProperties()->ReadProperty(key, value);
  }

  EnvCApi_PropertyResult ListProperty(
      void* userdata, const char* list_key,
      void (*list_callback)(void* userdata, const char* key,
                            EnvCApi_PropertyAttributes attributes)) {
    return env_.MutableProperties()->ListProperty(userdata, list_key,
                                                  list_callback);
  }

 private:
  EnvLuaApi env_;
};

}  // namespace
}  // namespace deepmind::lab2d

extern "C" int dmlab2d_connect(const DeepMindLab2DLaunchParams* params,
                               EnvCApi* env_c_api, void** context) {
  auto game = std::make_unique<deepmind::lab2d::Lab2D>(params);
  deepmind::rl_api::Bind(std::move(game), env_c_api, context);
  return 0;
}
