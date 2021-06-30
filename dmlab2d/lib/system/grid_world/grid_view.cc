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

#include "dmlab2d/lib/system/grid_world/grid_view.h"

#include <algorithm>

#include "dmlab2d/lib/system/grid_world/sprite_instance.h"
#include "dmlab2d/lib/system/math/math2d.h"

namespace deepmind::lab2d {

void GridView::ClearOutOfViewSprites(math::Orientation2d orientation,
                                     absl::Span<int> sprite_ids) const {
  CHECK_EQ(sprite_ids.size(), NumCells()) << "Invalid number of sprite_ids.";
  if (!window_.centered()) {
    return;
  }

  SpriteInstance out_of_view{out_of_view_sprite_, orientation};
  int sprites_per_cell = num_render_layers_;
  int sprites_per_row = window_.width() * sprites_per_cell;
  int top_start = 0;
  int top_end = 0;
  int bottom_start = window_.height();
  int bottom_end = bottom_start;

  int left_start = 0;
  int left_end = 0;
  int right_start = window_.width();
  int right_end = right_start;

  switch (orientation) {
    case math::Orientation2d::kNorth:
      top_end += window_.forward() - window_.forward_actual();
      bottom_start -= window_.backward() - window_.backward_actual();

      left_end += window_.left() - window_.left_actual();
      right_start -= window_.right() - window_.right_actual();
      break;
    case math::Orientation2d::kEast:
      top_end += window_.left() - window_.left_actual();
      bottom_start -= window_.right() - window_.right_actual();

      left_end += window_.backward() - window_.backward_actual();
      right_start -= window_.forward() - window_.forward_actual();
      break;
    case math::Orientation2d::kSouth:
      top_end += window_.backward() - window_.backward_actual();
      bottom_start -= window_.forward() - window_.forward_actual();

      left_end += window_.right() - window_.right_actual();
      right_start -= window_.left() - window_.left_actual();
      break;
    case math::Orientation2d::kWest:
      top_end += window_.right() - window_.right_actual();
      bottom_start -= window_.left() - window_.left_actual();

      left_end += window_.forward() - window_.forward_actual();
      right_start -= window_.backward() - window_.backward_actual();
      break;
  }
  int out_of_view_id = ToSpriteId(out_of_view);
  int row = top_start;
  for (; row < top_end; ++row) {
    std::fill(sprite_ids.begin() + sprites_per_row * row,
              sprite_ids.begin() + sprites_per_row * (row + 1), out_of_view_id);
  }
  for (; row < bottom_start; ++row) {
    std::fill(sprite_ids.begin() + sprites_per_row * row +
                  left_start * sprites_per_cell,
              sprite_ids.begin() + sprites_per_row * row +
                  left_end * sprites_per_cell,
              out_of_view_id);
    std::fill(sprite_ids.begin() + sprites_per_row * row +
                  right_start * sprites_per_cell,
              sprite_ids.begin() + sprites_per_row * row +
                  right_end * sprites_per_cell,
              out_of_view_id);
  }
  for (; row < bottom_end; ++row) {
    std::fill(sprite_ids.begin() + sprites_per_row * row,
              sprite_ids.begin() + sprites_per_row * (row + 1), out_of_view_id);
  }
}

}  // namespace deepmind::lab2d
