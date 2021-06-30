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

#ifndef DMLAB2D_LIB_SYSTEM_GRID_WORLD_GRID_WINDOW_H_
#define DMLAB2D_LIB_SYSTEM_GRID_WORLD_GRID_WINDOW_H_

#include <algorithm>

#include "dmlab2d/lib/system/math/math2d.h"

namespace deepmind::lab2d {

// Represents the cells visible from a location.
class GridWindow {
 public:
  // Number of cells visible on each direction. If centered the window size
  // is reported as the max of each dimension and the `*_actual()` can be
  // used to get the underlying shape.
  constexpr GridWindow(bool centered, int left, int right, int forward,
                       int backward)
      : centered_(centered),
        max_dim_(std::max({left, right, forward, backward})),
        left_(left),
        right_(right),
        forward_(forward),
        backward_(backward) {}
  constexpr bool centered() const { return centered_; }
  constexpr math::Size2d size2d() const { return {width(), height()}; }
  constexpr int width() const { return left() + right() + 1; }
  constexpr int height() const { return backward() + forward() + 1; }
  constexpr int left() const { return centered_ ? max_dim_ : left_; }
  constexpr int right() const { return centered_ ? max_dim_ : right_; }
  constexpr int forward() const { return centered_ ? max_dim_ : forward_; }
  constexpr int backward() const { return centered_ ? max_dim_ : backward_; }

  constexpr int left_actual() const { return left_; }
  constexpr int right_actual() const { return right_; }
  constexpr int forward_actual() const { return forward_; }
  constexpr int backward_actual() const { return backward_; }

 private:
  bool centered_;
  int max_dim_;
  int left_;
  int right_;
  int forward_;
  int backward_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_GRID_WORLD_GRID_WINDOW_H_
