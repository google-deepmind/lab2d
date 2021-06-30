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

#include "dmlab2d/lib/system/grid_world/grid_shape.h"

#include "dmlab2d/lib/system/math/math2d.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {

using ::testing::Eq;
using ::testing::IsFalse;
using ::testing::IsTrue;

TEST(GridShapeTest, InBoundsWorks) {
  const GridShape grid_shape(/*grid_size_2d=*/math::Size2d{5, 3},
                             /*layer_count=*/2, GridShape::Topology::kBounded);
  EXPECT_THAT(grid_shape.InBounds(math::Position2d{0, 0}), IsTrue());
  EXPECT_THAT(grid_shape.InBounds(math::Position2d{4, 0}), IsTrue());
  EXPECT_THAT(grid_shape.InBounds(math::Position2d{4, 2}), IsTrue());
  EXPECT_THAT(grid_shape.InBounds(math::Position2d{0, 2}), IsTrue());
  EXPECT_THAT(grid_shape.InBounds(math::Position2d{-1, 0}), IsFalse());
  EXPECT_THAT(grid_shape.InBounds(math::Position2d{5, 0}), IsFalse());
  EXPECT_THAT(grid_shape.InBounds(math::Position2d{4, 3}), IsFalse());
  EXPECT_THAT(grid_shape.InBounds(math::Position2d{0, 3}), IsFalse());
}

TEST(GridShapeTest, InBoundsTorusWorks) {
  const GridShape grid_shape(/*grid_size_2d=*/math::Size2d{5, 3},
                             /*layer_count=*/2, GridShape::Topology::kTorus);
  EXPECT_THAT(grid_shape.InBounds(math::Position2d{0, 0}), IsTrue());
  EXPECT_THAT(grid_shape.InBounds(math::Position2d{4, 0}), IsTrue());
  EXPECT_THAT(grid_shape.InBounds(math::Position2d{4, 2}), IsTrue());
  EXPECT_THAT(grid_shape.InBounds(math::Position2d{0, 2}), IsTrue());
  EXPECT_THAT(grid_shape.InBounds(math::Position2d{-1, 0}), IsTrue());
  EXPECT_THAT(grid_shape.InBounds(math::Position2d{5, 0}), IsTrue());
  EXPECT_THAT(grid_shape.InBounds(math::Position2d{4, 3}), IsTrue());
  EXPECT_THAT(grid_shape.InBounds(math::Position2d{0, 3}), IsTrue());
}

TEST(GridShapeTest, ToCellIndexWorksAndLayerMinor) {
  const GridShape grid_shape(
      /*grid_size_2d=*/math::Size2d{/*width=*/5, /*height=*/3},
      /*layer_count=*/2, GridShape::Topology::kBounded);
  //  First Cell.
  EXPECT_THAT(grid_shape.ToCellIndex(math::Position2d{0, 0}, Layer(0)),
              Eq(CellIndex(0)));

  EXPECT_THAT(grid_shape.ToCellIndex(math::Position2d{0, 0}, Layer(1)),
              Eq(CellIndex(1)));
  EXPECT_THAT(grid_shape.ToCellIndex(math::Position2d{1, 0}, Layer(0)),
              Eq(CellIndex(2)));
  EXPECT_THAT(grid_shape.ToCellIndex(math::Position2d{1, 0}, Layer(1)),
              Eq(CellIndex(3)));
  EXPECT_THAT(grid_shape.ToCellIndex(math::Position2d{0, 1}, Layer(0)),
              Eq(CellIndex(10)));
  EXPECT_THAT(grid_shape.ToCellIndex(math::Position2d{0, 1}, Layer(1)),
              Eq(CellIndex(11)));

  // Last Cell
  EXPECT_THAT(grid_shape.ToCellIndex(math::Position2d{4, 2}, Layer(1)),
              Eq(CellIndex(5 * 3 * 2 - 1)));
}

