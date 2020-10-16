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

#include "dmlab2d/system/math/math2d.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d::math {
namespace {
using ::testing::Eq;

TEST(Vector2Test, NegateVector2) {
  Vector2d a{3, 5};
  EXPECT_THAT(-a, Eq(Vector2d{-3, -5}));
}

TEST(Vector2Test, AddVector2) {
  Vector2d a{3, 5};
  Vector2d b{1, 2};
  EXPECT_THAT(a + b, Eq(Vector2d{4, 7}));
}

TEST(Vector2Test, SubVector2) {
  Vector2d a{3, 5};
  Vector2d b{1, 2};
  EXPECT_THAT(a - b, Eq(Vector2d{2, 3}));
}

TEST(Vector2Test, MulVector2) {
  Vector2d a{3, 5};
  EXPECT_THAT(a * 2, Eq(Vector2d{6, 10}));
  EXPECT_THAT(2 * a, Eq(Vector2d{6, 10}));
}

TEST(Vector2Test, DivVector2) {
  Vector2d a{4, 8};
  EXPECT_THAT(a / 2, Eq(Vector2d{2, 4}));
}

TEST(Vector2Test, InplaceAddVector2) {
  Vector2d a{3, 5};
  Vector2d b{1, 2};
  a += b;
  EXPECT_THAT(a, Eq(Vector2d{4, 7}));
}

TEST(Vector2Test, InplaceSubVector2) {
  Vector2d a{3, 5};
  Vector2d b{1, 2};
  a -= b;
  EXPECT_THAT(a, Eq(Vector2d{2, 3}));
}

TEST(Vector2Test, InplaceMulVector2) {
  Vector2d a{3, 5};
  a *= 2;
  EXPECT_THAT(a, Eq(Vector2d{6, 10}));
}

TEST(Vector2Test, InplaceDivVector2) {
  Vector2d a{4, 8};
  a /= 2;
  EXPECT_THAT(a, Eq(Vector2d{2, 4}));
}

TEST(Vector2Test, FromOrienation) {
  EXPECT_THAT(Vector2d::FromOrientation(Orientation2d::kNorth),
              Eq(Vector2d::North()));
  EXPECT_THAT(Vector2d::FromOrientation(Orientation2d::kEast),
              Eq(Vector2d::East()));
  EXPECT_THAT(Vector2d::FromOrientation(Orientation2d::kSouth),
              Eq(Vector2d::South()));
  EXPECT_THAT(Vector2d::FromOrientation(Orientation2d::kWest),
              Eq(Vector2d::West()));
}

TEST(Position2Test, AddPosition2) {
  Position2d a{3, 5};
  Vector2d b{1, 2};
  EXPECT_THAT(a + b, Eq(Position2d{4, 7}));
  EXPECT_THAT(b + a, Eq(Position2d{4, 7}));
}

TEST(Position2Test, SubPosition2) {
  Position2d a{3, 5};
  Vector2d b{1, 2};
  EXPECT_THAT(a - b, Eq(Position2d{2, 3}));
}

TEST(Position2Test, DiffPosition2) {
  Position2d a{3, 5};
  Position2d b{1, 2};
  EXPECT_THAT(a - b, Eq(Vector2d{2, 3}));
}

TEST(Position2Test, InplaceAddPosition2) {
  Position2d a{3, 5};
  Vector2d b{1, 2};
  a += b;
  EXPECT_THAT(a, Eq(Position2d{4, 7}));
}

TEST(Position2Test, InplaceSubPosition2) {
  Position2d a{3, 5};
  Vector2d b{1, 2};
  a -= b;
  EXPECT_THAT(a, Eq(Position2d{2, 3}));
}

TEST(Position2Test, OperatorEquals) {
  Position2d a0{3, 5};
  Position2d a1{3, 5};
  Position2d b0{3, 2};
  Position2d b1{2, 5};
  Position2d b2{2, 5};
  EXPECT_TRUE(a0 == a0);
  EXPECT_TRUE(a0 == a1);
  EXPECT_FALSE(a0 == b0);
  EXPECT_FALSE(a0 == b1);
  EXPECT_FALSE(a0 == b2);

  EXPECT_FALSE(a0 != a0);
  EXPECT_FALSE(a0 != a1);
  EXPECT_TRUE(a0 != b0);
  EXPECT_TRUE(a0 != b1);
  EXPECT_TRUE(a0 != b2);
}

TEST(Size2Test, NotContainsLower) {
  Size2d size{8, 10};
  EXPECT_THAT(size.Contains({0, 0}), Eq(true));
  EXPECT_THAT(size.Contains({-1, 0}), Eq(false));
  EXPECT_THAT(size.Contains({0, -1}), Eq(false));
  EXPECT_THAT(size.Contains({-1, -1}), Eq(false));
}

TEST(Size2Test, NotContainsUpper) {
  Size2d size{8, 10};
  EXPECT_THAT(size.Contains({7, 9}), Eq(true));
  EXPECT_THAT(size.Contains({8, 9}), Eq(false));
  EXPECT_THAT(size.Contains({7, 10}), Eq(false));
  EXPECT_THAT(size.Contains({9, 10}), Eq(false));
}

TEST(Size2Test, Area) {
  Size2d size{8, 10};
  EXPECT_THAT(size.Area(), Eq(80));
}

TEST(OrientationTest, TurnRight90) {
  EXPECT_THAT(Orientation2d::kNorth + Rotate2d::k90, Eq(Orientation2d::kEast));
  EXPECT_THAT(Orientation2d::kEast + Rotate2d::k90, Eq(Orientation2d::kSouth));
  EXPECT_THAT(Orientation2d::kSouth + Rotate2d::k90, Eq(Orientation2d::kWest));
  EXPECT_THAT(Orientation2d::kWest + Rotate2d::k90, Eq(Orientation2d::kNorth));
  EXPECT_THAT(Orientation2d::kNorth - Rotate2d::k270, Eq(Orientation2d::kEast));
  EXPECT_THAT(Orientation2d::kEast - Rotate2d::k270, Eq(Orientation2d::kSouth));
  EXPECT_THAT(Orientation2d::kSouth - Rotate2d::k270, Eq(Orientation2d::kWest));
  EXPECT_THAT(Orientation2d::kWest - Rotate2d::k270, Eq(Orientation2d::kNorth));
}

TEST(OrientationTest, TurnLeft90) {
  EXPECT_THAT(Orientation2d::kNorth + Rotate2d::k270, Eq(Orientation2d::kWest));
  EXPECT_THAT(Orientation2d::kNorth - Rotate2d::k90, Eq(Orientation2d::kWest));
  EXPECT_THAT(Orientation2d::kEast + Rotate2d::k270, Eq(Orientation2d::kNorth));
  EXPECT_THAT(Orientation2d::kEast - Rotate2d::k90, Eq(Orientation2d::kNorth));
  EXPECT_THAT(Orientation2d::kSouth + Rotate2d::k270, Eq(Orientation2d::kEast));
  EXPECT_THAT(Orientation2d::kSouth - Rotate2d::k90, Eq(Orientation2d::kEast));
  EXPECT_THAT(Orientation2d::kWest + Rotate2d::k270, Eq(Orientation2d::kSouth));
  EXPECT_THAT(Orientation2d::kWest - Rotate2d::k90, Eq(Orientation2d::kSouth));
}

TEST(OrientationTest, Turn180) {
  EXPECT_THAT(Orientation2d::kNorth + Rotate2d::k180,
              Eq(Orientation2d::kSouth));
  EXPECT_THAT(Orientation2d::kNorth - Rotate2d::k180,
              Eq(Orientation2d::kSouth));
  EXPECT_THAT(Orientation2d::kEast + Rotate2d::k180, Eq(Orientation2d::kWest));
  EXPECT_THAT(Orientation2d::kEast - Rotate2d::k180, Eq(Orientation2d::kWest));
  EXPECT_THAT(Orientation2d::kSouth + Rotate2d::k180,
              Eq(Orientation2d::kNorth));
  EXPECT_THAT(Orientation2d::kSouth - Rotate2d::k180,
              Eq(Orientation2d::kNorth));
  EXPECT_THAT(Orientation2d::kWest + Rotate2d::k180, Eq(Orientation2d::kEast));
  EXPECT_THAT(Orientation2d::kWest - Rotate2d::k180, Eq(Orientation2d::kEast));
}

TEST(OrientationTest, SubOrientationRight90) {
  EXPECT_THAT(Orientation2d::kEast - Orientation2d::kNorth, Eq(Rotate2d::k90));
  EXPECT_THAT(Orientation2d::kSouth - Orientation2d::kEast, Eq(Rotate2d::k90));
  EXPECT_THAT(Orientation2d::kWest - Orientation2d::kSouth, Eq(Rotate2d::k90));
  EXPECT_THAT(Orientation2d::kNorth - Orientation2d::kWest, Eq(Rotate2d::k90));
}

TEST(OrientationTest, SubOrientationLeft90) {
  EXPECT_THAT(Orientation2d::kWest - Orientation2d::kNorth, Eq(Rotate2d::k270));
  EXPECT_THAT(Orientation2d::kSouth - Orientation2d::kWest, Eq(Rotate2d::k270));
  EXPECT_THAT(Orientation2d::kEast - Orientation2d::kSouth, Eq(Rotate2d::k270));
  EXPECT_THAT(Orientation2d::kNorth - Orientation2d::kEast, Eq(Rotate2d::k270));
}

TEST(OrientationTest, SubOrientationRotate180) {
  EXPECT_THAT(Orientation2d::kNorth - Orientation2d::kSouth,
              Eq(Rotate2d::k180));
  EXPECT_THAT(Orientation2d::kSouth - Orientation2d::kNorth,
              Eq(Rotate2d::k180));
  EXPECT_THAT(Orientation2d::kEast - Orientation2d::kWest, Eq(Rotate2d::k180));
  EXPECT_THAT(Orientation2d::kWest - Orientation2d::kEast, Eq(Rotate2d::k180));
}

TEST(OrientationTest, FromViewOrientationNorth) {
  EXPECT_THAT(FromView(Orientation2d::kNorth, Orientation2d::kNorth),
              Eq(Orientation2d::kNorth));
  EXPECT_THAT(FromView(Orientation2d::kNorth, Orientation2d::kEast),
              Eq(Orientation2d::kEast));
  EXPECT_THAT(FromView(Orientation2d::kNorth, Orientation2d::kSouth),
              Eq(Orientation2d::kSouth));
  EXPECT_THAT(FromView(Orientation2d::kNorth, Orientation2d::kWest),
              Eq(Orientation2d::kWest));
}

TEST(OrientationTest, FromViewOrientationEast) {
  EXPECT_THAT(FromView(Orientation2d::kEast, Orientation2d::kNorth),
              Eq(Orientation2d::kWest));
  EXPECT_THAT(FromView(Orientation2d::kEast, Orientation2d::kEast),
              Eq(Orientation2d::kNorth));
  EXPECT_THAT(FromView(Orientation2d::kEast, Orientation2d::kSouth),
              Eq(Orientation2d::kEast));
  EXPECT_THAT(FromView(Orientation2d::kEast, Orientation2d::kWest),
              Eq(Orientation2d::kSouth));
}

TEST(OrientationTest, FromViewOrientationSouth) {
  EXPECT_THAT(FromView(Orientation2d::kSouth, Orientation2d::kNorth),
              Eq(Orientation2d::kSouth));
  EXPECT_THAT(FromView(Orientation2d::kSouth, Orientation2d::kEast),
              Eq(Orientation2d::kWest));
  EXPECT_THAT(FromView(Orientation2d::kSouth, Orientation2d::kSouth),
              Eq(Orientation2d::kNorth));
  EXPECT_THAT(FromView(Orientation2d::kSouth, Orientation2d::kWest),
              Eq(Orientation2d::kEast));
}

TEST(OrientationTest, FromViewOrientationWest) {
  EXPECT_THAT(FromView(Orientation2d::kWest, Orientation2d::kNorth),
              Eq(Orientation2d::kEast));
  EXPECT_THAT(FromView(Orientation2d::kWest, Orientation2d::kEast),
              Eq(Orientation2d::kSouth));
  EXPECT_THAT(FromView(Orientation2d::kWest, Orientation2d::kSouth),
              Eq(Orientation2d::kWest));
  EXPECT_THAT(FromView(Orientation2d::kWest, Orientation2d::kWest),
              Eq(Orientation2d::kNorth));
}

TEST(RotateTest, RotateTestNorth) {
  EXPECT_THAT(Vector2d::North() * Rotate2d::k0, Eq(Vector2d::North()));
  EXPECT_THAT(Vector2d::North() * Rotate2d::k90, Eq(Vector2d::East()));
  EXPECT_THAT(Vector2d::North() * Rotate2d::k180, Eq(Vector2d::South()));
  EXPECT_THAT(Vector2d::North() * Rotate2d::k270, Eq(Vector2d::West()));
}

TEST(RotateTest, RotateTestEast) {
  EXPECT_THAT(Rotate2d::k0 * Vector2d::East(), Eq(Vector2d::East()));
  EXPECT_THAT(Rotate2d::k90 * Vector2d::East(), Eq(Vector2d::South()));
  EXPECT_THAT(Rotate2d::k180 * Vector2d::East(), Eq(Vector2d::West()));
  EXPECT_THAT(Rotate2d::k270 * Vector2d::East(), Eq(Vector2d::North()));
}

TEST(RotateTest, RotateTestSouth) {
  EXPECT_THAT(Vector2d::South() * Rotate2d::k0, Eq(Vector2d::South()));
  EXPECT_THAT(Vector2d::South() * Rotate2d::k90, Eq(Vector2d::West()));
  EXPECT_THAT(Vector2d::South() * Rotate2d::k180, Eq(Vector2d::North()));
  EXPECT_THAT(Vector2d::South() * Rotate2d::k270, Eq(Vector2d::East()));
}

TEST(RotateTest, RotateTestWest) {
  EXPECT_THAT(Rotate2d::k0 * Vector2d::West(), Eq(Vector2d::West()));
  EXPECT_THAT(Rotate2d::k90 * Vector2d::West(), Eq(Vector2d::North()));
  EXPECT_THAT(Rotate2d::k180 * Vector2d::West(), Eq(Vector2d::East()));
  EXPECT_THAT(Rotate2d::k270 * Vector2d::West(), Eq(Vector2d::South()));
}

TEST(RotateTest, RotateTestZero) {
  EXPECT_THAT(Rotate2d::k0 * Vector2d::Zero(), Eq(Vector2d::Zero()));
  EXPECT_THAT(Rotate2d::k90 * Vector2d::Zero(), Eq(Vector2d::Zero()));
  EXPECT_THAT(Rotate2d::k180 * Vector2d::Zero(), Eq(Vector2d::Zero()));
  EXPECT_THAT(Rotate2d::k270 * Vector2d::Zero(), Eq(Vector2d::Zero()));
}

TEST(RotateTest, RotateTestInPlace) {
  Vector2d direction = {4, 3};
  EXPECT_THAT(direction *= Rotate2d::k0, Eq(Vector2d{4, 3}));
  EXPECT_THAT(direction *= Rotate2d::k90, Eq(Vector2d{-3, 4}));
  EXPECT_THAT(direction *= Rotate2d::k180, Eq(Vector2d{3, -4}));
  EXPECT_THAT(direction *= Rotate2d::k270, Eq(Vector2d{-4, -3}));
}

TEST(RotateTest, RotateTestAdd) {
  EXPECT_THAT(Rotate2d::k0 + Rotate2d::k0, Eq(Rotate2d::k0));
  EXPECT_THAT(Rotate2d::k0 + Rotate2d::k90, Eq(Rotate2d::k90));
  EXPECT_THAT(Rotate2d::k0 + Rotate2d::k180, Eq(Rotate2d::k180));
  EXPECT_THAT(Rotate2d::k0 + Rotate2d::k270, Eq(Rotate2d::k270));

  EXPECT_THAT(Rotate2d::k90 + Rotate2d::k0, Eq(Rotate2d::k90));
  EXPECT_THAT(Rotate2d::k90 + Rotate2d::k90, Eq(Rotate2d::k180));
  EXPECT_THAT(Rotate2d::k90 + Rotate2d::k180, Eq(Rotate2d::k270));
  EXPECT_THAT(Rotate2d::k90 + Rotate2d::k270, Eq(Rotate2d::k0));

  EXPECT_THAT(Rotate2d::k180 + Rotate2d::k0, Eq(Rotate2d::k180));
  EXPECT_THAT(Rotate2d::k180 + Rotate2d::k90, Eq(Rotate2d::k270));
  EXPECT_THAT(Rotate2d::k180 + Rotate2d::k180, Eq(Rotate2d::k0));
  EXPECT_THAT(Rotate2d::k180 + Rotate2d::k270, Eq(Rotate2d::k90));

  EXPECT_THAT(Rotate2d::k270 + Rotate2d::k0, Eq(Rotate2d::k270));
  EXPECT_THAT(Rotate2d::k270 + Rotate2d::k90, Eq(Rotate2d::k0));
  EXPECT_THAT(Rotate2d::k270 + Rotate2d::k180, Eq(Rotate2d::k90));
  EXPECT_THAT(Rotate2d::k270 + Rotate2d::k270, Eq(Rotate2d::k180));
}

TEST(RotateTest, RotateTestSubtract) {
  EXPECT_THAT(Rotate2d::k0 - Rotate2d::k0, Eq(Rotate2d::k0));
  EXPECT_THAT(Rotate2d::k0 - Rotate2d::k90, Eq(Rotate2d::k270));
  EXPECT_THAT(Rotate2d::k0 - Rotate2d::k180, Eq(Rotate2d::k180));
  EXPECT_THAT(Rotate2d::k0 - Rotate2d::k270, Eq(Rotate2d::k90));

  EXPECT_THAT(Rotate2d::k90 - Rotate2d::k0, Eq(Rotate2d::k90));
  EXPECT_THAT(Rotate2d::k90 - Rotate2d::k90, Eq(Rotate2d::k0));
  EXPECT_THAT(Rotate2d::k90 - Rotate2d::k180, Eq(Rotate2d::k270));
  EXPECT_THAT(Rotate2d::k90 - Rotate2d::k270, Eq(Rotate2d::k180));

  EXPECT_THAT(Rotate2d::k180 - Rotate2d::k0, Eq(Rotate2d::k180));
  EXPECT_THAT(Rotate2d::k180 - Rotate2d::k90, Eq(Rotate2d::k90));
  EXPECT_THAT(Rotate2d::k180 - Rotate2d::k180, Eq(Rotate2d::k0));
  EXPECT_THAT(Rotate2d::k180 - Rotate2d::k270, Eq(Rotate2d::k270));

  EXPECT_THAT(Rotate2d::k270 - Rotate2d::k0, Eq(Rotate2d::k270));
  EXPECT_THAT(Rotate2d::k270 - Rotate2d::k90, Eq(Rotate2d::k180));
  EXPECT_THAT(Rotate2d::k270 - Rotate2d::k180, Eq(Rotate2d::k90));
  EXPECT_THAT(Rotate2d::k270 - Rotate2d::k270, Eq(Rotate2d::k0));
}

}  // namespace
}  // namespace deepmind::lab2d::math
