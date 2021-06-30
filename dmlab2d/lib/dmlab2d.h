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

#ifndef DMLAB2D_LIB_DMLAB2D_H_
#define DMLAB2D_LIB_DMLAB2D_H_

#include "third_party/rl_api/env_c_api.h"

#ifdef __cplusplus
extern "C" {
#endif

struct DeepMindLab2DLaunchParams_s;
typedef struct DeepMindLab2DLaunchParams_s DeepMindLab2DLaunchParams;

struct DeepMindLab2DLaunchParams_s {
  // Path to where DeepMind Lab2D assets are stored.
  const char* runfiles_root;
};

int dmlab2d_connect(const DeepMindLab2DLaunchParams* params, EnvCApi* env_c_api,
                    void** context);

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // DMLAB2D_LIB_DMLAB2D_H_
