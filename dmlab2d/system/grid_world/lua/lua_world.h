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

#ifndef DMLAB2D_SYSTEM_GRID_WORLD_LUA_LUA_WORLD_H_
#define DMLAB2D_SYSTEM_GRID_WORLD_LUA_LUA_WORLD_H_

#include "dmlab2d/lua/class.h"
#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/n_results_or.h"
#include "dmlab2d/system/grid_world/world.h"

namespace deepmind::lab2d {

// For creating and storing World from Lua.
class LuaWorld : lua::Class<LuaWorld> {
  friend class Class;
  static const char* ClassName() { return "World"; }

 public:
  static int Module(lua_State* L);
  const World& world() { return world_; }

 private:
  explicit LuaWorld(const World::Args& args);
  static void Register(lua_State* L);
  static lua::NResultsOr Create(lua_State* L);
  lua::NResultsOr CreateGrid(lua_State* L);
  lua::NResultsOr CreateLayerView(lua_State* L);
  lua::NResultsOr SpriteNames(lua_State* L);
  const World world_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_SYSTEM_GRID_WORLD_LUA_LUA_WORLD_H_
