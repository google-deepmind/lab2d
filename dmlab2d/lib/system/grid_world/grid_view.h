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
//
////////////////////////////////////////////////////////////////////////////////

#ifndef DMLAB2D_LIB_SYSTEM_GRID_WORLD_GRID_VIEW_H_
#define DMLAB2D_LIB_SYSTEM_GRID_WORLD_GRID_VIEW_H_

#include <algorithm>
#include <memory>
#include <vector>

#include "absl/types/span.h"
#include "dmlab2d/lib/system/grid_world/collections/fixed_handle_map.h"
#include "dmlab2d/lib/system/grid_world/grid_window.h"
#include "dmlab2d/lib/system/grid_world/handles.h"
#include "dmlab2d/lib/system/grid_world/sprite_instance.h"
#include "dmlab2d/lib/system/math/math2d.h"

namespace deepmind::lab2d {

// Stores the view on to a grid with viewer specific SpriteInstance conversions.
class GridView {
 public:
  GridView(const GridWindow& window, int num_render_layers,
           FixedHandleMap<Sprite, Sprite> sprite_map,
           Sprite out_of_bounds_sprite, Sprite out_of_view_sprite)
      : window_(window),
        sprite_map_(std::move(sprite_map)),
        num_render_layers_(num_render_layers),
        out_of_bounds_sprite_(out_of_bounds_sprite),
        out_of_view_sprite_(out_of_view_sprite) {}

  const GridWindow& GetWindow() const { return window_; }
  int NumRenderLayers() const { return num_render_layers_; }
  int NumCells() const { return num_render_layers_ * window_.size2d().Area(); }
  Sprite OutOfBoundsSprite() const { return out_of_bounds_sprite_; }
  Sprite OutOfViewSprite() const { return out_of_view_sprite_; }

  // Sets all sprite_ids outside the players view area to
  // `out_of_view_sprite()`.
  //
  // Precondition: `sprite_ids.size()` equals: `NumCells()`.
  void ClearOutOfViewSprites(math::Orientation2d orientation,
                             absl::Span<int> sprite_ids) const;

  int ToSpriteId(const SpriteInstance& sprite) const {
    return sprite.handle.IsEmpty()
               ? 0
               : sprite_map_[sprite.handle].Value() * 4 + 1 +
                     static_cast<unsigned int>(sprite.orientation);
  }

  int NumSpriteIds() const { return sprite_map_.size() * 4 + 1; }

 private:
  GridWindow window_;
  FixedHandleMap<Sprite, Sprite> sprite_map_;
  int num_render_layers_;
  Sprite out_of_bounds_sprite_;
  Sprite out_of_view_sprite_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_GRID_WORLD_GRID_VIEW_H_
