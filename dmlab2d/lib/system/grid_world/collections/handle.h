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

#ifndef DMLAB2D_LIB_SYSTEM_GRID_WORLD_COLLECTIONS_HANDLE_H_
#define DMLAB2D_LIB_SYSTEM_GRID_WORLD_COLLECTIONS_HANDLE_H_

#include <ostream>

#include "dmlab2d/lib/support/logging.h"

namespace deepmind::lab2d {

// `Handle<Tag>` is a strongly typed index for collections.
// `Tag` is used for identifcation purposes only. Handles can be in two states;
// either a handle is empty, or it represents an index into a collection.
template <typename Tag>
class Handle {
 public:
  using ValueType = int;
  static constexpr ValueType kEmptyElement = ValueType(-1);

  // Constructs an empty handle.
  constexpr Handle() : value_(kEmptyElement) {}

  // Contructs a handle of a given value. If the value is `kEmptyElement` then
  // the handle will be empty and value may not be read.
  constexpr explicit Handle(ValueType value) : value_(value) {}

  // Requires: *this is not empty.
  ValueType Value() const {
    DCHECK(!IsEmpty()) << Tag::kName << " is empty!";
    return value_;
  }

  // Getting the value of a handle requires that the handle is not empty.
  constexpr bool IsEmpty() const { return value_ == kEmptyElement; }

  // Full support for Abseil hashing, comparing and sorting.
  constexpr bool operator==(Handle rhs) const { return value_ == rhs.value_; }
  constexpr bool operator!=(Handle rhs) const { return value_ != rhs.value_; }
  constexpr bool operator<(Handle rhs) const { return value_ < rhs.value_; }
  constexpr bool operator>(Handle rhs) const { return value_ > rhs.value_; }
  constexpr bool operator<=(Handle rhs) const { return value_ <= rhs.value_; }
  constexpr bool operator>=(Handle rhs) const { return value_ >= rhs.value_; }

  template <typename H>
  friend H AbslHashValue(H h, Handle c) {
    return H::combine(std::move(h), c.value_);
  }

  friend std::ostream& operator<<(std::ostream& out, const Handle& handle) {
    if (handle.IsEmpty()) {
      return out << Tag::kName << "(<empty>)";
    } else {
      return out << Tag::kName << "(" << handle.value_ << ")";
    }
  }

 private:
  ValueType value_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_GRID_WORLD_COLLECTIONS_HANDLE_H_
