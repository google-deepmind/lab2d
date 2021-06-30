// Copyright (C) 2016-2019 The DMLab2D Authors.
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

#include "dmlab2d/lib/system/tensor/tensor_view.h"

#include <algorithm>
#include <iomanip>
#include <iterator>
#include <string>
#include <vector>

namespace deepmind::lab2d {
namespace tensor {

void Layout::PrintToStream(
    std::size_t max_num_elements, std::ostream* os,
    std::function<void(std::ostream* os, std::size_t offset)> printer) const {
  std::size_t num_elements = this->num_elements();
  *os << std::setfill(' ');
  const auto& s = shape();
  if (s.empty()) {
    printer(os, offset_);
    *os << "\n";
    return;
  }

  *os << "Shape: [";
  for (auto it = s.begin(); it != s.end(); ++it) {
    if (it != s.begin()) {
      *os << ", ";
    }
    *os << *it;
  }
  *os << "]";

  if (num_elements == 0) {
    *os << " Empty\n";
    return;
  } else {
    *os << "\n";
  }

  struct Skip {
    std::size_t low = 0;
    std::size_t high = 0;
  };

  std::vector<Skip> skips(shape().size());
  if (num_elements > max_num_elements) {
    for (std::size_t i = 0; i < skips.size(); ++i) {
      if (shape()[i] < 7) continue;
      skips[i].low = 3;
      skips[i].high = shape()[i] - 3;
    }
  }

  std::size_t max_width = 0;
  ForEachOffset([&max_width, &printer](std::size_t offset) {
    std::ostringstream vals;
    printer(&vals, offset);
    max_width = std::max<std::size_t>(max_width, vals.tellp());
    return true;
  });

  ForEachIndexedOffset([os, &s, &printer, max_width, &skips](
                           const ShapeVector& index, std::size_t offset) {
    int open_brackets = std::distance(
        index.rbegin(), std::find_if(index.rbegin(), index.rend(),
                                     [](std::size_t val) { return val != 0; }));
    for (std::size_t i = 0; i < index.size(); ++i) {
      if (skips[i].low >= skips[i].high) {
        continue;
      }
      if (skips[i].low <= index[i] && index[i] < skips[i].high) {
        // If first skipped at top rank print '...'.
        if (index[i] == skips[i].low && open_brackets + i + 1 >= index.size()) {
          if (open_brackets != 0) {
            *os << std::string((s.size() - open_brackets), ' ') << "...,"
                << std::string(open_brackets, '\n');
          } else {
            *os << "..., ";
          }
        }
        return true;
      }
    }

    if (open_brackets != 0) {
      *os << std::string((s.size() - open_brackets), ' ');
      *os << std::string(open_brackets, '[');
    }
    *os << std::setw(max_width);
    printer(os, offset);
    if (index.back() + 1 != s.back()) {
      *os << ", ";
    } else {
      auto s_iter = s.rbegin();
      int closing_brackets = 0;
      for (auto it = index.rbegin(); it != index.rend() && (*it) + 1 == *s_iter;
           ++it, ++s_iter) {
        ++closing_brackets;
      }
      *os << std::string(closing_brackets, ']');
      if (closing_brackets < index.size()) {
        *os << ',' << std::string(closing_brackets, '\n');
      }
    }
    return true;
  });
}

}  // namespace tensor
}  // namespace deepmind::lab2d
