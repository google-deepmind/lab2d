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

#include "dmlab2d/system/tile/tile_renderer.h"

#include <array>
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

constexpr Pixel MakePixel(unsigned char r, unsigned char g, unsigned char b) {
  return Pixel{
      PixelByte(r),
      PixelByte(g),
      PixelByte(b),
  };
}

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

  void Set(int row, int col, Pixel pixel) {
    auto loc = tensor_view_;
    CHECK(loc.Select(0, row));
    CHECK(loc.Select(0, col));
    loc.Set(0, AsUChar(pixel.r));
    loc.Set(1, AsUChar(pixel.g));
    loc.Set(2, AsUChar(pixel.b));
  }

  void Set(int row, int col, Pixel pixel, PixelByte alpha) {
    auto loc = tensor_view_;
    CHECK(loc.Select(0, row));
    CHECK(loc.Select(0, col));
    loc.Set(0, AsUChar(pixel.r));
    loc.Set(1, AsUChar(pixel.g));
    loc.Set(2, AsUChar(pixel.b));
    loc.Set(3, AsUChar(alpha));
  }

  SpriteImage(SpriteImage&) = delete;

 private:
  tensor::ShapeVector tensor_shape_;
  std::vector<unsigned char> tensor_data_;
  tensor::TensorView<unsigned char> tensor_view_;
};

class Grid {
 public:
  Grid(math::Size2d shape, std::size_t layers)
      : shape_{static_cast<std::size_t>(shape.height),
               static_cast<std::size_t>(shape.width), layers},
        grid_(std::accumulate(shape_.begin(), shape_.end(), 1,
                              std::multiplies<std::size_t>())) {}

  absl::Span<const std::int32_t> grid() const {
    return absl::MakeConstSpan(grid_);
  }
  absl::Span<const std::size_t> shape() const {
    return absl::MakeConstSpan(shape_);
  }

  void Set(int row, int col, int layer, std::int32_t sprite_id) {
    grid_[row * shape_[2] * shape_[1] + col * shape_[2] + layer] = sprite_id;
  }

 private:
  std::array<std::size_t, 3> shape_;
  std::vector<std::int32_t> grid_;
};

class Scene {
 public:
  Scene(const Grid& grid, const TileSet& tile_set)
      : pixels_(grid.shape()[0] * tile_set.sprite_shape().height *
                grid.shape()[1] * tile_set.sprite_shape().width) {}

  absl::Span<Pixel> pixels() { return absl::MakeSpan(pixels_); }

 private:
  std::vector<Pixel> pixels_;
};

TEST(TileRendererTest, RenderDirect) {
  math::Size2d sprite_shape;
  sprite_shape.height = 1;
  sprite_shape.width = 2;
  TileSet tile_set(1, sprite_shape);
  SpriteImage image(sprite_shape, /*with_alpha=*/false);
  image.Set(0, 0, MakePixel(0, 127, 255));
  image.Set(0, 1, MakePixel(64, 64, 128));
  ASSERT_TRUE(tile_set.SetSprite(0, image.tensor_view()));
  Grid grid(/*shape=*/math::Size2d{1, 1}, /*layers=*/1);
  Scene scene(grid, tile_set);
  TileRenderer renderer(&tile_set);
  ASSERT_NO_FATAL_FAILURE(
      renderer.Render(grid.grid(), grid.shape(), scene.pixels()));
  EXPECT_THAT(scene.pixels(),
              ElementsAre(MakePixel(0, 127, 255), MakePixel(64, 64, 128)));
}

