#include "dmlab2d/lib/system/tile/tile_renderer.h"
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

#ifndef DMLAB2D_LIB_SYSTEM_GRID_WORLD_SPRITE_RENDERER_LUA_TILE_SCENE_H_
#define DMLAB2D_LIB_SYSTEM_GRID_WORLD_SPRITE_RENDERER_LUA_TILE_SCENE_H_

#include <utility>

#include "absl/types/span.h"
#include "dmlab2d/lib/lua/class.h"
#include "dmlab2d/lib/lua/lua.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/table_ref.h"
#include "dmlab2d/lib/system/tensor/tensor_view.h"
#include "dmlab2d/lib/system/tile/tile_set.h"

namespace deepmind::lab2d {

// Holds a scene and TileSet that can render a grid of a given size.
class LuaTileScene : lua::Class<LuaTileScene> {
  friend class Class;
  static const char* ClassName() { return "tile.scene"; }

 public:
  static lua::NResultsOr Create(lua_State* L);
  static void Register(lua_State* L);

 private:
  lua::NResultsOr Shape(lua_State* L);
  lua::NResultsOr Render(lua_State* L);

  explicit LuaTileScene(math::Size2d grid_shape, absl::Span<Pixel> scene,
                        lua::TableRef scene_ref, const TileSet* tile_set,
                        lua::TableRef tile_set_ref)
      : grid_shape_(grid_shape),
        scene_(scene),
        scene_ref_(std::move(scene_ref)),
        sprite_renderer_(tile_set),
        tile_set_ref_(std::move(tile_set_ref)) {}

  math::Size2d grid_shape_;
  absl::Span<Pixel> scene_;
  lua::TableRef scene_ref_;

  TileRenderer sprite_renderer_;
  lua::TableRef tile_set_ref_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_GRID_WORLD_SPRITE_RENDERER_LUA_TILE_SCENE_H_
