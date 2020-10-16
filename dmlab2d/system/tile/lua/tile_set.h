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

#ifndef DMLAB2D_SYSTEM_GRID_WORLD_SPRITE_RENDERER_LUA_TILE_PALETTE_H_
#define DMLAB2D_SYSTEM_GRID_WORLD_SPRITE_RENDERER_LUA_TILE_PALETTE_H_

#include <memory>
#include <string>

#include "dmlab2d/lua/class.h"
#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/n_results_or.h"
#include "dmlab2d/lua/ref.h"
#include "dmlab2d/system/tile/tile_set.h"

namespace deepmind::lab2d {

// Lua interface for constructing and modifying a TileSet.
class LuaTileSet : lua::Class<LuaTileSet> {
  friend class Class;
  static const char* ClassName() { return "tile.set"; }

 public:
  static void Register(lua_State* L);
  static lua::NResultsOr Create(lua_State* L);
  const TileSet& tile_set() { return tile_set_; }

 private:
  explicit LuaTileSet(std::vector<std::string> sprite_names, TileSet tile_set)
      : sprite_names_(std::move(sprite_names)),
        tile_set_(std::move(tile_set)) {}

  lua::NResultsOr SetSprite(lua_State* L);
  lua::NResultsOr SpriteShape(lua_State* L);
  lua::NResultsOr SpriteNames(lua_State* L);

  std::vector<std::string> sprite_names_;
  TileSet tile_set_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_SYSTEM_GRID_WORLD_SPRITE_RENDERER_LUA_TILE_PALETTE_H_