TEST(TileRendererTest, RenderAlphaBlack) {
  math::Size2d sprite_shape;
  sprite_shape.height = 1;
  sprite_shape.width = 2;
  TileSet tile_set(1, sprite_shape);
  SpriteImage image(sprite_shape, /*with_alpha=*/true);

  image.Set(0, 0, MakePixel(0x0a, 0x0c, 0x0e), PixelByte(0x7f));
  image.Set(0, 1, MakePixel(0x40, 0x40, 0x80), PixelByte(0xff));
  ASSERT_TRUE(tile_set.SetSprite(0, image.tensor_view()));
  Grid grid(/*shape=*/math::Size2d{1, 1}, /*layers=*/1);
  Scene scene(grid, tile_set);
  TileRenderer renderer(&tile_set);
  ASSERT_NO_FATAL_FAILURE(
      renderer.Render(grid.grid(), grid.shape(), scene.pixels()));
  EXPECT_THAT(scene.pixels(), ElementsAre(MakePixel(0x05, 0x06, 0x07),
                                          MakePixel(0x40, 0x40, 0x80)));
}

TEST(TileRendererTest, RenderEmpty) {
  math::Size2d sprite_shape;
  sprite_shape.height = 1;
  sprite_shape.width = 2;
  TileSet tile_set(0, sprite_shape);
  Grid grid(/*shape=*/math::Size2d{1, 1}, /*layers=*/0);
  Scene scene(grid, tile_set);
  TileRenderer renderer(&tile_set);
  ASSERT_NO_FATAL_FAILURE(
      renderer.Render(grid.grid(), grid.shape(), scene.pixels()));
  EXPECT_THAT(scene.pixels(), ElementsAre(MakePixel(0x00, 0x00, 0x00),
                                          MakePixel(0x00, 0x00, 0x00)));
}

TEST(TileRendererTest, RenderBlendOnConstRgb) {
  math::Size2d sprite_shape;
  sprite_shape.height = 1;
  sprite_shape.width = 2;
  TileSet tile_set(2, sprite_shape);
  SpriteImage image(sprite_shape, /*with_alpha=*/true);
  image.Set(0, 0, MakePixel(0x80, 0x80, 0x80), PixelByte(0xff));
  image.Set(0, 1, MakePixel(0x80, 0x80, 0x80), PixelByte(0xff));
  ASSERT_TRUE(tile_set.SetSprite(0, image.tensor_view()));
  image.Set(0, 0, MakePixel(0x00, 0x00, 0x00), PixelByte(0x7f));
  image.Set(0, 1, MakePixel(0x00, 0x00, 0x00), PixelByte(0x7f));
  ASSERT_TRUE(tile_set.SetSprite(1, image.tensor_view()));
  Grid grid(/*shape=*/math::Size2d{1, 1}, /*layers=*/2);
  grid.Set(/*row=*/0, /*col=*/0, /*layer=*/0, /*sprite_id=*/0);
  grid.Set(/*row=*/0, /*col=*/0, /*layer=*/1, /*sprite_id=*/1);
  Scene scene(grid, tile_set);
  TileRenderer renderer(&tile_set);
  ASSERT_NO_FATAL_FAILURE(
      renderer.Render(grid.grid(), grid.shape(), scene.pixels()));
  EXPECT_THAT(scene.pixels(), ElementsAre(MakePixel(0x40, 0x40, 0x40),
                                          MakePixel(0x40, 0x40, 0x40)));
}

struct BlendTestParam {
  unsigned char rgba0[4];
  unsigned char rgba1[4];
  unsigned char out0[3];
  unsigned char out1[3];
  TileSet::SpriteMetaData expected_meta_data;
};

class TileRendererTestEmpty : public ::testing::TestWithParam<BlendTestParam> {
};

TEST_P(TileRendererTestEmpty, RenderOnEmpty) {
  math::Size2d sprite_shape;
  sprite_shape.height = 1;
  sprite_shape.width = 2;
  TileSet tile_set(1, sprite_shape);
  SpriteImage image(sprite_shape, /*with_alpha=*/true);
  const auto& param = GetParam();
  image.Set(0, 0, MakePixel(param.rgba0), PixelByte(param.rgba0[3]));
  image.Set(0, 1, MakePixel(param.rgba1), PixelByte(param.rgba1[3]));
  ASSERT_TRUE(tile_set.SetSprite(0, image.tensor_view()));
  EXPECT_THAT(tile_set.GetSpriteMetaData(0), param.expected_meta_data);

  Grid grid(/*shape=*/math::Size2d{1, 1}, /*layers=*/1);
  grid.Set(/*row=*/0, /*col=*/0, /*layer=*/0, /*sprite_id=*/0);
  Scene scene(grid, tile_set);
  TileRenderer renderer(&tile_set);
  ASSERT_NO_FATAL_FAILURE(
      renderer.Render(grid.grid(), grid.shape(), scene.pixels()));
  EXPECT_THAT(scene.pixels(),
              ElementsAre(MakePixel(param.out0), MakePixel(param.out1)));
}

