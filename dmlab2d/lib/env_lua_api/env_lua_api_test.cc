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

#include "dmlab2d/lib/env_lua_api/env_lua_api.h"

#include <algorithm>
#include <string>
#include <vector>

#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "dmlab2d/lib/util/files.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind::lab2d {
namespace {

using ::testing::ElementsAre;
using ::testing::ElementsAreArray;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::Not;
using ::testing::StrEq;
using ::testing::UnorderedElementsAreArray;

class Test : public ::testing::Test {
 protected:
  Test() : env_(util::GetTempDirectory() + "/dmlab2d_lua_env_test") {
    util::MakeDirectory(env_.ExecutableRunfiles());
  }

  ~Test() override { util::RemoveDirectory(env_.ExecutableRunfiles()); }

  EnvLuaApi env_;
};

constexpr absl::string_view kDefaultRoot = "dmlab2d/lib/game_scripts/levels";

constexpr char kFullApi[] = R"(=
local events = require 'system.events'
local tensor = require 'system.tensor'
local sys_random = require 'system.random'
local properties = require 'system.properties'

local api = {
  _steps = 10,
  _reward = 0,
  _continuousAction = {0, 0},
  _observation = tensor.DoubleTensor{0.0, 0.0},
  _coverage = {}
}

function api:init(kwargs)
  self._coverage.init = true
  local errors = ''
  for k, v in pairs(kwargs) do
    if k == 'steps' and tonumber(v) and tonumber(v) >= 0 then
      self._steps = tonumber(v)
    else
      errors = errors .. 'Invalid setting \'' .. k .. '\'=\'' .. v .. '\'\n'
    end
  end
  if errors ~= '' then
    return 1, errors
  end
end

function api:observationSpec()
  self._coverage.observationSpec = true
  return {{
      name = 'VIEW',
      type = self._observation:type(),
      shape = self._observation:shape(),
  }}
end

function api:observation(idx)
  self._coverage.observation = true
  return idx == 1 and self._observation
end

function api:discreteActionSpec()
  self._coverage.discreteActionSpec = true
  return {{name = 'REWARD_ACT', min = 0, max = 4}}
end

function api:discreteActions(actions)
  self._coverage.discreteActions = true
  self._reward = actions[1]
end

function api:continuousActionSpec()
  self._coverage.continuousActionSpec = true
  return {
      {name = 'OFFSET_X', min = -1, max = 1},
      {name = 'OFFSET_Y', min = -1, max = 1},
  }
end

function api:continuousActions(actions)
  self._coverage.continuousActions = true
  self._continuousAction = actions
end

function api:start(episode, seed)
  self._coverage.start = true
  sys_random:seed(seed)
  self._continuousAction = {0.0, 0.0}
  self._observation:val(self._continuousAction)
  events:add('start')
end

function api:advance(steps)
  self._coverage.advance = true
  self._observation:val(self._continuousAction)
  return steps < self._steps, self._reward
end

function api:writeProperty(key, value)
  self._coverage.writeProperty = true
  if key == 'steps' then
    v = tonumber(value)
    if v ~= nil and v > 0 then
      self._steps = v
      return properties.SUCCESS
    else
      return properties.INVALID_ARGUMENT
    end
  elseif key == 'coverage' then
    return properties.PERMISSION_DENIED
  end
  return properties.NOT_FOUND
end

function api:readProperty(key)
  self._coverage.readProperty = true
  if key == 'steps' then
    return tostring(self._steps)
  elseif key == 'coverage' then
    local lst = {}
    for k, _ in pairs(self._coverage) do
        lst[#lst+1] = k
    end
    return table.concat(lst, ',')
  end
  return properties.NOT_FOUND
end

function api:listProperty(key, callback)
  self._coverage.listProperty = true
  if key == '' then
    callback('steps', 'rw')
    callback('coverage', 'r')
    return properties.SUCCESS
  elseif key == 'steps' or key == 'coverage' then
    return properties.PERMISSION_DENIED
  end
  return properties.NOT_FOUND
end

return api
)";

TEST_F(Test, FullApi) {
  ASSERT_THAT(env_.AddSetting("levelName", kFullApi), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.AddSetting("steps", "4"), Eq(0)) << env_.ErrorMessage();
  ASSERT_THAT(env_.Init(), Eq(0)) << env_.ErrorMessage();

  // Properties.
  const char* steps = nullptr;
  ASSERT_THAT(env_.MutableProperties()->ReadProperty("steps", &steps),
              Eq(EnvCApi_PropertyResult_Success));
  EXPECT_THAT(steps, StrEq("4"));

  ASSERT_THAT(env_.MutableProperties()->WriteProperty("steps", "2"),
              Eq(EnvCApi_PropertyResult_Success));

  ASSERT_THAT(env_.MutableProperties()->ReadProperty("steps", &steps),
              Eq(EnvCApi_PropertyResult_Success));
  EXPECT_THAT(steps, StrEq("2"));

  int counter = 0;
  EnvCApi_PropertyResult list_status = env_.MutableProperties()->ListProperty(
      &counter, "",
      +[](void* userdata, const char* key,
          EnvCApi_PropertyAttributes attributes) {
        int& counter = *static_cast<int*>(userdata);
        if (counter == 0) {
          EXPECT_THAT(key, StrEq("steps"));
          EXPECT_THAT(attributes, Eq(EnvCApi_PropertyAttributes_ReadWritable));
        } else {
          EXPECT_THAT(counter, Eq(1));
          EXPECT_THAT(key, StrEq("coverage"));
          EXPECT_THAT(attributes, Eq(EnvCApi_PropertyAttributes_Readable));
        }
        ++counter;
      });
  ASSERT_THAT(list_status, Eq(EnvCApi_PropertyResult_Success));
  EXPECT_THAT(counter, Eq(2));

  // Action Spec.
  ASSERT_THAT(env_.GetActions().DiscreteCount(), Eq(1));
  EXPECT_THAT(env_.GetActions().DiscreteName(0), StrEq("REWARD_ACT"));
  ASSERT_THAT(env_.GetActions().ContinuousCount(), Eq(2));
  EXPECT_THAT(env_.GetActions().ContinuousName(0), StrEq("OFFSET_X"));
  EXPECT_THAT(env_.GetActions().ContinuousName(1), StrEq("OFFSET_Y"));

  // Observation Spec
  ASSERT_THAT(env_.GetObservations().Count(), Eq(1));
  EnvCApi_ObservationSpec spec = {};
  env_.GetObservations().Spec(0, &spec);
  ASSERT_THAT(absl::MakeConstSpan(spec.shape, spec.dims), ElementsAre(2));
  ASSERT_THAT(spec.type, Eq(EnvCApi_ObservationDoubles));

  // Start
  ASSERT_THAT(env_.Start(0, 0), Eq(0)) << env_.ErrorMessage();

  // Events
  ASSERT_THAT(env_.GetEvents().Count(), Eq(1));
  EnvCApi_Event event = {};
  env_.MutableEvents()->Export(0, &event);
  EXPECT_THAT(env_.GetEvents().TypeName(event.id), StrEq("start"));

  // First Observation
  EnvCApi_Observation obs = {};
  env_.MutableObservations()->Observation(0, &obs);
  EXPECT_THAT(absl::MakeConstSpan(obs.payload.doubles, 2), ElementsAre(0, 0));

  int discrete_action = 3;
  env_.MutableActions()->DiscreteApply(&discrete_action);
  std::array<double, 2> continuous_actions = {1.0, 2.0};
  env_.MutableActions()->ContinuousApply(continuous_actions.data());
  double reward = 0.0;

  // Advance
  ASSERT_THAT(env_.Advance(1, &reward), Eq(EnvCApi_EnvironmentStatus_Running))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.GetEvents().Count(), Eq(0));
  EXPECT_THAT(reward, Eq(discrete_action));
  env_.MutableObservations()->Observation(0, &obs);
  EXPECT_THAT(absl::MakeConstSpan(obs.payload.doubles, 2),
              ElementsAreArray(continuous_actions));

