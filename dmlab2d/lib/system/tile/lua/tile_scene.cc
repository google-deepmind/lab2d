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

#include "dmlab2d/lib/system/tile/lua/tile_scene.h"

#include <algorithm>

#include "absl/strings/str_format.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/lua/table_ref.h"
#include "dmlab2d/lib/system/math/lua/math2d.h"
#include "dmlab2d/lib/system/math/math2d.h"
#include "dmlab2d/lib/system/tensor/lua/tensor.h"
#include "dmlab2d/lib/system/tensor/tensor_view.h"
#include "dmlab2d/lib/system/tile/lua/tile_set.h"
#include "dmlab2d/lib/system/tile/pixel.h"

namespace deepmind::lab2d {

void LuaTileScene::Register(lua_State* L) {
  const Class::Reg methods[] = {
      {"shape", &Class::Member<&LuaTileScene::Shape>},
      {"render", &Class::Member<&LuaTileScene::Render>},
  };
  Class::Register(L, methods);
}

lua::NResultsOr LuaTileScene::Shape(lua_State* L) {
  scene_ref_.PushMemberFunction("shape");
  return lua::Call(L, 1);
}

lua::NResultsOr LuaTileScene::Render(lua_State* L) {
  tensor::LuaTensor<int>* grid;
  if (!IsFound(Read(L, 2, &grid))) {
    return absl::StrFormat("Argument 1 must be an Int32Tensor; actual: '%s'",
                           lua::ToString(L, 2));
  }
  const auto& tensor_view = grid->tensor_view();
  if (!tensor_view.IsContiguous()) {
    return "Argument 1 must be contiguous!";
  }
  const auto& grid_shape_in = grid->tensor_view().shape();
  std::array<std::size_t, 3> grid_shape = {1, 1, 1};
  if (grid_shape_in.size() > 3) {
    return absl::StrFormat(
        "Argument 1 grid shape must be {%d[, %d[, layers]]}!, actual: '%s'",
        grid_shape_.height, grid_shape_.width, lua::ToString(L, 2));
  }
  std::copy(grid_shape_in.begin(), grid_shape_in.end(), grid_shape.begin());
  if (grid_shape[0] != grid_shape_.height ||
      grid_shape[1] != grid_shape_.width) {
    return absl::StrFormat(
        "Argument 1 grid shape must be {%d[, %d[, layers]]}!, actual: '%s'",
        grid_shape_.height, grid_shape_.width, lua::ToString(L, 2));
  }

  const auto& layer_view = grid->tensor_view();
  sprite_renderer_.Render(
      absl::MakeConstSpan(layer_view.storage() + layer_view.start_offset(),
                          layer_view.num_elements()),
      grid_shape, scene_);

  lua::Push(L, scene_ref_);
  return 1;
}

lua::NResultsOr LuaTileScene::Create(lua_State* L) {
  lua::TableRef table;

  if (!IsFound(lua::Read(L, 1, &table))) {
    return "[tile.scene] - Arg 1 must be a table containing 'shape' and 'set'.";
  }

  math::Size2d grid_shape;
  if (!IsFound(table.LookUp("shape", &grid_shape)) || grid_shape.height < 0 ||
      grid_shape.width < 0) {
    return "[tile.scene] - 'shape' must be a table with non-negative width an "
           "height";
  }

  LuaTileSet* lua_tile_set;
  if (!IsFound(table.LookUp("set", &lua_tile_set))) {
    return "[tile.scene] - 'set' must be a tile.set.";
  }

  lua::TableRef tile_set_ref;
  CHECK(IsFound(table.LookUp("set", &tile_set_ref)))
      << "[tile.scene] - Internal error";

  const auto& tile_set = lua_tile_set->tile_set();
  math::Size2d sprite_shape = tile_set.sprite_shape();

  std::size_t scene_height = grid_shape.height * sprite_shape.height;
  std::size_t scene_width = grid_shape.width * sprite_shape.width;
  auto storage = std::make_shared<tensor::StorageVector<Pixel>>(scene_height *
                                                                scene_width);
  // Storage type is Pixel but tensor type is unsigned char.
  // We reinterpret_cast the pixel array to an unsigned char* and add
  // a rank of sizeof(Pixel) to account for the number of elements.
  tensor::TensorView<unsigned char> tensor_view(
      tensor::Layout(
          tensor::ShapeVector{scene_height, scene_width, sizeof(Pixel)}),
      reinterpret_cast<unsigned char*>(storage->mutable_data()->data()));

  auto scene = absl::MakeSpan(*storage->mutable_data());
  tensor::LuaTensor<unsigned char>::CreateObject(L, std::move(tensor_view),
                                                 std::move(storage));
  lua::TableRef scene_ref;

  lua::Read(L, -1, &scene_ref);
  lua_pop(L, 1);
  CreateObject(L, grid_shape, scene, std::move(scene_ref), &tile_set,
               std::move(tile_set_ref));
  return 1;
}

}  // namespace deepmind::lab2d