INSTANTIATE_TEST_SUITE_P(
    RenderOnEmpty, TileRendererTestEmpty,
    ::testing::Values(
        BlendTestParam{{0x01, 0x02, 0x03, 0x00},
                       {0x02, 0x04, 0x06, 0x00},
                       {0x00, 0x00, 0x00},
                       {0x00, 0x00, 0x00},
                       TileSet::SpriteMetaData::kInvisible},
        BlendTestParam{{0x02, 0x04, 0x06, 0xff},
                       {0x02, 0x04, 0x06, 0xff},
                       {0x02, 0x04, 0x06},
                       {0x02, 0x04, 0x06},
                       TileSet::SpriteMetaData::kOpaqueConstRgb},
        BlendTestParam{{0x01, 0x02, 0x03, 0xff},
                       {0x02, 0x04, 0x06, 0xff},
                       {0x01, 0x02, 0x03},
                       {0x02, 0x04, 0x06},
                       TileSet::SpriteMetaData::kOpaque},
        BlendTestParam{{0x02, 0x04, 0x06, 0x7f},
                       {0x02, 0x04, 0x06, 0x7f},
                       {0x01, 0x02, 0x03},
                       {0x01, 0x02, 0x03},
                       TileSet::SpriteMetaData::kSemiTransparentConstRgbAlpha},
        BlendTestParam{{0x02, 0x04, 0x06, 0xff},
                       {0x02, 0x04, 0x06, 0x7f},
                       {0x02, 0x04, 0x06},
                       {0x01, 0x02, 0x03},
                       TileSet::SpriteMetaData::kSemiTransparentConstRgb},
        BlendTestParam{{0x04, 0x08, 0x0C, 0x7f},
                       {0x02, 0x04, 0x06, 0x7f},
                       {0x02, 0x04, 0x06},
                       {0x01, 0x02, 0x03},
                       TileSet::SpriteMetaData::kSemiTransparentConstAlpha},
        BlendTestParam{{0x02, 0x04, 0x06, 0x7f},
                       {0x04, 0x08, 0x0C, 0xff},
                       {0x01, 0x02, 0x03},
                       {0x04, 0x08, 0x0C},
                       TileSet::SpriteMetaData::kSemiTransparent},
        BlendTestParam{{0x02, 0x04, 0x06, 0xff},
                       {0x02, 0x04, 0x06, 0x00},
                       {0x02, 0x04, 0x06},
                       {0x00, 0x00, 0x00},
                       TileSet::SpriteMetaData::kOneBitAlphaConstRgb},
        BlendTestParam{{0x02, 0x08, 0x0C, 0xff},
                       {0x02, 0x04, 0x06, 0x00},
                       {0x02, 0x08, 0x0C},
                       {0x00, 0x00, 0x00},
                       TileSet::SpriteMetaData::kOneBitAlpha}));

class TileRendererTestBlackWhite
    : public ::testing::TestWithParam<BlendTestParam> {};

