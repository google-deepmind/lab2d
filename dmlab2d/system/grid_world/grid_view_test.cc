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

#include "dmlab2d/system/grid_world/grid_view.h"

#include "absl/types/span.h"
#include "dmlab2d/system/grid_world/collections/fixed_handle_map.h"
#include "dmlab2d/system/grid_world/grid_window.h"
#include "dmlab2d/system/grid_world/handles.h"
#include "dmlab2d/system/grid_world/sprite_instance.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {

using ::testing::Each;
using ::testing::Eq;

// Border is in bounds.
// +----+
// |    |
// |    |
// |    |
// |    |
// |    |
// |  p |
// +----+
constexpr GridWindow kOffsetWindow(/*centered=*/false, /*left=*/3, /*right=*/2,
                                   /*forward=*/6, /*backward=*/1);
// Border is in bounds.
// OOO+----+OOOO
// OOO|    |OOOO
// OOO|    |OOOO
// OOO|    |OOOO
// OOO|    |OOOO
// OOO|    |OOOO
// OOO|  p |OOOO
// OOO+----+OOOO
// OOOOOOOOOOOOO
// OOOOOOOOOOOOO
// OOOOOOOOOOOOO
// OOOOOOOOOOOOO
// OOOOOOOOOOOOO
// OOOOOOOOOOOOO
constexpr GridWindow kCenteredWindow(/*centered=*/true, /*left=*/3, /*right=*/2,
                                     /*forward=*/6, /*backward=*/1);

constexpr Sprite kTestSprite0(0);
constexpr Sprite kTestSprite1(1);
constexpr Sprite kTestSprite2(2);
constexpr Sprite kTestSprite3(3);
constexpr Sprite kOutOfBoundsSprite(4);
constexpr Sprite kOutOfViewSprite(5);

constexpr SpriteInstance kSprite0North{kTestSprite0,
                                       math::Orientation2d::kNorth};
constexpr SpriteInstance kSprite1East{kTestSprite1, math::Orientation2d::kEast};
constexpr SpriteInstance kSprite2South{kTestSprite2,
                                       math::Orientation2d::kSouth};
constexpr SpriteInstance kSprite3West{kTestSprite3, math::Orientation2d::kWest};

GridView MakeCenteredGridView() {
  FixedHandleMap<Sprite, Sprite> sprite_map(6);
  for (int i = 0; i < 6; ++i) {
    Sprite handle(i);
    sprite_map[handle] = handle;
  }
  return GridView(/*window=*/kCenteredWindow, /*num_render_layers=*/2,
                  /*sprite_map=*/sprite_map,
                  /*out_of_bounds_sprite=*/kOutOfBoundsSprite,
                  /*out_of_view_sprite=*/kOutOfViewSprite);
}

GridView MakeGridView() {
  FixedHandleMap<Sprite, Sprite> sprite_map(6);
  for (int i = 0; i < 6; ++i) {
    Sprite handle(i);
    sprite_map[handle] = handle;
  }
  return GridView(/*window=*/kOffsetWindow, /*num_render_layers=*/2,
                  /*sprite_map=*/sprite_map,
                  /*out_of_bounds_sprite=*/kOutOfBoundsSprite,
                  /*out_of_view_sprite=*/kOutOfViewSprite);
}

TEST(GridViewTest, GetWindowWorks) {
  EXPECT_THAT(MakeGridView().GetWindow().size2d().Area(), Eq(8 * 6));
  EXPECT_THAT(MakeCenteredGridView().GetWindow().size2d().Area(), Eq(13 * 13));
}

TEST(GridViewTest, NumRenderLayersWorks) {
  EXPECT_THAT(MakeGridView().NumRenderLayers(), Eq(2));
  EXPECT_THAT(MakeCenteredGridView().NumRenderLayers(), Eq(2));
}

TEST(GridViewTest, NumCellsWorks) {
  EXPECT_THAT(MakeGridView().NumCells(), Eq(2 * 8 * 6));
  EXPECT_THAT(MakeCenteredGridView().NumCells(), Eq(2 * 13 * 13));
}

