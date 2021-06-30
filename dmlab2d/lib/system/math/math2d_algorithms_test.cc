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

#include "dmlab2d/lib/system/math/math2d_algorithms.h"

#include <vector>

#include "dmlab2d/lib/system/math/math2d.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d::math {
namespace {

using testing::ElementsAre;
using testing::ElementsAreArray;
using testing::IsEmpty;
using testing::UnorderedElementsAre;

std::vector<Position2d> MakeLine(Position2d start, Position2d end) {
  std::vector<Position2d> result;
  RayCastLine(start, end, [&result](Position2d pos) {
    result.push_back(pos);
    return false;
  });
  return result;
}

TEST(Math2dAlgorithmTest, RayCastLineHorizontal) {
  EXPECT_THAT(MakeLine({0, 0}, {3, 0}),        //
              ElementsAre(Position2d{1, 0},    //
                          Position2d{2, 0},    //
                          Position2d{3, 0}));  //

  EXPECT_THAT(MakeLine({3, 0}, {0, 0}),        //
              ElementsAre(Position2d{2, 0},    //
                          Position2d{1, 0},    //
                          Position2d{0, 0}));  //
}

TEST(Math2dAlgorithmTest, RayCastLineVertical) {
  EXPECT_THAT(MakeLine({0, 0}, {0, 3}),        //
              ElementsAre(Position2d{0, 1},    //
                          Position2d{0, 2},    //
                          Position2d{0, 3}));  //

  EXPECT_THAT(MakeLine({0, 3}, {0, 0}),        //
              ElementsAre(Position2d{0, 2},    //
                          Position2d{0, 1},    //
                          Position2d{0, 0}));  //
}

TEST(Math2dAlgorithmTest, RayCastLineDiag) {
  EXPECT_THAT(MakeLine({0, 0}, {3, 3}),        //
              ElementsAre(Position2d{1, 0},    //
                          Position2d{1, 1},    //
                          Position2d{2, 1},    //
                          Position2d{2, 2},    //
                          Position2d{3, 2},    //
                          Position2d{3, 3}));  //
}

//   01234567890
// 0 S*
// 1  ****
// 2     ***
// 3       ****
// 4          **
TEST(Math2dAlgorithmTest, RayCastLineLowSlope) {
  EXPECT_THAT(MakeLine({0, 0}, {10, 4}),               //
              ElementsAreArray({Position2d{1, 0},      //
                                Position2d{1, 1},      //
                                Position2d{2, 1},      //
                                Position2d{3, 1},      //
                                Position2d{4, 1},      //
                                Position2d{4, 2},      //
                                Position2d{5, 2},      //
                                Position2d{6, 2},      //
                                Position2d{6, 3},      //
                                Position2d{7, 3},      //
                                Position2d{8, 3},      //
                                Position2d{9, 3},      //
                                Position2d{9, 4},      //
                                Position2d{10, 4}}));  //
}

//   01234567890
// 0 S*
// 1  ****
// 2     ***
// 3       ****
// 4          **
TEST(Math2dAlgorithmTest, RayCastLineSteepSlope) {
  EXPECT_THAT(MakeLine({0, 0}, {4, -10}),               //
              ElementsAreArray({Position2d{0, -1},      //
                                Position2d{1, -1},      //
                                Position2d{1, -2},      //
                                Position2d{1, -3},      //
                                Position2d{1, -4},      //
                                Position2d{2, -4},      //
                                Position2d{2, -5},      //
                                Position2d{2, -6},      //
                                Position2d{3, -6},      //
                                Position2d{3, -7},      //
                                Position2d{3, -8},      //
                                Position2d{3, -9},      //
                                Position2d{4, -9},      //
                                Position2d{4, -10}}));  //
}

TEST(Math2dAlgorithmTest, RayCastHit) {
  auto visitor = [](Position2d p) { return p == Position2d{0, 0}; };
  EXPECT_TRUE(RayCastLine({-1, 0}, {1, 0}, visitor));
  EXPECT_FALSE(RayCastLine({1, 0}, {3, 0}, visitor));
}

TEST(Math2dAlgorithmTest, NoVisit) {
  auto visitor = [](Position2d p) { return true; };
  EXPECT_TRUE(RayCastLine({1, 1}, {1, 2}, visitor));
  EXPECT_FALSE(RayCastLine({1, 1}, {1, 1}, visitor));
}

std::vector<Position2d> MakeRectangleClamped(Position2d corner0,
                                             Position2d corner1,
                                             Size2d window) {
  std::vector<Position2d> result;
  VisitRectangleClamped(corner0, corner1, window,
                        [&result](Position2d pos) { result.push_back(pos); });
  return result;
}

TEST(Math2dAlgorithmTest, VisitRectangleClampedTestSingle) {
  EXPECT_THAT(MakeRectangleClamped({1, 1}, {1, 1}, {2, 2}),
              ElementsAre(Position2d{1, 1}));
}

TEST(Math2dAlgorithmTest, VisitRectangleClampedTestTwoThree) {
  EXPECT_THAT(MakeRectangleClamped({1, 1}, {2, 3}, {4, 4}),
              UnorderedElementsAre(Position2d{1, 1},  //
                                   Position2d{2, 1},  //
                                   Position2d{1, 2},  //
                                   Position2d{2, 2},  //
                                   Position2d{1, 3},  //
                                   Position2d{2, 3}));
}

TEST(Math2dAlgorithmTest, VisitRectangleClampedOutSide) {
  EXPECT_THAT(MakeRectangleClamped({1, 1}, {1, 1}, {1, 1}), IsEmpty());
  EXPECT_THAT(MakeRectangleClamped({-1, -1}, {-1, -1}, {1, 1}), IsEmpty());
  EXPECT_THAT(MakeRectangleClamped({1, -1}, {1, -1}, {1, 1}), IsEmpty());
  EXPECT_THAT(MakeRectangleClamped({0, 0}, {0, 0}, {0, 0}), IsEmpty());
}

TEST(Math2dAlgorithmTest, VisitRectangleClampedCornersEquivalent) {
  EXPECT_THAT(MakeRectangleClamped({2, 4}, {6, 9}, {10, 10}),
              ElementsAreArray(MakeRectangleClamped({6, 4}, {2, 9}, {10, 10})));
  EXPECT_THAT(MakeRectangleClamped({2, 4}, {6, 9}, {10, 10}),
              ElementsAreArray(MakeRectangleClamped({2, 9}, {6, 4}, {10, 10})));
  EXPECT_THAT(MakeRectangleClamped({2, 4}, {6, 9}, {10, 10}),
              ElementsAreArray(MakeRectangleClamped({6, 9}, {2, 4}, {10, 10})));
}

TEST(Math2dAlgorithmTest, VisitRectangleClampedIsClamped) {
  EXPECT_THAT(MakeRectangleClamped({1, 1}, {3, 3}, {2, 3}),
              ElementsAreArray(MakeRectangleClamped({1, 1}, {1, 2}, {10, 10})));
}

std::vector<Position2d> MakeRectangle(Position2d corner0, Position2d corner1) {
  std::vector<Position2d> result;
  VisitRectangle(corner0, corner1,
                 [&result](Position2d pos) { result.push_back(pos); });
  return result;
}

///

TEST(Math2dAlgorithmTest, VisitRectangleTestSingle) {
  EXPECT_THAT(MakeRectangle({1, 1}, {1, 1}), ElementsAre(Position2d{1, 1}));
}

TEST(Math2dAlgorithmTest, VisitRectangleTestTwoThree) {
  EXPECT_THAT(MakeRectangle({1, 1}, {2, 3}),
              UnorderedElementsAre(Position2d{1, 1},  //
                                   Position2d{2, 1},  //
                                   Position2d{1, 2},  //
                                   Position2d{2, 2},  //
                                   Position2d{1, 3},  //
                                   Position2d{2, 3}));
}

TEST(Math2dAlgorithmTest, VisitRectangleOutSide) {
  EXPECT_THAT(MakeRectangle({1, 1}, {1, 1}), ElementsAre(Position2d{1, 1}));
  EXPECT_THAT(MakeRectangle({-1, -1}, {-1, -1}),
              ElementsAre(Position2d{-1, -1}));
  EXPECT_THAT(MakeRectangle({1, -1}, {1, -1}), ElementsAre(Position2d{1, -1}));
  EXPECT_THAT(MakeRectangle({0, 0}, {0, 0}), ElementsAre(Position2d{0, 0}));
}

TEST(Math2dAlgorithmTest, VisitRectangleCornersEquivalent) {
  EXPECT_THAT(MakeRectangle({-2, 4}, {6, -9}),
              ElementsAreArray(MakeRectangle({6, 4}, {-2, -9})));
  EXPECT_THAT(MakeRectangle({-2, 4}, {6, -9}),
              ElementsAreArray(MakeRectangle({-2, -9}, {6, 4})));
  EXPECT_THAT(MakeRectangle({-2, 4}, {6, -9}),
              ElementsAreArray(MakeRectangle({6, -9}, {-2, 4})));
}

std::vector<Position2d> MakeDisc(Position2d center, int radius) {
  std::vector<Position2d> result;
  VisitDisc(center, radius,
            [&result](Position2d pos) { result.push_back(pos); });
  return result;
}

TEST(Math2dAlgorithmTest, VisitDiscRadiusIsZero) {
  EXPECT_THAT(MakeDisc({0, 0}, 0), UnorderedElementsAre(Position2d{0, 0}));
}

TEST(Math2dAlgorithmTest, VisitDiscRadiusIsOne) {
  EXPECT_THAT(MakeDisc({0, 0}, 1), UnorderedElementsAre(Position2d{0, 0},   //
                                                        Position2d{-1, 0},  //
                                                        Position2d{1, 0},   //
                                                        Position2d{0, -1},  //
                                                        Position2d{0, 1}));
}

class Math2dAlgorithmTestRange : public ::testing::TestWithParam<int> {};

TEST_P(Math2dAlgorithmTestRange, VisitDiscEquivalenceTest) {
  const int radius = GetParam();
  Position2d center{radius + 1, radius + 1};
  int diameter = 2 * radius + 2;
  std::vector<int> expected_disc(diameter * diameter, 0);
  VisitDisc(center, radius, [&expected_disc, diameter](Position2d point) {
    int index = point.y * diameter + point.x;
    if (0 <= index && index < expected_disc.size()) {
      ++expected_disc[index];
    } else {
      ADD_FAILURE() << point;
    }
  });

  std::string rendered_disc;
  rendered_disc.reserve(diameter * diameter + diameter);
  constexpr char kLookup[] = ".*23456789";
  for (int i = 0; i < diameter; ++i) {
    for (int j = 0; j < diameter; ++j) {
      int index = i * diameter + j;
      ASSERT_LT(expected_disc[index] + 1, sizeof(kLookup));
      rendered_disc += kLookup[expected_disc[index]];
    }
    rendered_disc += "\n";
  }

  std::string target_disc;
  target_disc.reserve(diameter * diameter + diameter);
  int radius_squared = radius * radius;
  for (int i = 0; i < diameter; ++i) {
    for (int j = 0; j < diameter; ++j) {
      Position2d pos = {i, j};
      Vector2d delta = pos - center;
      int l2_norm_squared = delta.x * delta.x + delta.y * delta.y;
      target_disc += (l2_norm_squared <= radius_squared) ? '*' : '.';
    }
    target_disc += '\n';
  }
  EXPECT_EQ(rendered_disc, target_disc) << "Expected:\n"
                                        << target_disc << "Actual:\n"
                                        << rendered_disc;
}

INSTANTIATE_TEST_SUITE_P(VisitDiscEquivalenceTest, Math2dAlgorithmTestRange,
                         ::testing::Range(2, 40));

std::vector<Position2d> MakeDiamond(Position2d center, int radius) {
  std::vector<Position2d> result;
  VisitDiamond(center, radius,
               [&result](Position2d pos) { result.push_back(pos); });
  return result;
}

TEST(Math2dAlgorithmTest, VisitDiamondRadiusIsZero) {
  EXPECT_THAT(MakeDiamond({0, 0}, 0), UnorderedElementsAre(Position2d{0, 0}));
}

TEST(Math2dAlgorithmTest, VisitDiamondRadiusIsOne) {
  EXPECT_THAT(MakeDiamond({0, 0}, 1),
              UnorderedElementsAre(Position2d{0, 0},   //
                                   Position2d{-1, 0},  //
                                   Position2d{1, 0},   //
                                   Position2d{0, -1},  //
                                   Position2d{0, 1}));
}

TEST_P(Math2dAlgorithmTestRange, VisitDiamondEquivalenceTest) {
  const int radius = GetParam();
  Position2d center{radius + 1, radius + 1};
  int diameter = 2 * radius + 2;
  std::vector<int> expected_diamond(diameter * diameter, 0);
  VisitDiamond(center, radius, [&expected_diamond, diameter](Position2d point) {
    int idx = point.y * diameter + point.x;
    if (0 <= idx && idx < expected_diamond.size()) {
      ++expected_diamond[idx];
    } else {
      ADD_FAILURE() << point;
    }
  });

  std::string rendered_diamond;
  rendered_diamond.reserve(diameter * diameter + diameter);
  constexpr char kLookup[] = ".*23456789";
  for (int i = 0; i < diameter; ++i) {
    for (int j = 0; j < diameter; ++j) {
      int idx = i * diameter + j;
      ASSERT_LT(expected_diamond[idx] + 1, sizeof(kLookup));
      rendered_diamond += kLookup[expected_diamond[idx]];
    }
    rendered_diamond += "\n";
  }

  std::string target_diamond;
  target_diamond.reserve(diameter * diameter + diameter);
  for (int i = 0; i < diameter; ++i) {
    for (int j = 0; j < diameter; ++j) {
      Position2d pos = {i, j};
      Vector2d delta = pos - center;
      int l1_norm = std::abs(delta.x) + std::abs(delta.y);
      target_diamond += (l1_norm <= radius) ? '*' : '.';
    }
    target_diamond += '\n';
  }
  EXPECT_EQ(rendered_diamond, target_diamond);
}

INSTANTIATE_TEST_SUITE_P(VisitDiamondEquivalenceTest, Math2dAlgorithmTestRange,
                         ::testing::Range(2, 40));

}  // namespace
}  // namespace deepmind::lab2d::math
