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

#ifndef DMLAB2D_LIB_SYSTEM_GRID_WORLD_LUA_LUA_GRID_H_
#define DMLAB2D_LIB_SYSTEM_GRID_WORLD_LUA_LUA_GRID_H_

#include "absl/types/optional.h"
#include "dmlab2d/lib/lua/class.h"
#include "dmlab2d/lib/lua/lua.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/ref.h"
#include "dmlab2d/lib/lua/table_ref.h"
#include "dmlab2d/lib/system/grid_world/grid.h"
#include "dmlab2d/lib/system/grid_world/world.h"

namespace deepmind::lab2d {

// Creates and modify a Grid from Lua.
class LuaGrid : public lua::Class<LuaGrid> {
  friend class Class;
  static const char* ClassName() { return "Grid"; }

 public:
  static void SubModule(lua::TableRef module);
  static void Register(lua_State* L);

  // Creates a LuaGrid on the Lua stack. `world_ref` must refer to a Lua object
  // holding `world` to prevent LuaGrid from being invalidated.
  static lua::NResultsOr CreateGrid(lua_State* L, const World& world,
                                    lua::Ref world_ref);

  const Grid& GetGrid() const { return *grid_; }
  Grid* GetMutableGrid() { return &(*grid_); }

 private:
  // Called from lua::Class.
  bool IsValidObject() { return grid_.has_value(); }

  explicit LuaGrid(Grid grid, lua::Ref world_ref);
  lua::NResultsOr Destroy(lua_State* L);

  lua::NResultsOr DoUpdate(lua_State* L);
  lua::NResultsOr CreateLayout(lua_State* L);
  lua::NResultsOr CreatePiece(lua_State* L);
  lua::NResultsOr RemovePiece(lua_State* L);
  lua::NResultsOr Transform(lua_State* L);
  lua::NResultsOr Position(lua_State* L);
  lua::NResultsOr GetState(lua_State* L);
  lua::NResultsOr GetLayer(lua_State* L);
  lua::NResultsOr GetUserState(lua_State* L);
  lua::NResultsOr Frames(lua_State* L);
  lua::NResultsOr ToString(lua_State* L);

  lua::NResultsOr RotatePiece(lua_State* L);
  lua::NResultsOr SetPieceOrientation(lua_State* L);
  lua::NResultsOr TeleportPiece(lua_State* L);
  lua::NResultsOr PushPieceRelative(lua_State* L);
  lua::NResultsOr PushGridRelative(lua_State* L);
  lua::NResultsOr PushPiece(lua_State* L, Grid::Perspective perspective);
  lua::NResultsOr SetUpdater(lua_State* L);
  lua::NResultsOr SetState(lua_State* L);
  lua::NResultsOr SetUserState(lua_State* L);
  lua::NResultsOr TeleportToGroup(lua_State* L);
  lua::NResultsOr HitBeam(lua_State* L);

  // Convert.
  lua::NResultsOr ToRelativeDirection(lua_State* L);
  lua::NResultsOr ToRelativePosition(lua_State* L);
  lua::NResultsOr ToAbsoluteDirection(lua_State* L);
  lua::NResultsOr ToAbsolutePosition(lua_State* L);

  // Query Grid.
  lua::NResultsOr RayCast(lua_State* L);
  lua::NResultsOr RayCastDirection(lua_State* L);
  lua::NResultsOr QueryPosition(lua_State* L);
  lua::NResultsOr QueryRectangle(lua_State* L);
  lua::NResultsOr QueryDiamond(lua_State* L);
  lua::NResultsOr QueryDisc(lua_State* L);

  // Group.
  lua::NResultsOr GroupCount(lua_State* L);
  lua::NResultsOr GroupRandom(lua_State* L);
  lua::NResultsOr GroupShuffled(lua_State* L);
  lua::NResultsOr GroupShuffledWithCount(lua_State* L);
  lua::NResultsOr GroupShuffledWithProbability(lua_State* L);

  // Connect.
  lua::NResultsOr Connect(lua_State* L);
  lua::NResultsOr Disconnect(lua_State* L);
  lua::NResultsOr DisconnectAll(lua_State* L);

  absl::optional<Grid> grid_;

  // Required to keep `grid_` valid.
  lua::Ref world_ref_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_GRID_WORLD_LUA_LUA_GRID_H_