TEST(GridViewTest, OutOfBoundsSpriteWorks) {
  const GridView grid_view = MakeGridView();
  EXPECT_THAT(grid_view.OutOfBoundsSprite(), Eq(kOutOfBoundsSprite));
}

TEST(GridViewTest, OutOfViewSpriteWorks) {
  const GridView grid_view = MakeGridView();
  EXPECT_THAT(grid_view.OutOfViewSprite(), Eq(kOutOfViewSprite));
}

int CenterOffset(const GridView& grid_view, int offset_x, int offset_y) {
  int y = grid_view.GetWindow().forward() + offset_y;
  int x = grid_view.GetWindow().right() + offset_x;
  return (y * grid_view.GetWindow().width() + x) * grid_view.NumRenderLayers();
}

TEST(GridViewTest, ClearOutOfViewSpritesNorthWorks) {
  const GridView grid_view = MakeCenteredGridView();
  const int in_bounds = grid_view.ToSpriteId(kSprite0North);
  std::vector<int> cells(grid_view.NumCells(), in_bounds);

  grid_view.ClearOutOfViewSprites(/*orientation=*/math::Orientation2d::kNorth,
                                  /*sprite_ids=*/absl::MakeSpan(cells));
  // Grid shape is 13x13x2
  // OOO+----+OOOO
  // OOO|    |OOOO
  // OOO|    |OOOO
  // OOO|    |OOOO
  // OOO|    |OOOO
  // OOO|    |OOOO
  // OOO|  p |OOOO
  // OOO+----+OOOO
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  const int out_of_bounds = grid_view.ToSpriteId(
      SpriteInstance{kOutOfViewSprite, math::Orientation2d::kNorth});
  EXPECT_THAT(cells[CenterOffset(grid_view, 0, 0)], Eq(in_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 0, -2)], Eq(in_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 2, 0)], Eq(in_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 0, 2)], Eq(out_of_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, -2, 0)], Eq(in_bounds));

  EXPECT_THAT(cells[CenterOffset(grid_view, 0, -4)], Eq(in_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 4, 0)], Eq(out_of_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 0, 4)], Eq(out_of_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, -4, 0)], Eq(out_of_bounds));
}

TEST(GridViewTest, ClearOutOfViewSpritesEastWorks) {
  const GridView grid_view = MakeCenteredGridView();
  const int in_bounds = grid_view.ToSpriteId(kSprite0North);
  std::vector<int> cells(grid_view.NumCells(), in_bounds);
  grid_view.ClearOutOfViewSprites(/*orientation=*/math::Orientation2d::kEast,
                                  /*sprite_ids=*/absl::MakeSpan(cells));
  // Grid shape is 13x13x2
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  // OOOOO+------+
  // OOOOO|      |
  // OOOOO|      |
  // OOOOO|p     |
  // OOOOO|      |
  // OOOOO+------+
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  const int out_of_bounds = grid_view.ToSpriteId(
      SpriteInstance{kOutOfViewSprite, math::Orientation2d::kEast});
  EXPECT_THAT(absl::MakeConstSpan(cells.data(), 3 * 13 * 2),
              Each(out_of_bounds));
  EXPECT_THAT(absl::MakeConstSpan(cells.data() + 10 * 13 * 2,
                                  cells.size() - 10 * 13 * 2),
              Each(out_of_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 0, 0)], Eq(in_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 0, -2)], Eq(in_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 2, 0)], Eq(in_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 0, 2)], Eq(in_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, -2, 0)], Eq(out_of_bounds));

  EXPECT_THAT(cells[CenterOffset(grid_view, 0, -4)], Eq(out_of_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 4, 0)], Eq(in_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 0, 4)], Eq(out_of_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, -4, 0)], Eq(out_of_bounds));
}

