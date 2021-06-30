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

#include "dmlab2d/lib/system/grid_world/lua/lua_grid_view.h"

#include <algorithm>
#include <array>
#include <cstddef>
#include <string>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "absl/types/variant.h"
#include "dmlab2d/lib/lua/class.h"
#include "dmlab2d/lib/lua/lua.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/lua/ref.h"
#include "dmlab2d/lib/lua/table_ref.h"
#include "dmlab2d/lib/system/grid_world/collections/fixed_handle_map.h"
#include "dmlab2d/lib/system/grid_world/grid.h"
#include "dmlab2d/lib/system/grid_world/grid_view.h"
#include "dmlab2d/lib/system/grid_world/handles.h"
#include "dmlab2d/lib/system/grid_world/lua/lua_grid.h"
#include "dmlab2d/lib/system/grid_world/lua/lua_handle.h"
#include "dmlab2d/lib/system/grid_world/sprite_instance.h"
#include "dmlab2d/lib/system/grid_world/text_tools.h"
#include "dmlab2d/lib/system/math/lua/math2d.h"
#include "dmlab2d/lib/system/math/math2d.h"
#include "dmlab2d/lib/system/tensor/lua/tensor.h"
#include "dmlab2d/lib/system/tensor/tensor_view.h"

namespace deepmind::lab2d {

void LuaGridView::Register(lua_State* L) {
  const Class::Reg methods[] = {
      {"observationSpec", &Class::Member<&LuaGridView::ObservationSpec>},
      {"observation", &Class::Member<&LuaGridView::Observation>},
      {"gridSize", &Class::Member<&LuaGridView::GridSize>},
  };
  Class::Register(L, methods);
}

lua::NResultsOr LuaGridView::GridSize(lua_State* L) {
  math::Size2d grid_size;
  grid_size.height = view_.GetWindow().height();
  grid_size.width = view_.GetWindow().width();
  Push(L, grid_size);
  return 1;
}

lua::NResultsOr LuaGridView::ObservationSpec(lua_State* L) {
  absl::string_view name;
  if (!IsFound(lua::Read(L, 2, &name))) {
    return absl::StrCat("Arg 1 expect string, actual '", lua::ToString(L, 2),
                        "'");
  }
  lua::TableRef table = lua::TableRef::Create(L);
  table.Insert("name", name);
  table.Insert("type", "tensor.Int32Tensor");
  std::array<int, 3> shape = {view_.GetWindow().height(),
                              view_.GetWindow().width(),
                              view_.NumRenderLayers()};
  table.Insert("shape", shape);
  lua::Push(L, table);
  return 1;
}

lua::NResultsOr LuaGridView::Observation(lua_State* L) {
  lua::TableRef table;
  if (!IsFound(lua::Read(L, 2, &table))) {
    return "Must supply a table as first argument to observation!";
  }

  LuaGrid* lua_grid;
  if (!IsFound(table.LookUp("grid", &lua_grid))) {
    return "Must supply argument 'grid'!";
  }

  Piece piece_handle;
  if (IsTypeMismatch(table.LookUp("piece", &piece_handle))) {
    return "'piece' must be a PieceHandle!";
  }
  math::Transform2d transform{{0, 0}, math::Orientation2d::kNorth};
  bool render_level = true;
  if (!piece_handle.IsEmpty()) {
    render_level = !lua_grid->GetGrid().GetLayer(piece_handle).IsEmpty();
    if (render_level) {
      transform = lua_grid->GetGrid().GetPieceTransform(piece_handle);
      if (transform.position.x < 0 || transform.position.y < 0) {
        render_level = false;
      }
    }
  } else if (IsTypeMismatch(table.LookUp("transform", &transform))) {
    return "Invalid 'transform'. Must be in the form "
           "{pos = {x, y}, orientation = d} where x, y are coordinates and d "
           "is one of 'N', 'E', 'S' and 'W'!";
  }

  math::Orientation2d player_orientation = transform.orientation;
  if (IsTypeMismatch(table.LookUp("orientation", &transform.orientation))) {
    return "'orientation' must be one of 'N', 'E', 'S' and 'W'!";
  }
  if (render_level) {
    lua_grid->GetMutableGrid()->Render(transform, view_, grid_);
  } else {
    auto out_of_bounds_id = view_.ToSpriteId(
        SpriteInstance{view_.OutOfBoundsSprite(), player_orientation});
    std::fill(grid_.begin(), grid_.end(), out_of_bounds_id);
  }
  view_.ClearOutOfViewSprites(
      FromView(transform.orientation, player_orientation), grid_);

  lua::Push(L, tensor_ref_);
  return 1;
}

lua::NResultsOr LuaGridView::CreateLayerView(lua_State* L, const World& world) {
  lua::TableRef table;
  if (!IsFound(lua::Read(L, 2, &table))) {
    return "Arg 1 must be a table!";
  }

  absl::string_view layout;
  if (IsTypeMismatch(table.LookUp("layout", &layout))) {
    return "'layout' must be a string.";
  }

  int left, right, forward, backward;

  bool centered = false;
  if (!layout.empty()) {
    left = 0;
    forward = 0;
    math::Size2d layout_size = GetSize2dOfText(layout);
    right = layout_size.width - 1;
    backward = layout_size.height - 1;
  } else if (!IsFound(table.LookUp("left", &left)) ||
             !IsFound(table.LookUp("right", &right)) ||
             !IsFound(table.LookUp("forward", &forward)) ||
             !IsFound(table.LookUp("backward", &backward))) {
    return "'left', 'right', 'forward' and 'backward' must be integers.";
  }

  if (IsTypeMismatch(table.LookUp("centered", &centered))) {
    return "'centered' must be a boolean";
  }

  // Create identity mapping.
  FixedHandleMap<Sprite, Sprite> sprite_map(world.sprites().NumElements());
  for (auto [sprite, name] : world.sprites()) {
    sprite_map[sprite] = sprite;
  }

  absl::flat_hash_map<std::string, std::string> custom_sprite_map;
  if (IsTypeMismatch(table.LookUp("spriteMap", &custom_sprite_map))) {
    return "'spriteMap' must be string string table!";
  }

  for (const auto& [key, val] : custom_sprite_map) {
    Sprite handle_from = world.sprites().ToHandle(key);
    if (handle_from.IsEmpty()) {
      return absl::StrCat("Invalid source sprite in `spriteMap`: '", key, "'");
    }
    Sprite handle_to = world.sprites().ToHandle(val);
    if (handle_to.IsEmpty()) {
      return absl::StrCat("Invalid target sprite in `spriteMap`: '", val, "'");
    }
    sprite_map[handle_from] = handle_to;
  }

  GridWindow window(/*centered=*/centered, /*left=*/left,
                    /*right=*/right,
                    /*forward=*/forward, /*backward=*/backward);
  GridView view(
      /*window=*/window,
      /*num_render_layers=*/world.NumRenderLayers(),
      /*sprite_map=*/std::move(sprite_map),
      /*out_of_bounds_sprite=*/world.out_of_bounds_sprite(),
      /*out_of_view_sprite=*/world.out_of_view_sprite());

  CreateObject(L, L, std::move(view));
  return 1;
}

LuaGridView::LuaGridView(lua_State* L, GridView view) : view_(std::move(view)) {
  tensor::ShapeVector shape = {
      static_cast<std::size_t>(view_.GetWindow().height()),
      static_cast<std::size_t>(view_.GetWindow().width()),
      static_cast<std::size_t>(view_.NumRenderLayers())};
  auto num_elements = tensor::Layout::num_elements(shape);
  std::vector<int> data(num_elements);
  auto* tenor_view =
      tensor::LuaTensor<int>::CreateObject(L, std::move(shape), std::move(data))
          ->mutable_tensor_view();
  grid_ =
      absl::MakeSpan(tenor_view->mutable_storage() + tenor_view->start_offset(),
                     tenor_view->num_elements());
  CHECK(IsFound(lua::Read(L, -1, &tensor_ref_))) << "Internal logic error!";
  lua_pop(L, 1);
}

}  // namespace deepmind::lab2d
