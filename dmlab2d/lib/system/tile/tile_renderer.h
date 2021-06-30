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

// Defines `TileRenderer`, a class for rendering TileSet sprites to a scene.

#ifndef DMLAB2D_LIB_SYSTEM_TILE_TILE_RENDERER_H_
#define DMLAB2D_LIB_SYSTEM_TILE_TILE_RENDERER_H_

#include <cstdint>
#include <vector>

#include "absl/types/span.h"
#include "dmlab2d/lib/system/math/math2d.h"
#include "dmlab2d/lib/system/tile/pixel.h"
#include "dmlab2d/lib/system/tile/tile_set.h"

namespace deepmind::lab2d {

// An object for blending and placing sprites from a TileSet onto a scene.
class TileRenderer {
 public:
  // A referene to `*tile_set` is stored, which must hence out-live this object.
  explicit TileRenderer(const TileSet* tile_set)
      : tile_set_(*tile_set),
        empty_(tile_set_.sprite_pixels(), Pixel::Black()),
        pixels_(tile_set_.sprite_pixels()) {}

  math::Size2d sprite_shape() const { return tile_set_.sprite_shape(); }

  // Renders grid using tile_set to a scene.
  // `grid_shape` must have 3 elements and `grid.size()` must be equal to:
  // grid_shape[0] * grid_shape[1] * grid_shape[2].
  // `scene.size()` must be equal to:
  // sprite_shape().height() * sprit_shape().width() * grid_shape[0] * grid[1].
  void Render(absl::Span<const std::int32_t> grid,
              absl::Span<const std::size_t> grid_shape,
              absl::Span<Pixel> scene);

 private:
  // Alpha-blends the sprites in sprite_indices into a scratch space and
  // returns a reference to that blended data. The reference is only valid until
  // next call to MakeSprite.
  absl::Span<const Pixel> MakeSprite(absl::Span<const int> sprite_indices);

  const TileSet& tile_set_;

  std::vector<Pixel> empty_;

  // Temporary storage of blended sprites.
  std::vector<Pixel> pixels_;
  std::vector<int> sprite_indices_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_TILE_TILE_RENDERER_H_
