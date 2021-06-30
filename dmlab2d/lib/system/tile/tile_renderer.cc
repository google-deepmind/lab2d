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

#include "dmlab2d/lib/system/tile/tile_renderer.h"

#include <algorithm>
#include <cstdint>
#include <iterator>

#include "absl/types/span.h"
#include "dmlab2d/lib/support/logging.h"
#include "dmlab2d/lib/system/tile/pixel.h"

namespace deepmind::lab2d {
namespace {

void BlendSemiTransparentConstRgbAlpha(Pixel rgb, PixelByte alpha,
                                       absl::Span<Pixel> in_out) {
  for (std::size_t i = 0; i < in_out.size(); ++i) {
    in_out[i] = Interp(in_out[i], rgb, alpha);
  }
}

void BlendSemiTransparentConstRgb(Pixel rgb, absl::Span<const PixelByte> alpha,
                                  absl::Span<Pixel> in_out) {
  for (std::size_t i = 0; i < in_out.size(); ++i) {
    in_out[i] = Interp(in_out[i], rgb, alpha[i]);
  }
}

void BlendSemiTransparentConstAlpha(absl::Span<const Pixel> rgb,
                                    PixelByte alpha, absl::Span<Pixel> in_out) {
  for (std::size_t i = 0; i < in_out.size(); ++i) {
    in_out[i] = Interp(in_out[i], rgb[i], alpha);
  }
}

void BlendSemiTransparent(absl::Span<const Pixel> rgb,
                          absl::Span<const PixelByte> alpha,
                          absl::Span<Pixel> in_out) {
  for (std::size_t i = 0; i < in_out.size(); ++i) {
    in_out[i] = Interp(in_out[i], rgb[i], alpha[i]);
  }
}

void BlendOneBitAlphaConstRgb(Pixel rgb, absl::Span<const PixelByte> alpha,
                              absl::Span<Pixel> in_out) {
  for (std::size_t i = 0; i < in_out.size(); ++i) {
    in_out[i] = InterpOneBit(in_out[i], rgb, alpha[i]);
  }
}

void BlendOneBitAlpha(absl::Span<const Pixel> rgb,
                      absl::Span<const PixelByte> alpha,
                      absl::Span<Pixel> in_out) {
  for (std::size_t i = 0; i < in_out.size(); ++i) {
    in_out[i] = InterpOneBit(in_out[i], rgb[i], alpha[i]);
  }
}

void BlendBlackSemiTransparentConstRgbAlpha(Pixel rgb, PixelByte alpha,
                                            absl::Span<Pixel> out) {
  for (std::size_t i = 0; i < out.size(); ++i) {
    out[i] = Interp(Pixel::Black(), rgb, alpha);
  }
}

void BlendBlackSemiTransparentConstRgb(Pixel rgb,
                                       absl::Span<const PixelByte> alpha,
                                       absl::Span<Pixel> out) {
  for (std::size_t i = 0; i < out.size(); ++i) {
    out[i] = Interp(Pixel::Black(), rgb, alpha[i]);
  }
}

void BlendBlackSemiTransparentConstAlpha(absl::Span<const Pixel> rgb,
                                         PixelByte alpha,
                                         absl::Span<Pixel> out) {
  for (std::size_t i = 0; i < out.size(); ++i) {
    out[i] = Interp(Pixel::Black(), rgb[i], alpha);
  }
}

void BlendBlackSemiTransparent(absl::Span<const Pixel> rgb,
                               absl::Span<const PixelByte> alpha,
                               absl::Span<Pixel> out) {
  for (std::size_t i = 0; i < out.size(); ++i) {
    out[i] = Interp(Pixel::Black(), rgb[i], alpha[i]);
  }
}

void BlendBlackOneBitAlphaConstRgb(Pixel rgb, absl::Span<const PixelByte> alpha,
                                   absl::Span<Pixel> out) {
  for (std::size_t i = 0; i < out.size(); ++i) {
    out[i] = InterpOneBit(Pixel::Black(), rgb, alpha[i]);
  }
}

void BlendBlackOneBitAlpha(absl::Span<const Pixel> rgb,
                           absl::Span<const PixelByte> alpha,
                           absl::Span<Pixel> out) {
  for (std::size_t i = 0; i < out.size(); ++i) {
    out[i] = InterpOneBit(Pixel::Black(), rgb[i], alpha[i]);
  }
}

void BlendBlackOpaque(absl::Span<const Pixel> rgb, absl::Span<Pixel> out) {
  std::copy(rgb.begin(), rgb.end(), out.begin());
}

void BlendBlackOpaqueConstRgb(Pixel rgb, absl::Span<Pixel> out) {
  std::fill(out.begin(), out.end(), rgb);
}

void CopySpriteToScene(absl::Span<const Pixel> sprite,
                       std::size_t sprite_height, std::size_t sprite_width,
                       absl::Span<Pixel> scene_grid_top_left,
                       std::size_t scene_width) {
  for (std::size_t i = 0; i < sprite_height; ++i) {
    std::copy(sprite.begin() + i * sprite_width,
              sprite.begin() + i * sprite_width + sprite_width,
              scene_grid_top_left.begin() + i * scene_width);
  }
}

}  // namespace

absl::Span<const Pixel> TileRenderer::MakeSprite(
    absl::Span<const int> sprite_indices) {
  if (sprite_indices.empty()) {
    return absl::MakeConstSpan(empty_);
  }
  sprite_indices_.clear();
  sprite_indices_.reserve(sprite_indices.size());
  std::copy_if(sprite_indices.begin(), sprite_indices.end(),
               std::back_inserter(sprite_indices_), [this](int sprite_id) {
                 return sprite_id >= 0 && sprite_id < tile_set_.num_sprites() &&
                        tile_set_.GetSpriteMetaData(sprite_id) !=
                            TileSet::SpriteMetaData::kInvisible;
               });

  // Find kSet or kSetConstRgb from back; if it exists erase everything before
  // it.
  auto rfind_it = std::find_if(
      sprite_indices_.rbegin(), sprite_indices_.rend(), [this](int sprite_id) {
        auto sprite_meta_data = tile_set_.GetSpriteMetaData(sprite_id);
        return sprite_meta_data == TileSet::SpriteMetaData::kOpaque ||
               sprite_meta_data == TileSet::SpriteMetaData::kOpaqueConstRgb;
      });
  if (rfind_it != sprite_indices_.rend()) {
    sprite_indices_.erase(sprite_indices_.begin(), std::next(rfind_it).base());
  }

  if (sprite_indices_.empty()) {
    return absl::MakeConstSpan(empty_);
  }

  auto sprite_id_iter = sprite_indices_.begin();
  {
    auto rgb = tile_set_.GetSpriteRgbData(*sprite_id_iter);
    auto alpha = tile_set_.GetSpriteAlphaData(*sprite_id_iter);
    auto out = absl::MakeSpan(pixels_);
    switch (tile_set_.GetSpriteMetaData(*sprite_id_iter)) {
      case TileSet::SpriteMetaData::kInvisible:
        LOG(FATAL) << "Logic error - invisible sprites should be stripped.";
      case TileSet::SpriteMetaData::kOpaque:
        if (sprite_indices_.size() == 1) {
          return rgb;
        }
        BlendBlackOpaque(rgb, out);
        break;
      case TileSet::SpriteMetaData::kOpaqueConstRgb:
        if (sprite_indices_.size() == 1) {
          return rgb;
        }
        BlendBlackOpaqueConstRgb(rgb[0], out);
        break;
      case TileSet::SpriteMetaData::kSemiTransparent:
        BlendBlackSemiTransparent(rgb, alpha, out);
        break;
      case TileSet::SpriteMetaData::kSemiTransparentConstRgbAlpha:
        BlendBlackSemiTransparentConstRgbAlpha(rgb[0], alpha[0], out);
        break;
      case TileSet::SpriteMetaData::kSemiTransparentConstRgb:
        BlendBlackSemiTransparentConstRgb(rgb[0], alpha, out);
        break;
      case TileSet::SpriteMetaData::kSemiTransparentConstAlpha:
        BlendBlackSemiTransparentConstAlpha(rgb, alpha[0], out);
        break;
      case TileSet::SpriteMetaData::kOneBitAlphaConstRgb:
        BlendBlackOneBitAlphaConstRgb(rgb[0], alpha, out);
        break;
      case TileSet::SpriteMetaData::kOneBitAlpha:
        BlendBlackOneBitAlpha(rgb, alpha, out);
        break;
    }
    ++sprite_id_iter;
  }
  for (; sprite_id_iter != sprite_indices_.end(); ++sprite_id_iter) {
    auto rgb = tile_set_.GetSpriteRgbData(*sprite_id_iter);
    auto alpha = tile_set_.GetSpriteAlphaData(*sprite_id_iter);
    auto in_out = absl::MakeSpan(pixels_);
    switch (tile_set_.GetSpriteMetaData(*sprite_id_iter)) {
      case TileSet::SpriteMetaData::kInvisible:
      case TileSet::SpriteMetaData::kOpaque:
      case TileSet::SpriteMetaData::kOpaqueConstRgb:
        LOG(FATAL) << "Logic error - invisible sprites should be stripped and "
                      "opaque sprite can only be first.";
      case TileSet::SpriteMetaData::kSemiTransparent:
        BlendSemiTransparent(rgb, alpha, in_out);
        break;
      case TileSet::SpriteMetaData::kSemiTransparentConstRgbAlpha:
        BlendSemiTransparentConstRgbAlpha(rgb[0], alpha[0], in_out);
        break;
      case TileSet::SpriteMetaData::kSemiTransparentConstRgb:
        BlendSemiTransparentConstRgb(rgb[0], alpha, in_out);
        break;
      case TileSet::SpriteMetaData::kSemiTransparentConstAlpha:
        BlendSemiTransparentConstAlpha(rgb, alpha[0], in_out);
        break;
      case TileSet::SpriteMetaData::kOneBitAlphaConstRgb:
        BlendOneBitAlphaConstRgb(rgb[0], alpha, in_out);
        break;
      case TileSet::SpriteMetaData::kOneBitAlpha:
        BlendOneBitAlpha(rgb, alpha, in_out);
        break;
    }
  }
  return pixels_;
}

void TileRenderer::Render(absl::Span<const std::int32_t> grid,
                          absl::Span<const std::size_t> grid_shape,
                          absl::Span<Pixel> scene) {
  CHECK(grid_shape.size() == 3) << "Invalid grid shape.";
  const std::size_t height = grid_shape[0];
  const std::size_t width = grid_shape[1];
  const std::size_t layers = grid_shape[2];
  const std::size_t sprite_height = tile_set_.sprite_shape().height;
  const std::size_t sprite_width = tile_set_.sprite_shape().width;
  CHECK(scene.size() == height * sprite_height * width * sprite_width)
      << "Internal Error - scene shape does not match grid shape.";

  const int grid_width = width * layers;
  const int scene_width = width * sprite_width;

  for (std::size_t grid_i = 0; grid_i < height; ++grid_i) {
    auto grid_row = grid.subspan(grid_i * grid_width, grid_width);
    for (std::size_t grid_j = 0; grid_j < width; ++grid_j) {
      auto grid_ids = grid_row.subspan(grid_j * layers, layers);
      auto sprite = MakeSprite(grid_ids);
      auto scene_cell = scene.subspan(
          (grid_i * sprite_height) * scene_width + grid_j * sprite_width,
          scene_width * sprite_height);
      CopySpriteToScene(sprite, sprite_height, sprite_width, scene_cell,
                        scene_width);
    }
  }
}

}  // namespace deepmind::lab2d
