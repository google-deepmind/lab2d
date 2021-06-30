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

#include <string>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "dmlab2d/lib/util/test_srcdir.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "third_party/rl_api/env_c_api.h"
#include "third_party/rl_api/env_c_api_test_suite.h"

namespace {

using ::rl_api::EnvCApiConformanceTestSuite;
using ::rl_api::EnvCApiTestParam;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::Gt;
using ::testing::HasSubstr;
using ::testing::StrEq;
using ::testing::Test;

int test_connect(EnvCApi* api, void** context) {
  const std::string runfiles_root = deepmind::lab2d::util::TestSrcDir();
  DeepMindLab2DLaunchParams params = {};
  params.runfiles_root = runfiles_root.c_str();
  return dmlab2d_connect(&params, api, context);
}

INSTANTIATE_TEST_SUITE_P(DeepmindLab2DEnvCApiConformanceTest,
                         EnvCApiConformanceTestSuite,
                         testing::Values(EnvCApiTestParam{
                             test_connect,
                             {
                                 {"levelName", "examples/level_api"},
                             }}));

class DmLab2DTest : public Test {
 protected:
  DmLab2DTest() { test_connect(&env_, &ctx_); }

  ~DmLab2DTest() override { env_.release_context(ctx_); }

  EnvCApi env_;
  void* ctx_;
};

TEST_F(DmLab2DTest, HasEnvironmentName) {
  ASSERT_THAT(env_.environment_name(ctx_), StrEq("dmlab2d"));
}

TEST_F(DmLab2DTest, HasObservationSpec) {
  ASSERT_THAT(env_.setting(ctx_, "levelName", "examples/level_api"), Eq(0))
      << env_.error_message(ctx_);
  ASSERT_THAT(env_.init(ctx_), Eq(0)) << env_.error_message(ctx_);
  ASSERT_THAT(env_.observation_count(ctx_), Eq(5));
  ASSERT_THAT(env_.observation_name(ctx_, 0), StrEq("VIEW1"));
  ASSERT_THAT(env_.observation_name(ctx_, 1), StrEq("VIEW2"));
  ASSERT_THAT(env_.observation_name(ctx_, 2), StrEq("VIEW3"));
  ASSERT_THAT(env_.observation_name(ctx_, 3), StrEq("VIEW4"));
  ASSERT_THAT(env_.observation_name(ctx_, 4), StrEq("VIEW5"));

  EnvCApi_ObservationSpec spec;
  env_.observation_spec(ctx_, 0, &spec);
  ASSERT_THAT(absl::MakeConstSpan(spec.shape, spec.dims), ElementsAre(1));
  ASSERT_THAT(spec.type, Eq(EnvCApi_ObservationBytes));

  env_.observation_spec(ctx_, 1, &spec);
  ASSERT_THAT(absl::MakeConstSpan(spec.shape, spec.dims), ElementsAre(2));
  ASSERT_THAT(spec.type, Eq(EnvCApi_ObservationDoubles));

  env_.observation_spec(ctx_, 2, &spec);
  ASSERT_THAT(absl::MakeConstSpan(spec.shape, spec.dims), ElementsAre(3));
  ASSERT_THAT(spec.type, Eq(EnvCApi_ObservationInt32s));

  env_.observation_spec(ctx_, 3, &spec);
  ASSERT_THAT(absl::MakeConstSpan(spec.shape, spec.dims), ElementsAre(4));
  ASSERT_THAT(spec.type, Eq(EnvCApi_ObservationInt64s));

  env_.observation_spec(ctx_, 4, &spec);
  ASSERT_THAT(spec.dims, Eq(0));
  ASSERT_THAT(spec.type, Eq(EnvCApi_ObservationString));
}

TEST_F(DmLab2DTest, HasDiscreteActionSpec) {
  ASSERT_THAT(env_.setting(ctx_, "levelName", "examples/level_api"), Eq(0))
      << env_.error_message(ctx_);
  ASSERT_THAT(env_.init(ctx_), Eq(0)) << env_.error_message(ctx_);
  ASSERT_THAT(env_.action_discrete_count(ctx_), Eq(1));
  ASSERT_THAT(env_.action_discrete_name(ctx_, 0), StrEq("REWARD_ACT"));

  int min_value, max_value;
  env_.action_discrete_bounds(ctx_, 0, &min_value, &max_value);
  EXPECT_THAT(min_value, Eq(0));
  EXPECT_THAT(max_value, Eq(4));
}

TEST_F(DmLab2DTest, HasContinuousActionSpec) {
  ASSERT_THAT(env_.setting(ctx_, "levelName", "examples/level_api"), Eq(0))
      << env_.error_message(ctx_);
  ASSERT_THAT(env_.init(ctx_), Eq(0)) << env_.error_message(ctx_);
  ASSERT_THAT(env_.action_continuous_count(ctx_), Eq(1));
  ASSERT_THAT(env_.action_continuous_name(ctx_, 0), StrEq("OBSERVATION_ACT"));

  double min_value, max_value;
  env_.action_continuous_bounds(ctx_, 0, &min_value, &max_value);
  EXPECT_THAT(min_value, Eq(-5));
  EXPECT_THAT(max_value, Eq(5));
}

TEST_F(DmLab2DTest, HasTextActionSpec) {
  ASSERT_THAT(env_.setting(ctx_, "levelName", "examples/level_api"), Eq(0))
      << env_.error_message(ctx_);
  ASSERT_THAT(env_.init(ctx_), Eq(0)) << env_.error_message(ctx_);
  ASSERT_THAT(env_.action_text_count(ctx_), Eq(1));
  ASSERT_THAT(env_.action_text_name(ctx_, 0), StrEq("LOG_EVENT"));
}

TEST_F(DmLab2DTest, CanStart) {
  ASSERT_THAT(env_.setting(ctx_, "levelName", "examples/level_api"), Eq(0))
      << env_.error_message(ctx_);
  ASSERT_THAT(env_.init(ctx_), Eq(0)) << env_.error_message(ctx_);
  EXPECT_THAT(env_.start(ctx_, 0, 0), Eq(0)) << env_.error_message(ctx_);
}

TEST_F(DmLab2DTest, CanObserve) {
  ASSERT_THAT(env_.setting(ctx_, "levelName", "examples/level_api"), Eq(0))
      << env_.error_message(ctx_);
  ASSERT_THAT(env_.init(ctx_), Eq(0)) << env_.error_message(ctx_);
  ASSERT_THAT(env_.start(ctx_, 0, 0), Eq(0)) << env_.error_message(ctx_);
  EnvCApi_Observation obs;
  env_.observation(ctx_, 2, &obs);
  ASSERT_THAT(absl::MakeConstSpan(obs.spec.shape, obs.spec.dims),
              ElementsAre(3));
  ASSERT_THAT(obs.spec.type, Eq(EnvCApi_ObservationInt32s));
  EXPECT_THAT(absl::MakeConstSpan(obs.payload.int32s, 3), ElementsAre(1, 2, 3));
}

TEST_F(DmLab2DTest, CanActDiscrete) {
  ASSERT_THAT(env_.setting(ctx_, "levelName", "examples/level_api"), Eq(0))
      << env_.error_message(ctx_);
  ASSERT_THAT(env_.init(ctx_), Eq(0)) << env_.error_message(ctx_);
  ASSERT_THAT(env_.start(ctx_, 0, 0), Eq(0)) << env_.error_message(ctx_);
  int action = 3;
  env_.act_discrete(ctx_, &action);
  double reward = 0.0;
  ASSERT_THAT(env_.advance(ctx_, 1, &reward),
              Eq(EnvCApi_EnvironmentStatus_Running))
      << env_.error_message(ctx_);
  EXPECT_THAT(reward, Eq(3.0));
}

TEST_F(DmLab2DTest, CanActContinuous) {
  ASSERT_THAT(env_.setting(ctx_, "levelName", "examples/level_api"), Eq(0))
      << env_.error_message(ctx_);
  ASSERT_THAT(env_.init(ctx_), Eq(0)) << env_.error_message(ctx_);
  ASSERT_THAT(env_.start(ctx_, 0, 0), Eq(0)) << env_.error_message(ctx_);
  double action = -5;
  env_.act_continuous(ctx_, &action);
  double reward = 0.0;
  ASSERT_THAT(env_.advance(ctx_, 1, &reward),
              Eq(EnvCApi_EnvironmentStatus_Running))
      << env_.error_message(ctx_);
  EnvCApi_Observation obs;
  env_.observation(ctx_, 2, &obs);
  ASSERT_THAT(absl::MakeConstSpan(obs.spec.shape, obs.spec.dims),
              ElementsAre(3));
  ASSERT_THAT(obs.spec.type, Eq(EnvCApi_ObservationInt32s));
  EXPECT_THAT(absl::MakeConstSpan(obs.payload.int32s, 3),
              ElementsAre(-4, -3, -2));
}

TEST_F(DmLab2DTest, CanActText) {
  ASSERT_THAT(env_.setting(ctx_, "levelName", "examples/level_api"), Eq(0))
      << env_.error_message(ctx_);
  ASSERT_THAT(env_.init(ctx_), Eq(0)) << env_.error_message(ctx_);
  ASSERT_THAT(env_.start(ctx_, 0, 0), Eq(0)) << env_.error_message(ctx_);
  EnvCApi_TextAction text_action;
  text_action.data = "test";
  text_action.len = 4;
  env_.act_text(ctx_, &text_action);
  double reward = 0.0;
  ASSERT_THAT(env_.advance(ctx_, 1, &reward),
              Eq(EnvCApi_EnvironmentStatus_Running))
      << env_.error_message(ctx_);
  EnvCApi_Observation obs;
  env_.observation(ctx_, 4, &obs);
  ASSERT_THAT(absl::MakeConstSpan(obs.spec.shape, obs.spec.dims),
              ElementsAre(4));
  ASSERT_THAT(obs.spec.type, Eq(EnvCApi_ObservationString));
  EXPECT_THAT(absl::MakeConstSpan(obs.payload.string, 4),
              ElementsAre('t', 'e', 's', 't'));
}

TEST_F(DmLab2DTest, CanReadEvents) {
  ASSERT_THAT(env_.setting(ctx_, "levelName", "examples/level_api"), Eq(0))
      << env_.error_message(ctx_);
  ASSERT_THAT(env_.init(ctx_), Eq(0)) << env_.error_message(ctx_);
  ASSERT_THAT(env_.start(ctx_, 0, 0), Eq(0)) << env_.error_message(ctx_);
  ASSERT_THAT(env_.event_count(ctx_), Eq(1));
  EnvCApi_Event event = {};
  env_.event(ctx_, 0, &event);
  ASSERT_THAT(env_.event_type_count(ctx_), Gt(event.id));
  EXPECT_THAT(env_.event_type_name(ctx_, event.id), StrEq("start"));
  ASSERT_THAT(event.observation_count, Eq(1));
  const EnvCApi_Observation& obs = event.observations[0];
  ASSERT_THAT(absl::MakeConstSpan(obs.spec.shape, obs.spec.dims),
              ElementsAre(3));
  ASSERT_THAT(obs.spec.type, Eq(EnvCApi_ObservationInt64s));
  EXPECT_THAT(absl::MakeConstSpan(obs.payload.int64s, 3), ElementsAre(1, 2, 3));
}

TEST_F(DmLab2DTest, CanReadProperties) {
  ASSERT_THAT(env_.setting(ctx_, "levelName", "examples/level_api"), Eq(0))
      << env_.error_message(ctx_);
  ASSERT_THAT(env_.init(ctx_), Eq(0)) << env_.error_message(ctx_);
  const char* value = nullptr;
  ASSERT_THAT(env_.read_property(ctx_, "steps", &value),
              Eq(EnvCApi_PropertyResult_Success));
  EXPECT_THAT(value, StrEq("10"));
}

TEST_F(DmLab2DTest, CanWriteProperties) {
  ASSERT_THAT(env_.setting(ctx_, "levelName", "examples/level_api"), Eq(0))
      << env_.error_message(ctx_);
  ASSERT_THAT(env_.init(ctx_), Eq(0)) << env_.error_message(ctx_);
  ASSERT_THAT(env_.write_property(ctx_, "steps", "2"),
              Eq(EnvCApi_PropertyResult_Success));
  ASSERT_THAT(env_.start(ctx_, 0, 0), Eq(0)) << env_.error_message(ctx_);
  double reward = 0.0;
  ASSERT_THAT(env_.advance(ctx_, 1, &reward),
              Eq(EnvCApi_EnvironmentStatus_Running))
      << env_.error_message(ctx_);
  ASSERT_THAT(env_.advance(ctx_, 1, &reward),
              Eq(EnvCApi_EnvironmentStatus_Terminated))
      << env_.error_message(ctx_);
}

TEST_F(DmLab2DTest, CanListProperties) {
  ASSERT_THAT(env_.setting(ctx_, "levelName", "examples/level_api"), Eq(0))
      << env_.error_message(ctx_);
  ASSERT_THAT(env_.init(ctx_), Eq(0)) << env_.error_message(ctx_);
  struct Results {
    std::vector<std::string> names;
    std::vector<EnvCApi_PropertyAttributes> attributes;
  };

  auto callback = +[](void* userdata, const char* key,
                      EnvCApi_PropertyAttributes attributes) {
    auto* results_ptr = static_cast<Results*>(userdata);
    results_ptr->names.emplace_back(key);
    results_ptr->attributes.emplace_back(attributes);
  };

  Results result;
  ASSERT_THAT(env_.list_property(ctx_, &result, "", callback),
              Eq(EnvCApi_PropertyResult_Success));

  EXPECT_THAT(result.names, ElementsAre("steps"));
  EXPECT_THAT(result.attributes,
              ElementsAre(EnvCApi_PropertyAttributes_ReadWritable));
}

TEST_F(DmLab2DTest, CanApplySettings) {
  ASSERT_THAT(env_.setting(ctx_, "levelName", "examples/level_api"), Eq(0))
      << env_.error_message(ctx_);

  ASSERT_THAT(env_.setting(ctx_, "steps", "2"), Eq(0))
      << env_.error_message(ctx_);
  ASSERT_THAT(env_.init(ctx_), Eq(0)) << env_.error_message(ctx_);
  ASSERT_THAT(env_.start(ctx_, 0, 0), Eq(0)) << env_.error_message(ctx_);
  double reward = 0.0;
  ASSERT_THAT(env_.advance(ctx_, 1, &reward),
              Eq(EnvCApi_EnvironmentStatus_Running))
      << env_.error_message(ctx_);
  ASSERT_THAT(env_.advance(ctx_, 1, &reward),
              Eq(EnvCApi_EnvironmentStatus_Terminated))
      << env_.error_message(ctx_);
}

TEST_F(DmLab2DTest, FailOnApplyingInvalidSettings) {
  ASSERT_THAT(env_.setting(ctx_, "levelName", "examples/level_api"), Eq(0))
      << env_.error_message(ctx_);

  ASSERT_THAT(env_.setting(ctx_, "unkown", "blah"), Eq(0))
      << env_.error_message(ctx_);

  ASSERT_THAT(env_.init(ctx_), Eq(2));
  EXPECT_THAT(absl::string_view(env_.error_message(ctx_)),
              HasSubstr("Invalid setting unkown=blah"));
}

}  // namespace
