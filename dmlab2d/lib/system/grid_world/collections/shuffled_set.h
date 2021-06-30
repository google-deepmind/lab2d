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

#ifndef DMLAB2D_LIB_SYSTEM_GRID_WORLD_COLLECTIONS_SHUFFLED_SET_H_
#define DMLAB2D_LIB_SYSTEM_GRID_WORLD_COLLECTIONS_SHUFFLED_SET_H_

#include <algorithm>
#include <iterator>
#include <random>
#include <utility>
#include <vector>

#include "absl/types/span.h"
#include "dmlab2d/lib/support/logging.h"

namespace deepmind::lab2d {

// A set of elements that are always accessed in a random order.
template <typename T>
class ShuffledSet {
 public:
  // Returns whether the set is empty.
  bool IsEmpty() const { return data_.empty(); }

  // Returns the number of elements in the set.
  std::size_t NumElements() const { return data_.size(); }

  // Inserts `element` into the set. It must not already be in set. Invalidates
  // the elements returned by `ShuffledElements*`.
  void Insert(const T& element) { data_.push_back(element); }

  // Erases `element` from the set. It must already be in set. Invalidates
  // the elements returned by `ShuffledElements*`.
  void Erase(const T& element) {
    data_.erase(std::remove(data_.begin(), data_.end(), element), data_.end());
  }

  // Shuffles the elements in the set and returns a reference to them.
  // Calls to non-const members will invalidate the returned reference.
  absl::Span<const T> ShuffledElements(std::mt19937_64* rng) {
    std::shuffle(data_.begin(), data_.end(), *rng);
    return absl::MakeConstSpan(data_);
  }

  // Returns a random element. The set must not be empty.
  T RandomElement(std::mt19937_64* random) const {
    CHECK(!IsEmpty()) << "Must not sample from empty set!";
    const auto n = NumElements() - 1;
    const std::size_t index =
        std::uniform_int_distribution<std::size_t>(0, n)(*random);
    return data_[index];
  }

  // Shuffles the elements in the set and returns a reference to them. Calls to
  // non-const members will invalidate the returned reference. Will return at
  // most max_count elements. Calls to non-const members will invalidate the
  // returned reference.
  absl::Span<const T> ShuffledElementsWithMaxCount(std::mt19937_64* rng,
                                                   std::size_t max_count) {
    if (max_count <= 0) {
      return {};
    }
    if (max_count >= data_.size()) {
      return ShuffledElements(rng);
    }
    // Fisher-Yates shuffle for first max_count elements.
    auto first = data_.begin();
    for (std::size_t i = 0; i < max_count; ++i) {
      std::uniform_int_distribution<std::size_t> dist(0, data_.size() - i - 1);
      auto selected = first + dist(*rng);
      std::iter_swap(first, selected);
      ++first;
    }
    return absl::MakeConstSpan(data_.data(), max_count);
  }

  // Selects each element with a probability `probability`. Selected elements
  // are shuffled and returned by reference. Calls to non-const members will
  // invalidate the returned reference.
  absl::Span<const T> ShuffledElementsWithProbability(std::mt19937_64* rng,
                                                      double probability) {
    if (probability <= 0) {
      return {};
    } else if (probability < 1.0) {
      std::size_t max_count =
          std::binomial_distribution<>(data_.size(), probability)(*rng);
      return ShuffledElementsWithMaxCount(rng, max_count);
    } else {
      return ShuffledElements(rng);
    }
  }

  // Calls predicate on each item in a random order until a call to `predicate`
  // returns true or the sequence is finished. Returns the address of the found
  // element if predecate returns true otherwise returns nullptr.
  template <typename Pred>
  const T* ShuffledElementsFind(std::mt19937_64* rng, Pred predicate) {
    for (auto first = data_.begin(), last = data_.end(); first != last;
         ++first) {
      std::uniform_int_distribution<std::size_t> dist(
          0, std::distance(first, last) - 1);
      auto selected = first + dist(*rng);
      if (predicate(*selected)) {
        return &(*selected);
      }
      std::iter_swap(first, selected);
    }
    return nullptr;
  }

 private:
  std::vector<T> data_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_GRID_WORLD_COLLECTIONS_SHUFFLED_SET_H_
