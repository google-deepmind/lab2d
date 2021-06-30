// Copyright (C) 2020 The DMLab2D Authors.
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

#ifndef DMLAB2D_LIB_SYSTEM_GENERATORS_PUSHBOX_LUA_PUSHBOX_H_
#define DMLAB2D_LIB_SYSTEM_GENERATORS_PUSHBOX_LUA_PUSHBOX_H_

#include "dmlab2d/lib/lua/lua.h"

namespace deepmind::lab2d {
// Returns a table of pushbox related functions:
// * generate{
//       seed = <int>
//       width = <int>,
//       height = <int>,
//       numBoxes = <int>,
//       roomSteps = [optional] <int>,
//       roomSeed = [optional] <unsigned int>,
//       targetsSeed = [optional] <unsigned int>,
//       actionsSeed = [optional] <unsigned int>
//   }
int LuaPushboxRequire(lua_State* L);

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_GENERATORS_PUSHBOX_LUA_PUSHBOX_H_
