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
#include "dmlab2d/lib/dmlab2d.h"
#include "dmlab2d/lib/util/test_srcdir.h"
#include "gtest/gtest.h"
#include "third_party/rl_api/env_c_api.h"
#include "third_party/rl_api/env_c_api_test_suite.h"

namespace deepmind::lab2d {
namespace {

const char* level_name;
const char* levels_root;

using ::rl_api::EnvCApiConformanceTestSuite;
using ::rl_api::EnvCApiTestParam;

int test_connect(EnvCApi* api, void** context) {
  const std::string runfiles_root = util::TestSrcDir();
  DeepMindLab2DLaunchParams params = {};
  params.runfiles_root = runfiles_root.c_str();
  if (int err = dmlab2d_connect(&params, api, context)) {
    return err;
  }

  if (levels_root) {
    api->setting(*context, "levelDirectory", levels_root);
  }
  if (level_name) {
    api->setting(*context, "levelName", level_name);
  }
  return 0;
}

INSTANTIATE_TEST_SUITE_P(MyEnvConformanceTest, EnvCApiConformanceTestSuite,
                         testing::Values(EnvCApiTestParam{test_connect}));

}  // namespace
}  // namespace deepmind::lab2d

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  if (argc > 1 && argv[1][0] != '\0') {
    deepmind::lab2d::level_name = argv[1];
  }
  if (argc > 2 && argv[2][0] != '\0') {
    deepmind::lab2d::levels_root = argv[2];
  }
  return RUN_ALL_TESTS();
}
