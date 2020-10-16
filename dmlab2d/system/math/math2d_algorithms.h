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

#ifndef DMLAB2D_SYSTEM_MATH_MATH2D_ALGORITHMS_H_
#define DMLAB2D_SYSTEM_MATH_MATH2D_ALGORITHMS_H_

#include <cmath>

#include "dmlab2d/system/math/math2d.h"

namespace deepmind::lab2d::math {

// Calls has_hit_func for all points from `p0` to `p1` in a straight line with
// orthoganal steps until has_hit_func(position) returns true or the `p1` is
// reached. Returns whether has_hit_func returns true at any point.
// Note 'p0' is not tested.
// Based upon a modified Bresenham line-algorithm:
// https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
//   01234567890
// 0 S*
// 1  ****
// 2     ***
// 3       ****
// 4          *E
template <typename Pred>
bool RayCastLine(Position2d p0, Position2d p1, Pred has_hit_func) {
  const int abs_delta_x = std::abs(p1.x - p0.x);
  const int abs_delta_y = std::abs(p1.y - p0.y);
  const int step_x = p0.x < p1.x ? 1 : -1;
  const int step_y = p0.y < p1.y ? 1 : -1;
  int error = abs_delta_x - abs_delta_y;
  while (p0 != p1) {
    if (error < 0) {
      p0.y += step_y;
      error += 2 * abs_delta_x;
    } else {
      p0.x += step_x;
      error -= 2 * abs_delta_y;
    }
    if (has_hit_func(p0)) {
      return true;
    }
  }
  return false;
}

// Visits all points 'p' in inclusive rectangle formed between `corner0` and
// `corner1`.
template <typename Visitor>
void VisitRectangle(Position2d corner0, Position2d corner1, Visitor visitor) {
  Position2d top_left{std::min(corner0.x, corner1.x),
                      std::min(corner0.y, corner1.y)};

  Position2d bottom_right{std::max(corner0.x, corner1.x),
                          std::max(corner0.y, corner1.y)};

  for (int y = top_left.y; y <= bottom_right.y; ++y) {
    for (int x = top_left.x; x <= bottom_right.x; ++x) {
      visitor(Position2d{x, y});
    }
  }
}

// Visits all points 'p' in inclusive rectangle formed between `corner0` and
// `corner1` clamped to `window`.
template <typename Visitor>
void VisitRectangleClamped(Position2d corner0, Position2d corner1,
                           Size2d window, Visitor visitor) {
  Position2d top_left{std::max(std::min(corner0.x, corner1.x), 0),
                      std::max(std::min(corner0.y, corner1.y), 0)};

  Position2d bottom_right{
      std::min(std::max(corner0.x, corner1.x), window.width - 1),
      std::min(std::max(corner0.y, corner1.y), window.height - 1)};

  for (int y = top_left.y; y <= bottom_right.y; ++y) {
    for (int x = top_left.x; x <= bottom_right.x; ++x) {
      visitor(Position2d{x, y});
    }
  }
}

// Visits all points 'p' such that magnintude of `(p - center)` is less than or
// equal to radius. Modified version of Midpoint circle algorithm. Ensures each
// point is only visited once.
// https://en.wikipedia.org/wiki/Midpoint_circle_algorithm Modification is such
// that the center grid squares visited are within the disc.
template <typename Visitor>
void VisitDisc(Position2d center, int radius, Visitor visitor) {
  auto visit_line = [&visitor, center](int x0, int x1, int y) {
    for (int x = x0; x <= x1; ++x) {
      visitor(Position2d{center.x + x, center.y + y});
    }
  };

  int delta_error_y = 1 - (radius * 2);
  int delta_error_x = 1;
  int error = 0;

  for (int x = 0, y = radius; x <= y;) {
    visit_line(-y, y, x);
    if (x > 0) {
      visit_line(-y, y, -x);
    }

    error += delta_error_x;
    delta_error_x += 2;
    if (error > 0) {
      if (x != y) {
        visit_line(-x, x, y);
        visit_line(-x, x, -y);
      }
      --y;
      error += delta_error_y;
      delta_error_y += 2;
    }
    ++x;
  }
}

// Visits all points 'p' such that L1 Norm of `(p - center)` is less than or
// equal to radius. See https://en.wikipedia.org/wiki/Taxicab_geometry
template <typename Visitor>
void VisitDiamond(Position2d center, int radius, Visitor visitor) {
  auto visit_line = [&visitor, center](int x0, int x1, int y) {
    for (int x = x0; x <= x1; ++x) {
      visitor(Position2d{center.x + x, center.y + y});
    }
  };
  for (int y = -radius; y < 0; ++y) {
    int x = radius + y;
    visit_line(-x, x, y);
  }
  for (int y = 0; y <= radius; ++y) {
    int x = radius - y;
    visit_line(-x, x, y);
  }
}

}  // namespace deepmind::lab2d::math

#endif  // DMLAB2D_SYSTEM_MATH_MATH2D_ALGORITHMS_H_
