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

#include "dmlab2d/lib/system/grid_world/text_tools.h"

#include <vector>

#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "dmlab2d/lib/system/math/math2d.h"

namespace deepmind::lab2d {

absl::string_view RemoveLeadingAndTrailingNewLines(absl::string_view text) {
  if (const auto prefix = text.find_first_not_of('\n');
      prefix != std::string::npos && prefix > 0) {
    text.remove_prefix(prefix);
  }

  if (const auto suffix = text.find_last_not_of('\n');
      suffix != std::string::npos && suffix > 0) {
    text.remove_suffix(text.size() - suffix - 1);
  }
  return text;
}

math::Size2d GetSize2dOfText(absl::string_view layout) {
  layout = RemoveLeadingAndTrailingNewLines(layout);
  if (layout.empty()) {
    return math::Size2d{0, 0};
  }
  std::vector<absl::string_view> lines = absl::StrSplit(layout, '\n');
  math::Size2d grid_size = {};
  grid_size.height = lines.size();
  if (grid_size.height > 0) {
    const auto longest_line =
        std::max_element(lines.begin(), lines.end(),
                         [](absl::string_view lhs, absl::string_view rhs) {
                           return lhs.size() < rhs.size();
                         });
    grid_size.width = static_cast<int>(longest_line->size());
  }
  return grid_size;
}

}  // namespace deepmind::lab2d