TEST(GridViewTest, ClearOutOfViewSpritesSouthWorks) {
  const GridView grid_view = MakeCenteredGridView();
  const int in_bounds = grid_view.ToSpriteId(kSprite0North);
  std::vector<int> cells(grid_view.NumCells(), in_bounds);

  grid_view.ClearOutOfViewSprites(/*orientation=*/math::Orientation2d::kSouth,
                                  /*sprite_ids=*/absl::MakeSpan(cells));
  // Grid shape is 13x13x2
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  // OOOO+----+OOO
  // OOOO| p  |OOO
  // OOOO|    |OOO
  // OOOO|    |OOO
  // OOOO|    |OOO
  // OOOO|    |OOO
  // OOOO|    |OOO
  // OOOO+----+OOO
  const int out_of_bounds = grid_view.ToSpriteId(
      SpriteInstance{kOutOfViewSprite, math::Orientation2d::kSouth});
  EXPECT_THAT(cells[CenterOffset(grid_view, 0, 0)], Eq(in_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 0, -2)], Eq(out_of_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 2, 0)], Eq(in_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 0, 2)], Eq(in_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, -2, 0)], Eq(in_bounds));

  EXPECT_THAT(cells[CenterOffset(grid_view, 0, -4)], Eq(out_of_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 4, 0)], Eq(out_of_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 0, 4)], Eq(in_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, -4, 0)], Eq(out_of_bounds));
}

TEST(GridViewTest, ClearOutOfViewSpritesWestWorks) {
  const GridView grid_view = MakeCenteredGridView();
  const int in_bounds = grid_view.ToSpriteId(kSprite0North);
  std::vector<int> cells(grid_view.NumCells(), in_bounds);
  grid_view.ClearOutOfViewSprites(/*orientation=*/math::Orientation2d::kWest,
                                  /*sprite_ids=*/absl::MakeSpan(cells));
  // Grid shape is 13x13x2
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  // +------+OOOOO
  // |      |OOOOO
  // |     p|OOOOO
  // |      |OOOOO
  // |      |OOOOO
  // +------+OOOOO
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  // OOOOOOOOOOOOO
  const int out_of_bounds = grid_view.ToSpriteId(
      SpriteInstance{kOutOfViewSprite, math::Orientation2d::kWest});
  EXPECT_THAT(cells[CenterOffset(grid_view, 0, 0)], Eq(in_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 0, -2)], Eq(in_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 2, 0)], Eq(out_of_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 0, 2)], Eq(in_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, -2, 0)], Eq(in_bounds));

  EXPECT_THAT(cells[CenterOffset(grid_view, 0, -4)], Eq(out_of_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 4, 0)], Eq(out_of_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, 0, 4)], Eq(out_of_bounds));
  EXPECT_THAT(cells[CenterOffset(grid_view, -4, 0)], Eq(in_bounds));
}

TEST(GridViewTest, ClearOutOfViewSpritesOffsetWorks) {
  const GridView grid_view = MakeGridView();
  const int in_bounds = grid_view.ToSpriteId(kSprite0North);
  std::vector<int> cells(grid_view.NumCells(), in_bounds);
  grid_view.ClearOutOfViewSprites(/*orientation=*/math::Orientation2d::kWest,
                                  /*sprite_ids=*/absl::MakeSpan(cells));
  EXPECT_THAT(cells, Each(in_bounds));
}

TEST(GridViewStaticTest, ToSpriteIdWorks) {
  const GridView grid_view = MakeGridView();
  EXPECT_THAT(grid_view.ToSpriteId(kSprite0North), Eq(0 * 4 + 0 + 1));
  EXPECT_THAT(grid_view.ToSpriteId(kSprite1East), Eq(1 * 4 + 1 + 1));
  EXPECT_THAT(grid_view.ToSpriteId(kSprite2South), Eq(2 * 4 + 2 + 1));
  EXPECT_THAT(grid_view.ToSpriteId(kSprite3West), Eq(3 * 4 + 3 + 1));
}

TEST(GridViewStaticTest, NumSpriteIdsWorks) {
  const GridView grid_view = MakeGridView();
  EXPECT_THAT(grid_view.NumSpriteIds(), Eq(6 * 4 + 1));
}

}  // namespace
}  // namespace deepmind::lab2d
