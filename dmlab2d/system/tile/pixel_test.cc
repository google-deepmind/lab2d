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

#include "dmlab2d/system/tile/pixel.h"

#include <cmath>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {

using ::testing::Eq;

static_assert(sizeof(Pixel) == 3, "Pixel must be exactly 3 bytes.");
static_assert(alignof(Pixel) == 1, "Pixel must have one byte alignment");

constexpr Pixel MakePixel(unsigned char r, unsigned char g, unsigned char b) {
  return Pixel{PixelByte(r), PixelByte(g), PixelByte(b)};
}

TEST(PixelTest, InterpRound) {
  EXPECT_THAT(
      Interp(Pixel::Black(), Pixel::White(), static_cast<PixelByte>(127)),
      Eq(MakePixel(127, 127, 127)));
  EXPECT_THAT(
      Interp(Pixel::White(), Pixel::Black(), static_cast<PixelByte>(127)),
      Eq(MakePixel(128, 128, 128)));
}

TEST(PixelTest, InterpRgb) {
  EXPECT_THAT(Interp(Pixel::Black(), MakePixel(255, 127, 0),
                     static_cast<PixelByte>(127)),
              Eq(MakePixel(127, 63, 0)));
  EXPECT_THAT(Interp(Pixel::White(), MakePixel(255, 127, 0),
                     static_cast<PixelByte>(127)),
              Eq(MakePixel(255, 127 + 64, 128)));
}

TEST(PixelTest, InterpOneBit) {
  EXPECT_THAT(Interp(Pixel::Black(), Pixel::White(), static_cast<PixelByte>(0)),
              Eq(Pixel::Black()));
  EXPECT_THAT(
      Interp(Pixel::Black(), Pixel::White(), static_cast<PixelByte>(255)),
      Eq(Pixel::White()));
}

TEST(PixelTest, InterpAllBlack) {
  for (int p = 0; p < 256; ++p) {
    for (int a = 0; a < 256; ++a) {
      auto blend =
          Interp(Pixel::Black(), MakePixel(p, p, p), static_cast<PixelByte>(a));
      double alpha = a / 255.0;
      ASSERT_THAT(static_cast<int>(blend.r), Eq(std::round(alpha * p)));
    }
  }
}

TEST(PixelTest, InterpAllWhite) {
  for (int p = 0; p < 256; ++p) {
    for (int a = 0; a < 256; ++a) {
      auto blend =
          Interp(Pixel::White(), MakePixel(p, p, p), static_cast<PixelByte>(a));
      double alpha = a / 255.0;
      ASSERT_THAT(static_cast<int>(blend.r),
                  Eq(std::round(alpha * p + (1.0 - alpha) * 255)));
    }
  }
}

TEST(PixelTest, InterpAllGrey) {
  for (int p = 0; p < 256; ++p) {
    for (int a = 0; a < 256; ++a) {
      auto blend = Interp(MakePixel(127, 127, 127), MakePixel(p, p, p),
                          static_cast<PixelByte>(a));
      double alpha = a / 255.0;
      ASSERT_THAT(static_cast<int>(blend.r),
                  Eq(std::round(alpha * p + (1.0 - alpha) * 127)));
    }
  }
}

}  // namespace
}  // namespace deepmind::lab2d
