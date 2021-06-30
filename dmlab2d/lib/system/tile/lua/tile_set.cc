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

#include "dmlab2d/lib/system/tile/lua/tile_set.h"

#include <memory>
#include <utility>

#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/string_view.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/lua/ref.h"
#include "dmlab2d/lib/lua/table_ref.h"
#include "dmlab2d/lib/system/math/lua/math2d.h"
#include "dmlab2d/lib/system/math/math2d.h"
#include "dmlab2d/lib/system/tensor/lua/tensor.h"
#include "dmlab2d/lib/system/tile/tile_set.h"

namespace deepmind::lab2d {

void LuaTileSet::Register(lua_State* L) {
  const Class::Reg methods[] = {
      {"setSprite", &Class::Member<&LuaTileSet::SetSprite>},
      {"shape", &Class::Member<&LuaTileSet::SpriteShape>},
      {"names", &Class::Member<&LuaTileSet::SpriteNames>},
  };
  Class::Register(L, methods);
}

lua::NResultsOr LuaTileSet::SpriteNames(lua_State* L) {
  lua::Push(L, sprite_names_);
  return 1;
}

lua::NResultsOr LuaTileSet::SpriteShape(lua_State* L) {
  Push(L, tile_set_.sprite_shape());
  return 1;
}

lua::NResultsOr LuaTileSet::SetSprite(lua_State* L) {
  lua::TableRef table;
  if (!IsFound(lua::Read(L, 2, &table))) {
    return "Arg 1 must be a kwarg table!";
  }

  absl::string_view name;
  if (!IsFound(table.LookUp("name", &name))) {
    return "'name' must be a string";
  }

  tensor::LuaTensor<unsigned char>* image;
  if (!IsFound(table.LookUp("image", &image))) {
    return "'image' - Must supply ByteTensor(h, w, 3 or 4) or "
           " ByteTensor(count, h, w, 3 or 4)!";
  }

  const auto& image_view = image->tensor_view();

  int sprites_set = 0;
  for (std::size_t sprite_id = 0; sprite_id < sprite_names_.size();
       ++sprite_id) {
    absl::string_view suffix = sprite_names_[sprite_id];

    if (absl::StartsWith(suffix, name)) {
      suffix.remove_prefix(name.size());
      if (suffix.empty() || suffix[0] == '.') {
        tensor::TensorView<unsigned char> facing = image_view;
        if (facing.shape().size() == 4) {
          facing.Select(0, sprites_set);
        }
        if (!tile_set_.SetSprite(sprite_id, facing)) {
          return absl::StrFormat(
              "Error occured when setting sprite '%s%s' to %s", name, suffix,
              absl::FormatStreamed(facing));
        }
        ++sprites_set;
      }
    }
  }
  if (image_view.shape().size() == 4 && sprites_set != image_view.shape()[0]) {
    return absl::StrFormat(
        "Mismatch count of sprites with prefix '%s'; Required: %d, Actual: %d.",
        name, image_view.shape()[0], sprites_set);
  }
  lua::Push(L, sprites_set);
  return 1;
}

lua::NResultsOr LuaTileSet::Create(lua_State* L) {
  lua::TableRef table;
  if (!IsFound(lua::Read(L, 1, &table))) {
    return "[tile.set] - Arg 1 must be a table.";
  }

  math::Size2d sprite_shape;
  std::vector<std::string> sprite_names;
  if (!IsFound(table.LookUp("names", &sprite_names))) {
    return "[tile.set] - 'names' must be an array of strings.";
  }
  if (!IsFound(table.LookUp("shape", &sprite_shape))) {
    return "[tile.set] - 'shape' must be a table containing height and width.";
  }
  lua_pop(L, 1);
  CreateObject(L, std::move(sprite_names),
               TileSet(sprite_names.size(), sprite_shape));
  return 1;
}

}  // namespace deepmind::lab2d