  ASSERT_THAT(env_.Advance(1, &reward),
              Eq(EnvCApi_EnvironmentStatus_Terminated))
      << env_.ErrorMessage();

  const char* coverage = "";
  ASSERT_THAT(env_.MutableProperties()->ReadProperty("coverage", &coverage),
              Eq(EnvCApi_PropertyResult_Success));
  EXPECT_THAT(absl::StrSplit(coverage, ','),
              UnorderedElementsAreArray({
                  "init",                  //
                  "observationSpec",       //
                  "observation",           //
                  "discreteActionSpec",    //
                  "discreteActions",       //
                  "continuousActionSpec",  //
                  "continuousActions",     //
                  "start",                 //
                  "advance",               //
                  "writeProperty",         //
                  "readProperty",          //
                  "listProperty",          //
              }));
}

constexpr char kMixerSeed[] = R"(=
return {
  init = function(_, kwargs)
    for k, v in pairs(kwargs) do
      error('Unrecognised setting ' .. k)
    end
  end,
  start = function(_, episode, seed)
    error(seed)
  end,
}
)";

TEST_F(Test, MixerSeed) {
  EnvLuaApi context2(env_.ExecutableRunfiles());
  ASSERT_THAT(env_.AddSetting("levelName", kMixerSeed), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(context2.AddSetting("levelName", kMixerSeed), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.AddSetting("mixerSeed", absl::StrCat(0x600D5EED)), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(context2.AddSetting("mixerSeed", absl::StrCat(0x5EED600D)), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.Init(), Eq(0)) << env_.ErrorMessage();
  ASSERT_THAT(context2.Init(), Eq(0)) << env_.ErrorMessage();
  ASSERT_THAT(env_.Start(0, 10), Not(Eq(0)));
  ASSERT_THAT(env_.Start(0, 10), Not(Eq(0)));
  EXPECT_THAT(env_.ErrorMessage(), Not(Eq(context2.ErrorMessage())));
}

TEST_F(Test, BadMixerSeed) {
  ASSERT_THAT(env_.AddSetting("mixerSeed", "hello"), Not(Eq(0)));
  EXPECT_THAT(absl::string_view(env_.ErrorMessage()),
              HasSubstr("Invalid settings 'mixerSeed' : hello"));
}

TEST_F(Test, ErrorOnNoLevelName) {
  ASSERT_THAT(env_.Init(), Not(Eq(0)));
  EXPECT_THAT(absl::string_view(env_.ErrorMessage()), HasSubstr("'levelName'"));
}

constexpr char kErrorOnInit[] = R"(=
return {
  init = function() error('init error') end
}
)";

TEST_F(Test, ErrorOnInit) {
  ASSERT_THAT(env_.AddSetting("levelName", kErrorOnInit), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.Init(), Not(Eq(0)));
  EXPECT_THAT(absl::string_view(env_.ErrorMessage()), HasSubstr("init error"));
}

constexpr char kErrorOnInitUndecorated[] = R"(=
return {
  init = function() return 2, 'init error exact' end
}
)";

