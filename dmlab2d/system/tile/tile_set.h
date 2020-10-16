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

// Defines `TileSet`, a class for storing sprites.

#ifndef DMLAB2D_SYSTEM_GRID_WORLD_SPRITE_RENDERER_TILE_SET_H_
#define DMLAB2D_SYSTEM_GRID_WORLD_SPRITE_RENDERER_TILE_SET_H_

#include <cstddef>
#include <vector>

#include "absl/types/span.h"
#include "dmlab2d/system/math/math2d.h"
#include "dmlab2d/system/tensor/tensor_view.h"
#include "dmlab2d/system/tile/pixel.h"

namespace deepmind::lab2d {

// A TileSet is a collection sprites all of which are the same shape.
class TileSet {
 public:
  // Contains metadata to improve rendering speed.
  enum class SpriteMetaData {
    kInvisible,
    kOpaqueConstRgb,
    kOpaque,
    kSemiTransparentConstRgbAlpha,
    kSemiTransparentConstRgb,
    kSemiTransparentConstAlpha,
    kSemiTransparent,
    kOneBitAlphaConstRgb,
    kOneBitAlpha,
  };

  // Creates a collection of `number_of_sprites` invisible sprites all with
  // the same shape `sprite_shape`.
  TileSet(std::size_t number_of_sprites, math::Size2d sprite_shape)
      : sprite_shape_(sprite_shape),
        sprite_meta_data_(number_of_sprites, SpriteMetaData::kInvisible),
        sprite_data_rgb_(number_of_sprites * sprite_shape_.Area()),
        sprite_data_alpha_(number_of_sprites * sprite_shape_.Area(),
                           PixelByte::Max) {}

  std::size_t num_sprites() const { return sprite_meta_data_.size(); }
  math::Size2d sprite_shape() const { return sprite_shape_; }

  // Number of pixels in each sprite.
  std::size_t sprite_pixels() const { return sprite_shape_.Area(); }

  // Sets sprite at `index` with image_tensor and returns true if the shape of
  // `image_tensor` is: {sprites_size().height, sprites_size().width, 3 or 4}.
  // Otherwise returns false.
  // `index` shall be less than num_sprites().
  bool SetSprite(std::size_t index,
                 const tensor::TensorView<unsigned char>& image_tensor);

  // `index` shall be less than num_sprites().
  absl::Span<const Pixel> GetSpriteRgbData(std::size_t index) const {
    return absl::MakeConstSpan(
        sprite_data_rgb_.data() + sprite_pixels() * index, sprite_pixels());
  }

  // `index` shall be less than num_sprites().
  absl::Span<const PixelByte> GetSpriteAlphaData(std::size_t index) const {
    return absl::MakeConstSpan(
        sprite_data_alpha_.data() + sprite_pixels() * index, sprite_pixels());
  }

  // `index` shall be less than num_sprites().
  SpriteMetaData GetSpriteMetaData(std::size_t index) const {
    return sprite_meta_data_[index];
  }

 private:
  absl::Span<Pixel> MutableSpriteRgbData(std::size_t index) {
    return absl::MakeSpan(sprite_data_rgb_.data() + sprite_pixels() * index,
                          sprite_pixels());
  }

  absl::Span<PixelByte> MutableSpriteAlphaData(std::size_t index) {
    return absl::MakeSpan(sprite_data_alpha_.data() + sprite_pixels() * index,
                          sprite_pixels());
  }

  math::Size2d sprite_shape_;

  // Sprite data stored as a structure of arrays.
  std::vector<SpriteMetaData> sprite_meta_data_;
  std::vector<Pixel> sprite_data_rgb_;
  std::vector<PixelByte> sprite_data_alpha_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_SYSTEM_GRID_WORLD_SPRITE_RENDERER_TILE_SET_H_
