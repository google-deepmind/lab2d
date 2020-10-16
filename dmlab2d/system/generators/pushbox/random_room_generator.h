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

// The RandomRoomGenerator implements procedural generation of Pushbox levels.

#ifndef DMLAB2D_SYSTEM_GENERATORS_PUSHBOX_RANDOM_ROOM_GENERATOR_H_
#define DMLAB2D_SYSTEM_GENERATORS_PUSHBOX_RANDOM_ROOM_GENERATOR_H_

#include <random>
#include <vector>

#include "absl/status/status.h"
#include "absl/types/optional.h"
#include "dmlab2d/system/generators/pushbox/room.h"
#include "dmlab2d/system/math/math2d.h"

namespace deepmind::lab2d::pushbox {

// The RandomRoomGenerator receives a number of generator parameters and
// provides tools for generating as many procedural Pushbox rooms as required.
// Each room is generated with the passed in dimensions and number of targets.
// If required, the same room topology (walls and floor tiles) can be kept and
// regenerate the targets and player positions only.
// The generated rooms represent a final Pushbox position, that is the boxes are
// placed on top of their corresponding targets.
class RandomRoomGenerator {
 public:
  // Constructs generator with the following parameters:
  // - width, height: Room dimensions.
  // - num_targets:   Number of targets (and boxes) in the room.
  // - gen_steps:     Number of steps employed in the room generation. Starting
  //                  from a random position in the room, for each step we will
  //                  move in a random position and apply a 'floor pattern',
  //                  which means to mark as floor the tiles in certain position
  //                  around the current one.
  // - p_change_direction:
  //                  Probability of changing direction for the next step.
  // - room_seed:     Random seed for the generation of the room shape.
  // - positions_seed:Random seed for the placement of the targets/player.
  //
  // The passed in parameters will be kept fixed in this generator for
  // subsequent room generation requests.
  RandomRoomGenerator(int width, int height, int num_targets, int gen_steps,
                      float p_change_direction, std::uint32_t room_seed,
                      std::uint32_t positions_seed);

  // Generates a random room (topology only) without player or targets placed.
  // Returns absl::nullopt if the room generation couldn't be completed (e.g. if
  // we exceed the maximum number of step retries).
  absl::optional<std::vector<TileType>> GenerateRoomTopology();

  // Uses a preexisting room layout and regenerates the player and boxes
  // positions, returning the resulting room. Returns absl::nullopt if the room
  // couldn't be generated: for example if it wasn't possible to find suitable
  // positions for the targets and player with the existing room topology. Note
  // that if a room topology wasn't previously generated this method will return
  // absl::nullopt as there will not be suitable positions to place the boxes
  // and player.
  absl::optional<Room> UpdateBoxAndPlayerPositions(
      absl::Span<TileType> topology);

 private:
  // Check that the given position is valid: it's inside the room and not on top
  // of the wall margins left on the room sides.
  bool IsValidPosition(const math::Vector2d& position);

  // Returns true if a box in the given position could be, in principle, pulled
  // in the indicated direction. Implies that there are at least two floor tiles
  // towards the indicated direction, however the possibility of other boxes
  // blocking the pull is ignored.
  bool CanPull(const Room* room, const math::Vector2d& position,
               const math::Vector2d& direction) const;

  // Return true if the given position is a valid placement for a target: it is
  // an empty floor tile and there is at least one direction in which we can
  // pull a box if placed in the given position.
  bool IsValidTargetPosition(const Room* room,
                             const math::Vector2d& position) const;

  // Returns a direction randomly picked (using the passed in random number
  // generator) from the set of available ones.
  const math::Vector2d& RandomDirection(std::mt19937_64* rng);

  // Returns a random position in the room (generated using the passed in random
  // number generator). The position respects the stablished wall margins at the
  // room sides.
  math::Vector2d RandomPosition(int margin, std::mt19937_64* rng);

  // Applies a floor pattern, randomly picked from the set of available ones,
  // to the specified position.
  void AddRandomFloorPattern(const math::Vector2d& position,
                             absl::Span<TileType> topology);

  // Adds the provided floor pattern in the given position. The pattern
  // indicates a set of displacement with respect to the original position that
  // have to be marked as floor.
  void AddFloorPattern(const math::Vector2d& position,
                       absl::Span<const math::Vector2d> pattern,
                       absl::Span<TileType> topology);

  // Adds as many random targets in the room as specified in the interanl
  // parameters.
  // This methods is expected to be called with a room without any entities
  // placed beforehand.
  // Returns whether it was able to place as many targets as required
  // within the predefined amount of retries.
  bool AddRandomTargets(Room* room, absl::Span<TileType> topology);

  // Randomly adds a player position. This methods is expected to be called
  // after the targets/boxes have been placed with the AddRandomTargets method.
  // Returns whether it was able to find a suitable position for the player
  // within a predefined amount of retries.
  bool AddPlayerRandomPosition(Room* room);

  // Dimensions of the generated room.
  const int width_, height_;
  // Number of targets (and boxes) in the room.
  const int num_targets_;

  // Number of generation steps.
  const int gen_steps_;

  // Probability of changing direction on the next room generation step.
  const float p_change_direction_;

  // Internal random number generators for the room shape and box/player
  // positions.
  std::mt19937_64 room_rng_, positions_rng_;

  // Zobrist bitstrings https://en.wikipedia.org/wiki/Zobrist_hashing
  const std::vector<std::uint64_t> zobrist_bitstrings_;
};

}  // namespace deepmind::lab2d::pushbox

#endif  // DMLAB2D_SYSTEM_GENERATORS_PUSHBOX_RANDOM_ROOM_GENERATOR_H_
