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

#ifndef DMLAB2D_LIB_SYSTEM_GRID_WORLD_COLLECTIONS_SHUFFLED_MEMBERSHIP_H_
#define DMLAB2D_LIB_SYSTEM_GRID_WORLD_COLLECTIONS_SHUFFLED_MEMBERSHIP_H_

#include <algorithm>

#include "absl/types/span.h"
#include "dmlab2d/lib/system/grid_world/collections/fixed_handle_map.h"
#include "dmlab2d/lib/system/grid_world/collections/shuffled_set.h"
#include "dmlab2d/lib/util/visit_set_difference_and_intersection.h"

namespace deepmind::lab2d {

// Manages membership to multiple ShuffledSets.
template <typename SetHandle, typename Handle>
class ShuffledMembership
    : public FixedHandleMap<SetHandle, ShuffledSet<Handle>> {
 public:
  using FixedHandleMap<SetHandle, ShuffledSet<Handle>>::FixedHandleMap;
  void ChangeMembership(Handle handle, absl::Span<const SetHandle> source_sets,
                        absl::Span<const SetHandle> target_sets) {
    VisitSetDifferencesAndIntersection(
        source_sets.begin(), source_sets.end(), target_sets.begin(),
        target_sets.end(),
        // Sets `handle` is no longer members of.
        // Called for elements only in source_sets.
        [this, handle](SetHandle set_handle) {
          (*this)[set_handle].Erase(handle);
        },
        // Sets `handle` is now member of.
        // Called for elements only in target_sets.
        [this, handle](SetHandle set_handle) {
          (*this)[set_handle].Insert(handle);
        },
        // Called for elements in both sets.
        [](SetHandle set_handle) {});
  }
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_GRID_WORLD_COLLECTIONS_SHUFFLED_MEMBERSHIP_H_
