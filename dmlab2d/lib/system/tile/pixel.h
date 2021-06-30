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

// Defines `Pixel` and `PixelByte` types for blending and manipulating images.

#ifndef DMLAB2D_LIB_SYSTEM_TILE_PIXEL_H_
#define DMLAB2D_LIB_SYSTEM_TILE_PIXEL_H_

#include "dmlab2d/lib/support/logging.h"

namespace deepmind::lab2d {

// One byte of a Pixel.
//
// Note: unlike the unsigned char type itself, the PixelByte type does not have
// a special role in the language that would allow it to alias any other object,
// and is thus easier and cheaper to reason about.
enum class PixelByte : unsigned char {
  Min = 0,
  Max = 255,
};

constexpr inline unsigned char AsUChar(PixelByte b) {
  return static_cast<unsigned char>(b);
}

// Single pixel. Small enough to be passed by value.
struct Pixel {
  PixelByte r;
  PixelByte g;
  PixelByte b;

  static constexpr Pixel Black() {
    return Pixel{PixelByte::Min, PixelByte::Min, PixelByte::Min};
  }

  static constexpr Pixel White() {
    return Pixel{PixelByte::Max, PixelByte::Max, PixelByte::Max};
  }

  friend constexpr bool operator==(Pixel lhs, Pixel rhs) {
    return lhs.r == rhs.r && lhs.g == rhs.g && lhs.b == rhs.b;
  }
};

// A restricted form of `Interp()`, where `alpha` must be either
// `PixelByte::Max` ("fully opaque", returns `to`) or
// `PixelByte::Min` ("fully transparent", returns `from`).
inline Pixel InterpOneBit(Pixel from, Pixel to, PixelByte alpha) {
  DCHECK(alpha == PixelByte::Max || alpha == PixelByte::Min) << "Invalid alpha";
  return alpha != PixelByte::Min ? to : from;
}

// Returns `from` interpolated to `to` by `alpha`.
constexpr inline Pixel Interp(Pixel from, Pixel to, PixelByte alpha) {
  constexpr unsigned int kMax = AsUChar(PixelByte::Max);
  constexpr unsigned int kMin = AsUChar(PixelByte::Min);
  constexpr unsigned int kHalf = (kMax - kMin) / 2 + kMin;
  const unsigned int pattern_w_1 = AsUChar(alpha);
  const unsigned int pattern_w_0 = kMax - pattern_w_1;
  const unsigned int r1 = AsUChar(from.r);
  const unsigned int g1 = AsUChar(from.g);
  const unsigned int b1 = AsUChar(from.b);
  const unsigned int r2 = AsUChar(to.r);
  const unsigned int g2 = AsUChar(to.g);
  const unsigned int b2 = AsUChar(to.b);
  return {PixelByte((pattern_w_0 * r1 + pattern_w_1 * r2 + kHalf) / kMax),
          PixelByte((pattern_w_0 * g1 + pattern_w_1 * g2 + kHalf) / kMax),
          PixelByte((pattern_w_0 * b1 + pattern_w_1 * b2 + kHalf) / kMax)};
}

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_TILE_PIXEL_H_
