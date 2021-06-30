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

#include <string>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "dmlab2d/lib/dmlab2d.h"
#include "dmlab2d/lib/util/test_srcdir.h"
#include "gtest/gtest.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind::lab2d {
namespace {

const char* test_script = nullptr;
const char* levels_root = nullptr;

TEST(LuaUnitTest, RunsTest) {
  const std::string runfiles_root = util::TestSrcDir();
  ASSERT_NE(test_script, nullptr) << "Must pass test_script as first arg!";
  DeepMindLab2DLaunchParams params = {};
  params.runfiles_root = runfiles_root.c_str();
  EnvCApi env_c_api;
  void* context;
  ASSERT_EQ(dmlab2d_connect(&params, &env_c_api, &context), 0);
  if (levels_root) {
    ASSERT_EQ(env_c_api.setting(context, "levelDirectory", levels_root), 0);
  }
  ASSERT_EQ(env_c_api.setting(context, "levelName", test_script), 0);
  if (env_c_api.init(context) != 0) {
    ADD_FAILURE() << env_c_api.error_message(context);
  }
  env_c_api.release_context(context);
}

}  // namespace
}  // namespace deepmind::lab2d

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  if (argc > 1 && argv[1][0] != '\0') {
    deepmind::lab2d::test_script = argv[1];
  }
  if (argc > 2 && argv[2][0] != '\0') {
    deepmind::lab2d::levels_root = argv[2];
  }
  return RUN_ALL_TESTS();
}