TEST(GridShapeTest, ToCellIndexWorksAndLayerMinorTorus) {
  constexpr int width = 5;
  constexpr int height = 3;
  const GridShape grid_shape(
      /*grid_size_2d=*/math::Size2d{/*width=*/width, /*height=*/height},
      /*layer_count=*/2, GridShape::Topology::kTorus);
  //  First Cell.
  EXPECT_THAT(grid_shape.ToCellIndex(math::Position2d{5, height}, Layer(0)),
              Eq(CellIndex(0)));
  EXPECT_THAT(
      grid_shape.ToCellIndex(math::Position2d{0 - 5, 0 - height}, Layer(1)),
      Eq(CellIndex(1)));
  EXPECT_THAT(
      grid_shape.ToCellIndex(math::Position2d{1 - 5, 0 + height}, Layer(0)),
      Eq(CellIndex(2)));
  EXPECT_THAT(grid_shape.ToCellIndex(
                  math::Position2d{1 + 4 * 5, 0 + 3 * height}, Layer(1)),
              Eq(CellIndex(height)));
  EXPECT_THAT(grid_shape.ToCellIndex(math::Position2d{-6 * 5, 1 + 3 * height},
                                     Layer(0)),
              Eq(CellIndex(10)));
  EXPECT_THAT(grid_shape.ToCellIndex(
                  math::Position2d{0 + 18 * 5, 1 - 3 * height}, Layer(1)),
              Eq(CellIndex(11)));

  // Last Cell
  EXPECT_THAT(
      grid_shape.ToCellIndex(math::Position2d{4 + 3 * 5, 2 + 2 * 3}, Layer(1)),
      Eq(CellIndex(5 * 3 * 2 - 1)));
}

TEST(GridShapeTest, TryToCellIndexWorks) {
  const GridShape grid_shape(/*grid_size_2d=*/math::Size2d{5, 3},
                             /*layer_count=*/2, GridShape::Topology::kBounded);
  EXPECT_THAT(grid_shape.TryToCellIndex(math::Position2d{0, 0}, Layer(0)),
              Eq(CellIndex(0)));
  EXPECT_THAT(grid_shape.TryToCellIndex(math::Position2d{5, 0}, Layer(0)),
              Eq(CellIndex()));
  EXPECT_THAT(grid_shape.TryToCellIndex(math::Position2d{0, 0}, Layer()),
              Eq(CellIndex()));
}

TEST(GridShapeTest, TryToCellIndexTorusWorks) {
  const GridShape grid_shape(/*grid_size_2d=*/math::Size2d{5, 3},
                             /*layer_count=*/2, GridShape::Topology::kTorus);
  EXPECT_THAT(grid_shape.TryToCellIndex(math::Position2d{0, 0}, Layer(0)),
              Eq(CellIndex(0)));
  EXPECT_THAT(grid_shape.TryToCellIndex(math::Position2d{-5, 0}, Layer(0)),
              Eq(CellIndex(0)));
  EXPECT_THAT(grid_shape.TryToCellIndex(math::Position2d{0, 0}, Layer()),
              Eq(CellIndex()));
}

TEST(GridShapeTest, GetCellCountWorks) {
  const GridShape grid_shape(/*grid_size_2d=*/math::Size2d{5, 3},
                             /*layer_count=*/2, GridShape::Topology::kBounded);
  EXPECT_THAT(grid_shape.GetCellCount(), Eq(5 * 3 * 2));
}

TEST(GridShapeTest, GridSize2dWorks) {
  const GridShape grid_shape(/*grid_size_2d=*/math::Size2d{5, 3},
                             /*layer_count=*/2, GridShape::Topology::kBounded);
  EXPECT_THAT(grid_shape.GridSize2d(), Eq(math::Size2d{5, 3}));
}

TEST(GridShapeTest, LayerCountWorks) {
  const GridShape grid_shape(/*grid_size_2d=*/math::Size2d{5, 3},
                             /*layer_count=*/2, GridShape::Topology::kBounded);
  EXPECT_THAT(grid_shape.layer_count(), Eq(2));
}

TEST(GridShapeTest, IsTorusWorks) {
  const GridShape grid_shape1(/*grid_size_2d=*/math::Size2d{5, 3},
                              /*layer_count=*/2, GridShape::Topology::kBounded);
  EXPECT_THAT(grid_shape1.topology(), Eq(GridShape::Topology::kBounded));
  const GridShape grid_shape2(/*grid_size_2d=*/math::Size2d{5, 3},
                              /*layer_count=*/2, GridShape::Topology::kTorus);
  EXPECT_THAT(grid_shape2.topology(), Eq(GridShape::Topology::kTorus));
}

