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

#ifndef DMLAB2D_LIB_SYSTEM_GRID_WORLD_COLLECTIONS_OBJECT_POOL_H_
#define DMLAB2D_LIB_SYSTEM_GRID_WORLD_COLLECTIONS_OBJECT_POOL_H_

#include <utility>
#include <vector>

#include "dmlab2d/lib/support/logging.h"

namespace deepmind::lab2d {

// A pool of instances of type `T` accessed by a handle.
template <typename Handle, typename T>
class ObjectPool {
 public:
  // Creates or recycles an instance of `T`. Returns the handle to that
  // instance. Calls invalidate references to objects owned by the pool.
  template <typename... Args>
  [[nodiscard]] Handle Create(Args&&... args) {
    if (unused_handles_.empty()) {
      int new_index = values_.size();
      values_.emplace_back(std::forward<Args>(args)...);
#ifndef NDEBUG
      engaged_.push_back(true);
#endif
      return Handle(new_index);
    } else {
      Handle handle = unused_handles_.back();
      unused_handles_.pop_back();
#ifndef NDEBUG
      CHECK(!engaged_[handle.Value()]) << "Unused handle still engaged";
      engaged_[handle.Value()] = true;
#endif
      values_[handle.Value()] = T(std::forward<Args>(args)...);
      return handle;
    }
  }

  // Releases an object back to the pool. The handle shall not be empty or
  // released. When NDEBUG is defined, preconditions are not checked at runtime.
  void Release(Handle handle) {
#ifndef NDEBUG
    CHECK(engaged_.size() > handle.Value() && engaged_[handle.Value()])
        << "Object removed twice! " << handle.Value();
#endif
    if (unused_handles_.size() + 1 != values_.size()) {
      unused_handles_.push_back(handle);
      values_[handle.Value()] = T();
#ifndef NDEBUG
      engaged_[handle.Value()] = false;
#endif
    } else {
      unused_handles_.clear();
      values_.clear();
#ifndef NDEBUG
      engaged_.clear();
#endif
    }
  }

  // Returns object associated with `handle`. `handle` must not be empty or
  // released.
  T& operator[](Handle handle) {
#ifndef NDEBUG
    CHECK(engaged_.size() > handle.Value() && engaged_[handle.Value()])
        << "Attempting to use released handle! " << handle.Value();
#endif
    return values_[handle.Value()];
  }

  // Returns object associated with `handle`. `handle` must not be empty or
  // released.
  const T& operator[](Handle handle) const {
#ifndef NDEBUG
    CHECK(engaged_.size() > handle.Value() && engaged_[handle.Value()])
        << "Attempting to use released handle! " << handle.Value();
#endif
    return values_[handle.Value()];
  }

 private:
  std::vector<T> values_;
  std::vector<Handle> unused_handles_;
#ifndef NDEBUG
  std::vector<bool> engaged_;
#endif
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_GRID_WORLD_COLLECTIONS_OBJECT_POOL_H_
