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

#include "dmlab2d/lib/system/tile/tile_set.h"

#include <algorithm>
#include <array>
#include <cstdlib>
#include <iterator>
#include <vector>

#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "absl/types/span.h"
#include "dmlab2d/lib/system/tensor/tensor_view.h"
#include "dmlab2d/lib/system/tile/pixel.h"

namespace deepmind::lab2d {
namespace {

struct ChannelInfo {
  bool all_same = true;
  bool all_min = true;
  bool all_max = true;
  bool all_min_or_max = true;
};

TileSet::SpriteMetaData SpriteMetaDataFromChannelInfos(
    const std::array<ChannelInfo, 4>& channel_infos) {
  if (channel_infos[3].all_min) {
    return TileSet::SpriteMetaData::kInvisible;
  }

  bool const_rgb = channel_infos[0].all_same && channel_infos[1].all_same &&
                   channel_infos[2].all_same;
  // Opaque
  if (channel_infos[3].all_max) {
    if (const_rgb) {
      return TileSet::SpriteMetaData::kOpaqueConstRgb;
    } else {
      return TileSet::SpriteMetaData::kOpaque;
    }
  }

  // One bit alpha.
  if (channel_infos[3].all_min_or_max) {
    if (const_rgb) {
      return TileSet::SpriteMetaData::kOneBitAlphaConstRgb;
    } else {
      return TileSet::SpriteMetaData::kOneBitAlpha;
    }
  }

  // Transparent.
  bool const_alpha = channel_infos[3].all_same;
  if (const_rgb) {
    if (const_alpha) {
      return TileSet::SpriteMetaData::kSemiTransparentConstRgbAlpha;
    } else {
      return TileSet::SpriteMetaData::kSemiTransparentConstRgb;
    }
  } else {
    if (const_alpha) {
      return TileSet::SpriteMetaData::kSemiTransparentConstAlpha;
    } else {
      return TileSet::SpriteMetaData::kSemiTransparent;
    }
  }
}

TileSet::SpriteMetaData CalculateSpriteMetaData(
    const tensor::TensorView<unsigned char>& image_tensor) {
  const auto& shape = image_tensor.shape();
  std::array<ChannelInfo, 4> channel_infos = {};

  for (int channel = 0; channel < shape[2]; ++channel) {
    tensor::TensorView<unsigned char> channel_data = image_tensor;
    channel_data.Select(2, channel);
    bool first = true;
    PixelByte first_byte = {};
    ChannelInfo& channel_info = channel_infos[channel];
    channel_data.ForEach(
        [&channel_info, &first_byte, &first](unsigned char data) {
          PixelByte pixel_byte = PixelByte(data);
          if (first) {
            first = false;
            first_byte = pixel_byte;
          }

          if (channel_info.all_same && first_byte != pixel_byte) {
            channel_info.all_same = false;
          }
          switch (pixel_byte) {
            case PixelByte::Min:
              channel_info.all_max = false;
              break;
            case PixelByte::Max:
              channel_info.all_min = false;
              break;
            default:
              channel_info.all_min_or_max = false;
              break;
          }
        });
    if (!channel_info.all_min_or_max) {
      channel_info.all_min = false;
      channel_info.all_max = false;
    }
  }
  if (shape[2] < 4) {
    channel_infos[3].all_min = false;
  }
  return SpriteMetaDataFromChannelInfos(channel_infos);
}

}  // namespace

bool TileSet::SetSprite(std::size_t index,
                        const tensor::TensorView<unsigned char>& image_tensor) {
  const auto& shape = image_tensor.shape();
  if (shape.size() != 3 || shape[0] != sprite_shape().height ||
      shape[1] != sprite_shape().width || (shape[2] != 3 && shape[2] != 4)) {
    return false;
  }

  sprite_meta_data_[index] = CalculateSpriteMetaData(image_tensor);
  auto mutable_sprite_rgb_data = MutableSpriteRgbData(index);
  auto mutable_sprite_alpha_data = MutableSpriteAlphaData(index);
  std::size_t offset = 0;
  std::size_t offset_r = 0;
  std::size_t offset_g = 0;
  std::size_t offset_b = 0;
  std::size_t offset_a = 0;
  if (shape[2] == 4) {
    image_tensor.ForEach([mutable_sprite_rgb_data, mutable_sprite_alpha_data,
                          &offset, &offset_r, &offset_g, &offset_b,
                          &offset_a](unsigned char val) {
      PixelByte in = PixelByte(val);
      switch ((offset++) % 4) {
        case 0:
          mutable_sprite_rgb_data[offset_r++].r = in;
          return;
        case 1:
          mutable_sprite_rgb_data[offset_g++].g = in;
          return;
        case 2:
          mutable_sprite_rgb_data[offset_b++].b = in;
          return;
        case 3:
          mutable_sprite_alpha_data[offset_a++] = in;
          return;
      }
    });
  } else {
    image_tensor.ForEach([mutable_sprite_rgb_data, &offset](unsigned char val) {
      PixelByte in = PixelByte(val);
      int index = offset / 3;
      switch ((offset++) % 3) {
        case 0:
          mutable_sprite_rgb_data[index].r = in;
          return;
        case 1:
          mutable_sprite_rgb_data[index].g = in;
          return;
        case 2:
          mutable_sprite_rgb_data[index].b = in;
          return;
      }
    });
    std::fill(mutable_sprite_alpha_data.begin(),
              mutable_sprite_alpha_data.end(), PixelByte::Max);
  }
  return true;
}

}  // namespace deepmind::lab2d
