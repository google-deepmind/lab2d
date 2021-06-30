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

// Functions for procedural generation of Pushbox levels.

#ifndef DMLAB2D_LIB_SYSTEM_GENERATORS_PUSHBOX_PUSHBOX_H_
#define DMLAB2D_LIB_SYSTEM_GENERATORS_PUSHBOX_PUSHBOX_H_

#include <cstdint>
#include <string>
#include <utility>

#include "dmlab2d/lib/system/generators/pushbox/random_room_generator.h"
#include "dmlab2d/lib/system/generators/pushbox/room.h"

namespace deepmind::lab2d::pushbox {

struct Settings {
  // Seed used for generating other seeds if unset. Must be provided.
  std::uint32_t seed;

  // The room dimensions. Note that the room first and last column and row in
  // the room are always walls.
  int width = 14;
  int height = 14;

  // Number of boxes to place in the room.
  int num_boxes = 4;

  // Number of generation steps for building the rooms. The higher this value
  // the more likely the room is to contain more floor tiles.
  int room_steps = 20;

  // Seed for the room shape generation. Unset uses one generated from seed.
  absl::optional<std::uint32_t> room_seed;

  // Seed for the box targets' locations. Unset uses one generated from seed.
  absl::optional<std::uint32_t> targets_seed;

  // Random seed for the order in which reverse actions are applied to the room
  // when searching for a valid starting position.  Unset uses one generated
  // from seed.
  absl::optional<std::uint32_t> actions_seed;
};

struct ResultOr {
  static ResultOr Success(std::string level) { return {std::move(level)}; }
  static ResultOr Error(std::string error) {
    return {std::string(), std::move(error)};
  }

  std::string level;
  std::string error;
};

// Generates and returns a Pushbox level (room, boxes and player in a valid
// starting position). Returns an error if a valid solution wasn't found with
// the passed in parameters.
// The room is generated according to the input parameters as follows:
//  - settings: See Settings struct.
//
// The algorithm generates a new room in the following steps.
//  - Generate a room shape.
//  - Place target locations (and boxes in top of them) as well as the player.
//  - Apply random inverse actions (moving the player and pulling from boxes)
//    assigning a score to each generated position roughly based on the
//    complexity of the box moves that lead to the position.
//  - Return the position that obtained the higher score.
//
// Returns [level, error] containing either the generated level or an error
// message of why the generation failed.
ResultOr GenerateLevel(const Settings& settings);

}  // namespace deepmind::lab2d::pushbox

#endif  // DMLAB2D_LIB_SYSTEM_GENERATORS_PUSHBOX_PUSHBOX_H_
