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

#ifndef DMLAB2D_LIB_SYSTEM_TILE_LUA_TILE_H_
#define DMLAB2D_LIB_SYSTEM_TILE_LUA_TILE_H_

#include "dmlab2d/lib/lua/lua.h"

namespace deepmind::lab2d {

// Registers Lua classes `LuaTileScene` and `LuaTileSet` and returns their
// respective constructors 'scene' and 'set' in a table on the Lua stack.
int LuaTileModule(lua_State* L);

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_TILE_LUA_TILE_H_
