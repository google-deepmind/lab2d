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

#ifndef DMLAB2D_SYSTEM_GRID_WORLD_COLLECTIONS_HANDLE_NAMES_H_
#define DMLAB2D_SYSTEM_GRID_WORLD_COLLECTIONS_HANDLE_NAMES_H_

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <string>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"

namespace deepmind::lab2d {

// A bidirectional map of handles and names.
template <typename Handle>
class HandleNames {
 public:
  HandleNames(std::vector<std::string> names) : names_(std::move(names)) {
    reverse_lookup_.reserve(names_.size());
    for (std::size_t i = 0; i < names_.size(); ++i) {
      reverse_lookup_[names_[i]] = Handle(i);
    }
  }

  // Internal references are stored in reverse_lookup_.
  HandleNames(const HandleNames&) = delete;

  // Returns a sorted and unique set of handles associated with the list of
  // names.
  std::vector<Handle> ToHandles(absl::Span<const std::string> names) const {
    std::vector<Handle> result;
    result.reserve(names.size());
    for (const auto& name : names) {
      Handle handle = ToHandle(name);
      if (!handle.IsEmpty()) {
        result.push_back(handle);
      }
    }
    std::sort(result.begin(), result.end());
    result.erase(std::unique(result.begin(), result.end()), result.end());
    return result;
  }

  // Returns handle associated with `name` if `name` exists otherwise the empty
  // handle is returned.
  Handle ToHandle(absl::string_view name) const {
    auto it = reverse_lookup_.find(name);
    return (it != reverse_lookup_.end()) ? it->second : Handle();
  }

  // Returns name associated with `handle`. `handle` must not be empty.
  const std::string& ToName(Handle handle) const {
    return names_[handle.Value()];
  }

  std::size_t NumElements() const { return names_.size(); }
  const std::vector<std::string>& Names() const { return names_; }

  // Iterator for treating HandleNames as a collection.
  class HandleNamesIterator {
   public:
    friend class HandleNames;

    std::pair<Handle, const std::string&> operator*() const {
      return {Handle(index_), *vec_iter_};
    }

    bool operator!=(HandleNamesIterator rhs) const {
      return vec_iter_ != rhs.vec_iter_;
    }

    HandleNamesIterator& operator++() {
      ++vec_iter_;
      ++index_;
      return *this;
    }

   private:
    explicit HandleNamesIterator(
        std::vector<std::string>::const_iterator vec_iter, int index)
        : vec_iter_(vec_iter), index_(index) {}
    std::vector<std::string>::const_iterator vec_iter_;
    int index_;
  };

  HandleNamesIterator begin() const {
    return HandleNamesIterator(names_.cbegin(), 0);
  }
  HandleNamesIterator end() const {
    return HandleNamesIterator(names_.cend(), names_.size());
  }

 private:
  absl::flat_hash_map<absl::string_view, Handle> reverse_lookup_;
  // `names_` must remain stable to prevent reverse_lookup_ keys from being
  // corrupted.
  const std::vector<std::string> names_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_SYSTEM_GRID_WORLD_COLLECTIONS_HANDLE_NAMES_H_