TEST(GridShapeTest, SmallestVectorWidthWorks) {
  const GridShape shape(/*grid_size_2d=*/math::Size2d{5, 3},
                        /*layer_count=*/2, GridShape::Topology::kTorus);
  EXPECT_THAT(shape.SmallestVector({0, 0}, {1, 0}), Eq(math::Vector2d{1, 0}));
  EXPECT_THAT(shape.SmallestVector({0, 0}, {2, 0}), Eq(math::Vector2d{2, 0}));
  EXPECT_THAT(shape.SmallestVector({0, 0}, {3, 0}), Eq(math::Vector2d{-2, 0}));
  EXPECT_THAT(shape.SmallestVector({0, 0}, {4, 0}), Eq(math::Vector2d{-1, 0}));
  EXPECT_THAT(shape.SmallestVector({0, 0}, {5, 0}), Eq(math::Vector2d{0, 0}));

  EXPECT_THAT(shape.SmallestVector({2, 0}, {1, 0}), Eq(math::Vector2d{-1, 0}));
  EXPECT_THAT(shape.SmallestVector({2, 0}, {2, 0}), Eq(math::Vector2d{0, 0}));
  EXPECT_THAT(shape.SmallestVector({2, 0}, {3, 0}), Eq(math::Vector2d{1, 0}));
  EXPECT_THAT(shape.SmallestVector({2, 0}, {4, 0}), Eq(math::Vector2d{2, 0}));
  EXPECT_THAT(shape.SmallestVector({2, 0}, {5, 0}), Eq(math::Vector2d{-2, 0}));
}

TEST(GridShapeTest, SmallestVectorHeightWorks) {
  const GridShape shape(/*grid_size_2d=*/math::Size2d{5, 8},
                        /*layer_count=*/2, GridShape::Topology::kTorus);

  EXPECT_THAT(shape.SmallestVector({0, 0}, {0, 1}), Eq(math::Vector2d{0, 1}));
  EXPECT_THAT(shape.SmallestVector({0, 0}, {0, 3}), Eq(math::Vector2d{0, 3}));
  EXPECT_THAT(shape.SmallestVector({0, 0}, {0, 4}), Eq(math::Vector2d{0, -4}));
  EXPECT_THAT(shape.SmallestVector({0, 0}, {0, 5}), Eq(math::Vector2d{0, -3}));
  EXPECT_THAT(shape.SmallestVector({0, 0}, {0, 7}), Eq(math::Vector2d{0, -1}));

  EXPECT_THAT(shape.SmallestVector({0, 2}, {0, 1}), Eq(math::Vector2d{0, -1}));
  EXPECT_THAT(shape.SmallestVector({0, 2}, {0, 3}), Eq(math::Vector2d{0, 1}));
  EXPECT_THAT(shape.SmallestVector({0, 2}, {0, 4}), Eq(math::Vector2d{0, 2}));
  EXPECT_THAT(shape.SmallestVector({0, 2}, {0, 5}), Eq(math::Vector2d{0, 3}));
  EXPECT_THAT(shape.SmallestVector({0, 2}, {0, 7}), Eq(math::Vector2d{0, -3}));

  EXPECT_THAT(shape.SmallestVector({0, 6}, {0, 1}), Eq(math::Vector2d{0, 3}));
  EXPECT_THAT(shape.SmallestVector({0, 6}, {0, 3}), Eq(math::Vector2d{0, -3}));
  EXPECT_THAT(shape.SmallestVector({0, 6}, {0, 4}), Eq(math::Vector2d{0, -2}));
  EXPECT_THAT(shape.SmallestVector({0, 6}, {0, 5}), Eq(math::Vector2d{0, -1}));
  EXPECT_THAT(shape.SmallestVector({0, 6}, {0, 7}), Eq(math::Vector2d{0, 1}));
}

}  // namespace
}  // namespace deepmind::lab2d
