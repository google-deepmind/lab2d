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

#ifndef DMLAB2D_LIB_SYSTEM_GRID_WORLD_TEXT_TOOLS_H_
#define DMLAB2D_LIB_SYSTEM_GRID_WORLD_TEXT_TOOLS_H_

#include <array>
#include <limits>

#include "absl/strings/string_view.h"
#include "dmlab2d/lib/system/grid_world/handles.h"
#include "dmlab2d/lib/system/math/math2d.h"

namespace deepmind::lab2d {

// A class for mapping text characters to states.
class CharMap {
 public:
  State& operator[](char c) { return data_[ToChar(c)]; }
  const State& operator[](char c) const { return data_[ToChar(c)]; }

 private:
  static constexpr unsigned char ToChar(char value) {
    return static_cast<unsigned char>(value);
  }
  std::array<State, std::numeric_limits<unsigned char>::max() + 1> data_ = {};
};

// Removes only leading and trailing new-lines. Internal new-lines are
// maintained.
absl::string_view RemoveLeadingAndTrailingNewLines(absl::string_view text);

// Returns the shape of layout after the leading and trailing new-lines are
// removed.
math::Size2d GetSize2dOfText(absl::string_view layout);

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_GRID_WORLD_TEXT_TOOLS_H_