TEST_F(Test, ErrorOnInitUndecorated) {
  ASSERT_THAT(env_.AddSetting("levelName", kErrorOnInitUndecorated), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.Init(), Eq(2));
  EXPECT_THAT(env_.ErrorMessage(), StrEq("init error exact"));
}

constexpr char kErrorOnStart[] = R"(=
return {
  init = function() end,
  start = function() error('start error') end,
}
)";

TEST_F(Test, ErrorOnStart) {
  ASSERT_THAT(env_.AddSetting("levelName", kErrorOnStart), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.Init(), Eq(0)) << env_.ErrorMessage();
  ASSERT_THAT(env_.Start(0, 0), Not(Eq(0)));
  EXPECT_THAT(absl::string_view(env_.ErrorMessage()), HasSubstr("start error"));
}

constexpr char kErrorOnAdvance[] = R"(=
return {
  init = function() end,
  start = function() end,
  advance = function() error('advance error') end,
}
)";

TEST_F(Test, ErrorOnAdvance) {
  ASSERT_THAT(env_.AddSetting("levelName", kErrorOnAdvance), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.Init(), Eq(0)) << env_.ErrorMessage();
  ASSERT_THAT(env_.Start(0, 0), Eq(0)) << env_.ErrorMessage();
  double reward = 0.0;
  ASSERT_THAT(env_.Advance(1, &reward), Not(Eq(0)));
  EXPECT_THAT(absl::string_view(env_.ErrorMessage()),
              HasSubstr("advance error"));
}

constexpr char kLoadFromFile[] = R"(
return {
  init = function() return 3, 'Loaded from file!' end
}
)";

TEST_F(Test, LoadFromFileLevelDirectory) {
  std::string path = env_.ExecutableRunfiles() + "/load_from_file.lua";
  util::SetContents(path, kLoadFromFile);
  ASSERT_THAT(env_.AddSetting("levelDirectory", env_.ExecutableRunfiles()),
              Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.AddSetting("levelName", "load_from_file"), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.Init(), Eq(3)) << env_.ErrorMessage();
  EXPECT_THAT(env_.ErrorMessage(), StrEq("Loaded from file!"));
}

TEST_F(Test, LoadFromFileLevelDirectoryInit) {
  std::string path = env_.ExecutableRunfiles() + "/load_from_file_init";
  util::MakeDirectory(path);
  util::SetContents(path + "/init.lua", kLoadFromFile);
  ASSERT_THAT(env_.AddSetting("levelDirectory", env_.ExecutableRunfiles()),
              Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.AddSetting("levelName", "load_from_file_init"), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.Init(), Eq(3)) << env_.ErrorMessage();
  EXPECT_THAT(env_.ErrorMessage(), StrEq("Loaded from file!"));
}

