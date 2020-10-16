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

#include "dmlab2d/system/tile/tile_set.h"

#include <cstdint>
#include <numeric>
#include <vector>

#include "absl/types/span.h"
#include "dmlab2d/system/math/math2d.h"
#include "dmlab2d/system/tile/pixel.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {

using ::testing::ElementsAre;
using ::testing::Eq;

constexpr Pixel MakePixel(const unsigned char rgb[]) {
  return Pixel{
      PixelByte(rgb[0]),
      PixelByte(rgb[1]),
      PixelByte(rgb[2]),
  };
}

class SpriteImage {
 public:
  // Constructs white sprite.
  SpriteImage(math::Size2d sprite_shape, bool with_alpha)
      : tensor_shape_{{static_cast<std::size_t>(sprite_shape.height),
                       static_cast<std::size_t>(sprite_shape.width),
                       with_alpha ? 4ul : 3ul}},
        tensor_data_(tensor::Layout::num_elements(tensor_shape_), 255),
        tensor_view_(tensor::Layout(tensor_shape_), tensor_data_.data()) {}

  const tensor::TensorView<unsigned char>& tensor_view() {
    return tensor_view_;
  }

  void Set(int row, int col, const unsigned char rgba[]) {
    auto loc = tensor_view_;
    CHECK(loc.Select(0, row));
    CHECK(loc.Select(0, col));
    loc.Set(0, rgba[0]);
    loc.Set(1, rgba[1]);
    loc.Set(2, rgba[2]);
    loc.Set(3, rgba[3]);
  }

  SpriteImage(SpriteImage&) = delete;

 private:
  tensor::ShapeVector tensor_shape_;
  std::vector<unsigned char> tensor_data_;
  tensor::TensorView<unsigned char> tensor_view_;
};

TEST(TileSetTest, SetSpriteWithAlpha) {
  math::Size2d sprite_shape;
  sprite_shape.height = 2;
  sprite_shape.width = 3;
  TileSet tile_set(1, sprite_shape);
  SpriteImage image(sprite_shape, /*with_alpha=*/true);
  EXPECT_TRUE(tile_set.SetSprite(0, image.tensor_view()));
}

TEST(TileSetTest, SetSpriteNoAlpha) {
  math::Size2d sprite_shape;
  sprite_shape.height = 4;
  sprite_shape.width = 7;
  TileSet tile_set(1, sprite_shape);
  SpriteImage image(sprite_shape, /*with_alpha=*/false);
  EXPECT_TRUE(tile_set.SetSprite(0, image.tensor_view()));
}

struct SpriteParam {
  unsigned char rgba0[4];
  unsigned char rgba1[4];
  TileSet::SpriteMetaData expected_meta_data;
};

class TileSetTestMeta : public ::testing::TestWithParam<SpriteParam> {};

TEST_P(TileSetTestMeta, SpriteMetaData) {
  math::Size2d sprite_shape;
  sprite_shape.height = 1;
  sprite_shape.width = 2;
  TileSet tile_set(1, sprite_shape);
  SpriteImage image(sprite_shape, /*with_alpha=*/true);
  const auto& param = GetParam();
  image.Set(0, 0, param.rgba0);
  image.Set(0, 1, param.rgba1);
  ASSERT_TRUE(tile_set.SetSprite(0, image.tensor_view()));

  EXPECT_THAT(tile_set.GetSpriteRgbData(0),
              ElementsAre(MakePixel(param.rgba0), MakePixel(param.rgba1)));
  EXPECT_THAT(
      tile_set.GetSpriteAlphaData(0),
      ElementsAre(PixelByte(param.rgba0[3]), PixelByte(param.rgba1[3])));
  EXPECT_THAT(tile_set.GetSpriteMetaData(0), Eq(param.expected_meta_data));
}

INSTANTIATE_TEST_SUITE_P(
    SpriteMetaData, TileSetTestMeta,
    ::testing::Values(
        SpriteParam{{0x01, 0x02, 0x03, 0x00},
                    {0x01, 0x02, 0x03, 0x00},
                    TileSet::SpriteMetaData::kInvisible},
        SpriteParam{{0x01, 0x02, 0x03, 0xff},
                    {0x01, 0x02, 0x03, 0xff},
                    TileSet::SpriteMetaData::kOpaqueConstRgb},
        SpriteParam{{0x01, 0x02, 0x03, 0xff},
                    {0x02, 0x04, 0x06, 0xff},
                    TileSet::SpriteMetaData::kOpaque},
        SpriteParam{{0x01, 0x02, 0x03, 0x7f},
                    {0x01, 0x02, 0x03, 0x7f},
                    TileSet::SpriteMetaData::kSemiTransparentConstRgbAlpha},
        SpriteParam{{0x01, 0x02, 0x03, 0x7f},
                    {0x01, 0x02, 0x03, 0xff},
                    TileSet::SpriteMetaData::kSemiTransparentConstRgb},
        SpriteParam{{0x01, 0x02, 0x03, 0x7f},
                    {0x02, 0x04, 0x06, 0x7f},
                    TileSet::SpriteMetaData::kSemiTransparentConstAlpha},
        SpriteParam{{0x01, 0x02, 0x03, 0x7f},
                    {0x02, 0x04, 0x06, 0xff},
                    TileSet::SpriteMetaData::kSemiTransparent},
        SpriteParam{{0x01, 0x02, 0x03, 0x00},
                    {0x01, 0x02, 0x03, 0xff},
                    TileSet::SpriteMetaData::kOneBitAlphaConstRgb},
        SpriteParam{{0x01, 0x02, 0x03, 0x00},
                    {0x02, 0x04, 0x06, 0xff},
                    TileSet::SpriteMetaData::kOneBitAlpha}));

}  // namespace
}  // namespace deepmind::lab2d
