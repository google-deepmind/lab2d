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

#ifndef DMLAB2D_SYSTEM_GRID_WORLD_COLLECTIONS_FIXED_HANDLE_MAP_H_
#define DMLAB2D_SYSTEM_GRID_WORLD_COLLECTIONS_FIXED_HANDLE_MAP_H_

#include <utility>
#include <vector>

namespace deepmind::lab2d {

// Fixed sized map of a handle of type `Handle` to value of type `T`.
template <typename Handle, typename T>
class FixedHandleMap : private std::vector<T> {
 public:
  explicit FixedHandleMap(std::size_t num_elements)
      : std::vector<T>(num_elements) {}
  explicit FixedHandleMap(std::vector<T> values)
      : std::vector<T>(std::move(values)) {}

  const T& operator[](Handle handle) const {
    return std::vector<T>::operator[](handle.Value());
  }
  T& operator[](Handle handle) {
    return std::vector<T>::operator[](handle.Value());
  }

  // Support iteration.
  using std::vector<T>::size;
  using std::vector<T>::empty;
  using std::vector<T>::begin;
  using std::vector<T>::end;
  using std::vector<T>::cbegin;
  using std::vector<T>::cend;
  using std::vector<T>::rbegin;
  using std::vector<T>::rend;
  using std::vector<T>::crbegin;
  using std::vector<T>::crend;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_SYSTEM_GRID_WORLD_COLLECTIONS_FIXED_HANDLE_MAP_H_
