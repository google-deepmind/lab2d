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

#ifndef DMLAB2D_LIB_SYSTEM_GRID_WORLD_LUA_LUA_GRID_VIEW_H_
#define DMLAB2D_LIB_SYSTEM_GRID_WORLD_LUA_LUA_GRID_VIEW_H_

#include <string>

#include "absl/types/span.h"
#include "dmlab2d/lib/lua/class.h"
#include "dmlab2d/lib/lua/lua.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/ref.h"
#include "dmlab2d/lib/system/grid_world/grid_view.h"
#include "dmlab2d/lib/system/grid_world/handles.h"
#include "dmlab2d/lib/system/grid_world/world.h"
#include "dmlab2d/lib/system/math/math2d.h"

namespace deepmind::lab2d {

// Stores GridView and accompanying Int32Tensor for rendering part of a LuaGrid.
class LuaGridView : lua::Class<LuaGridView> {
  friend class Class;
  static const char* ClassName() { return "LayerView"; }

 public:
  static lua::NResultsOr CreateLayerView(lua_State* L, const World& world);
  static void Register(lua_State* L);
  static lua::NResultsOr Module(lua_State* L);

 private:
  explicit LuaGridView(lua_State* L, GridView GridView);
  static lua::NResultsOr Create(lua_State* L);

  lua::NResultsOr ObservationSpec(lua_State* L);
  lua::NResultsOr Observation(lua_State* L);
  lua::NResultsOr GridSize(lua_State* L);

  const GridView view_;
  absl::Span<int> grid_;
  lua::Ref tensor_ref_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_GRID_WORLD_LUA_LUA_GRID_VIEW_H_