TEST_F(Test, LoadFromFileLevelDirectoryRelative) {
  std::string path = absl::StrCat(env_.ExecutableRunfiles(), "/", kDefaultRoot);
  util::MakeDirectory(path);
  util::SetContents(path + "/load_from_file.lua", kLoadFromFile);
  ASSERT_THAT(env_.AddSetting("levelDirectory", kDefaultRoot), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.AddSetting("levelName", "load_from_file"), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.Init(), Eq(3)) << env_.ErrorMessage();
  EXPECT_THAT(env_.ErrorMessage(), StrEq("Loaded from file!"));
}

TEST_F(Test, LoadFromFileLevelDirectoryRelativeInit) {
  std::string path = absl::StrCat(env_.ExecutableRunfiles(), "/", kDefaultRoot,
                                  "/tests/load_from_file_init");
  util::MakeDirectory(path);
  util::SetContents(path + "/init.lua", kLoadFromFile);
  ASSERT_THAT(
      env_.AddSetting("levelDirectory", absl::StrCat(kDefaultRoot, "/tests")),
      Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.AddSetting("levelName", "load_from_file_init"), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.Init(), Eq(3)) << env_.ErrorMessage();
  EXPECT_THAT(env_.ErrorMessage(), StrEq("Loaded from file!"));
}

TEST_F(Test, LoadFromFileLevels) {
  std::string path = absl::StrCat(env_.ExecutableRunfiles(), "/", kDefaultRoot);
  util::MakeDirectory(path);
  util::SetContents(path + "/load_from_file.lua", kLoadFromFile);
  ASSERT_THAT(env_.AddSetting("levelName", "load_from_file"), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.Init(), Eq(3)) << env_.ErrorMessage();
  EXPECT_THAT(env_.ErrorMessage(), StrEq("Loaded from file!"));
}

TEST_F(Test, LoadFromFileLevelsInit) {
  std::string path = absl::StrCat(env_.ExecutableRunfiles(), "/", kDefaultRoot,
                                  "/load_from_file_init");
  util::MakeDirectory(path);
  util::SetContents(path + "/init.lua", kLoadFromFile);
  ASSERT_THAT(env_.AddSetting("levelName", "load_from_file_init"), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.Init(), Eq(3)) << env_.ErrorMessage();
  EXPECT_THAT(env_.ErrorMessage(), StrEq("Loaded from file!"));
}

TEST_F(Test, LoadFromFileAbsolute) {
  std::string path = absl::StrCat(env_.ExecutableRunfiles(), "/", kDefaultRoot);
  util::MakeDirectory(path);
  util::SetContents(path + "/load_from_file.lua", kLoadFromFile);
  ASSERT_THAT(env_.AddSetting("levelName", path + "/load_from_file.lua"), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.Init(), Eq(3)) << env_.ErrorMessage();
  EXPECT_THAT(env_.ErrorMessage(), StrEq("Loaded from file!"));
}

constexpr char kApiFromFunctionCall[] = R"(
return function(name)
  return {
      init = function() return 3, 'Loaded from function!' .. name end
  }
end
)";

TEST_F(Test, LoadFromFunctionCall) {
  std::string path = absl::StrCat(env_.ExecutableRunfiles(), "/", kDefaultRoot);
  util::MakeDirectory(path);
  util::SetContents(path + "/function_call.lua", kApiFromFunctionCall);
  ASSERT_THAT(env_.AddSetting("levelName", "function_call:hello"), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.Init(), Eq(3)) << env_.ErrorMessage();
  EXPECT_THAT(env_.ErrorMessage(), StrEq("Loaded from function!hello"));
}

TEST_F(Test, LoadFromFunctionCallInit) {
  std::string path = absl::StrCat(env_.ExecutableRunfiles(), "/", kDefaultRoot,
                                  "/load_from_folder");
  util::MakeDirectory(path);
  util::SetContents(path + "/init.lua", kApiFromFunctionCall);
  ASSERT_THAT(env_.AddSetting("levelName", "load_from_folder:hello"), Eq(0))
      << env_.ErrorMessage();
  ASSERT_THAT(env_.Init(), Eq(3)) << env_.ErrorMessage();
  EXPECT_THAT(env_.ErrorMessage(), StrEq("Loaded from function!hello"));
}

}  // namespace
}  // namespace deepmind::lab2d