TEST_P(TileRendererTestBlackWhite, RenderBlendOnBlackWhite) {
  math::Size2d sprite_shape;
  sprite_shape.height = 1;
  sprite_shape.width = 2;
  TileSet tile_set(2, sprite_shape);
  SpriteImage image(sprite_shape, /*with_alpha=*/true);
  const auto& param = GetParam();
  image.Set(0, 0, MakePixel(0, 0, 0));
  // Image is now two pixels black, white.
  ASSERT_TRUE(tile_set.SetSprite(0, image.tensor_view()));
  image.Set(0, 0, MakePixel(param.rgba0), PixelByte(param.rgba0[3]));
  image.Set(0, 1, MakePixel(param.rgba1), PixelByte(param.rgba1[3]));
  ASSERT_TRUE(tile_set.SetSprite(1, image.tensor_view()));
  EXPECT_THAT(tile_set.GetSpriteMetaData(1), param.expected_meta_data);
  Grid grid(/*shape=*/math::Size2d{1, 1}, /*layers=*/2);
  grid.Set(/*row=*/0, /*col=*/0, /*layer=*/0, /*sprite_id=*/0);
  grid.Set(/*row=*/0, /*col=*/0, /*layer=*/1, /*sprite_id=*/1);
  Scene scene(grid, tile_set);
  TileRenderer renderer(&tile_set);
  ASSERT_NO_FATAL_FAILURE(
      renderer.Render(grid.grid(), grid.shape(), scene.pixels()));
  EXPECT_THAT(scene.pixels(),
              ElementsAre(MakePixel(param.out0), MakePixel(param.out1)));
}

INSTANTIATE_TEST_SUITE_P(
    RenderBlendOnBlackWhite, TileRendererTestBlackWhite,
    ::testing::Values(
        BlendTestParam{{0x01, 0x02, 0x03, 0x00},
                       {0x02, 0x04, 0x06, 0x00},
                       {0x00, 0x00, 0x00},
                       {0xff, 0xff, 0xff},
                       TileSet::SpriteMetaData::kInvisible},
        BlendTestParam{{0x02, 0x04, 0x06, 0xff},
                       {0x02, 0x04, 0x06, 0xff},
                       {0x02, 0x04, 0x06},
                       {0x02, 0x04, 0x06},
                       TileSet::SpriteMetaData::kOpaqueConstRgb},
        BlendTestParam{{0x01, 0x02, 0x03, 0xff},
                       {0x02, 0x04, 0x06, 0xff},
                       {0x01, 0x02, 0x03},
                       {0x02, 0x04, 0x06},
                       TileSet::SpriteMetaData::kOpaque},
        BlendTestParam{{0x02, 0x04, 0x06, 0x7f},
                       {0x02, 0x04, 0x06, 0x7f},
                       {0x01, 0x02, 0x03},
                       {0x81, 0x82, 0x83},
                       TileSet::SpriteMetaData::kSemiTransparentConstRgbAlpha},
        BlendTestParam{{0x02, 0x04, 0x06, 0xff},
                       {0x02, 0x04, 0x06, 0x7f},
                       {0x02, 0x04, 0x06},
                       {0x81, 0x82, 0x83},
                       TileSet::SpriteMetaData::kSemiTransparentConstRgb},
        BlendTestParam{{0x04, 0x08, 0x0C, 0x7f},
                       {0x02, 0x04, 0x06, 0x7f},
                       {0x02, 0x04, 0x06},
                       {0x81, 0x82, 0x83},
                       TileSet::SpriteMetaData::kSemiTransparentConstAlpha},
        BlendTestParam{{0x02, 0x04, 0x06, 0x7f},
                       {0x04, 0x08, 0x0C, 0xff},
                       {0x01, 0x02, 0x03},
                       {0x04, 0x08, 0x0C},
                       TileSet::SpriteMetaData::kSemiTransparent},
        BlendTestParam{{0x02, 0x04, 0x06, 0xff},
                       {0x02, 0x04, 0x06, 0x00},
                       {0x02, 0x04, 0x06},
                       {0xff, 0xff, 0xff},
                       TileSet::SpriteMetaData::kOneBitAlphaConstRgb},
        BlendTestParam{{0x02, 0x08, 0x0C, 0xff},
                       {0x02, 0x04, 0x06, 0x00},
                       {0x02, 0x08, 0x0C},
                       {0xff, 0xff, 0xff},
                       TileSet::SpriteMetaData::kOneBitAlpha}));

}  // namespace
}  // namespace deepmind::lab2d
