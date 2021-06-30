// Copyright (C) 2020 The DMLab2D Authors.
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

#ifndef DMLAB2D_LIB_SYSTEM_GENERATORS_PUSHBOX_ROOM_CANDIDATE_GENERATOR_H_
#define DMLAB2D_LIB_SYSTEM_GENERATORS_PUSHBOX_ROOM_CANDIDATE_GENERATOR_H_

#include <array>
#include <limits>
#include <random>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "dmlab2d/lib/system/generators/pushbox/room.h"
#include "dmlab2d/lib/system/math/math2d.h"

namespace deepmind::lab2d::pushbox {

// Room candidate generator uses flood fill to find all boxes accessible by the
// player and creates room layout candidates for the next generation step by
// moving all accessible boxes in all possible directions.
class RoomCandidateGenerator {
  // A layout cell value indicating walls.
  static constexpr int kWall = std::numeric_limits<int>::max();

  // A layout cell value indicating boxes.
  static constexpr int kBox = std::numeric_limits<int>::max() - 1;

  // A structure that associates a player action with corresponding location
  // offset.
  struct ActionOffset {
    Action player_action;
    int offset;
  };

 public:
  explicit RoomCandidateGenerator(const Room& base_room);

  // Generates a vector of room layout candidates for the next step. The room
  // passed to this method is supposed to have the same walls layout as
  // base_room used during construction.
  void GenerateRoomCandidates(const Room& room, std::vector<Room>* candidates);

  // Moves the player into a random position that can be accessed by the current
  // one without moving any boxes.
  void MovePlayerToRandomAccessiblePosition(std::mt19937_64* rng, Room* room);

 private:
  int width_;
  int height_;

  // The index used to mark layout cells as visited.
  int last_visited_index_;

  // An array of possible actions and corresponding layout offsets.
  std::array<ActionOffset, 4> actions_;

  // Room layout used by flood fill algorithm. The same vector is reused by all
  // iterations to avoid memory allocation and walls layout copy. All cells with
  // values lower than last_visited_index_ are considered not visited.
  std::vector<int> layout_;

  // Accessible locations for the current flood fill iteration.
  std::vector<int> flood_fill_candidates_;

  // Accessible locations for the next flood fill iteration. This vector is
  // swapped with flood_fill_candidates_ after it's exhausted.
  std::vector<int> next_flood_fill_candidates_;

  // A set of all target locations.
  absl::flat_hash_set<int> target_locations_;

  int location(const math::Vector2d& position) const;

  void SetBoxPositions(const std::vector<Box>& boxes);

  void ClearBoxPositions(const std::vector<Box>& boxes);

  void FloodFillRoom(const math::Vector2d& player_position);

  math::Vector2d FindRandomAccessbilePosition(std::mt19937_64* rng) const;
};

}  // namespace deepmind::lab2d::pushbox

#endif  // DMLAB2D_LIB_SYSTEM_GENERATORS_PUSHBOX_ROOM_CANDIDATE_GENERATOR_H_
