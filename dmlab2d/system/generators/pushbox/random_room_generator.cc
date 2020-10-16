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

#include "dmlab2d/system/generators/pushbox/random_room_generator.h"

#include <algorithm>
#include <cstdint>
#include <iterator>
#include <random>
#include <vector>

#include "absl/random/distributions.h"
#include "absl/random/random.h"
#include "absl/types/span.h"
#include "dmlab2d/system/generators/pushbox/constants.h"
#include "dmlab2d/system/math/math2d.h"

namespace deepmind::lab2d::pushbox {
namespace {

constexpr math::Vector2d kZ = math::Vector2d::Zero();
constexpr math::Vector2d kN = math::Vector2d::North();
constexpr math::Vector2d kE = math::Vector2d::East();
constexpr math::Vector2d kS = math::Vector2d::South();
constexpr math::Vector2d kW = math::Vector2d::West();

// Provide the patterns for marking room tiles as 'floor'. Each row contains a
// possible pattern composed by an arbitrary number of displacement vectors.
// Each displacement vector is applied to an original position to obtain a list
// of resulting positions that will be marked as 'floor'. All the following
// patterns include the displacement {0, 0} which indicates that the original
// position will always marked as floor.
constexpr math::Vector2d kWestEast[] = {kZ, kW, kE};
constexpr math::Vector2d kNorthSouth[] = {kZ, kN, kS};
constexpr math::Vector2d kEastSouth[] = {kZ, kE, kS};
constexpr math::Vector2d kWestSouth[] = {kZ, kW, kS};
constexpr math::Vector2d kWestAndSouth[] = {kZ, kW, kS, kS + kW};
constexpr absl::Span<const math::Vector2d> kFloorPattern[] = {
    kWestEast, kNorthSouth, kEastSouth, kWestSouth, kWestAndSouth,
};

// Directions that we can take when generating the room. For each step one of
// the directions will be applied to the current position so we will randomly
// move within the room.
constexpr std::array<math::Vector2d, 4> kDirections = {kE, kW, kS, kN};

std::vector<std::uint64_t> GenerateZobristBitstrings(int width, int height,
                                                     int layers) {
  std::mt19937_64 rnd(room::kZobristSeed);

  std::vector<std::uint64_t> bitstrings;

  int size = width * height * layers;
  bitstrings.reserve(size);
  std::generate_n(std::back_inserter(bitstrings), size, [&rnd]() {
    return std::uniform_int_distribution<std::uint64_t>()(rnd);
  });
  return bitstrings;
}

}  // namespace

RandomRoomGenerator::RandomRoomGenerator(int width, int height, int num_targets,
                                         int gen_steps,
                                         float p_change_direction,
                                         std::uint32_t room_seed,
                                         std::uint32_t positions_seed)
    : width_(width),
      height_(height),
      num_targets_(num_targets),
      gen_steps_(gen_steps),
      p_change_direction_(p_change_direction),
      room_rng_(room_seed),
      positions_rng_(positions_seed),
      zobrist_bitstrings_(GenerateZobristBitstrings(width, height, 2)) {}

absl::optional<Room> RandomRoomGenerator::UpdateBoxAndPlayerPositions(
    absl::Span<TileType> topology) {
  std::replace(topology.begin(), topology.end(), TileType::kTarget,
               TileType::kFloor);

  Room room(width_, height_, topology, zobrist_bitstrings_);
  if (!AddRandomTargets(&room, topology)) {
    return absl::nullopt;
  }
  if (!AddPlayerRandomPosition(&room)) {
    return absl::nullopt;
  }
  return room;
}

absl::optional<std::vector<TileType>>
RandomRoomGenerator::GenerateRoomTopology() {
  // Start a new room from scratch.
  std::vector<TileType> topology(width_ * height_, TileType::kWall);

  // Start in a random position within the room.
  auto pos = RandomPosition(room::kDefaultWallMargin, &room_rng_);
  auto direction = RandomDirection(&room_rng_);
  int applied_steps = 0;
  int retried_steps = 0;
  // Try to apply as many generation steps as requested.
  while (applied_steps < gen_steps_) {
    // Randomly change direction.
    if (std::bernoulli_distribution(p_change_direction_)(room_rng_))
      direction = RandomDirection(&room_rng_);

    // Try to apply a floor pattern to the new position.
    if (IsValidPosition(pos + direction)) {
      pos = pos + direction;
      AddRandomFloorPattern(pos, absl::MakeSpan(topology));
      ++applied_steps;
    }

    if (++retried_steps >= room::kMaxGenerationStepRetries)
      return absl::nullopt;
  }

  return topology;
}

bool RandomRoomGenerator::IsValidPosition(const math::Vector2d& position) {
  return (position.x >= room::kDefaultWallMargin &&
          position.x < width_ - room::kDefaultWallMargin &&
          position.y >= room::kDefaultWallMargin &&
          position.y < height_ - room::kDefaultWallMargin);
}

bool RandomRoomGenerator::CanPull(const Room* room,
                                  const math::Vector2d& position,
                                  const math::Vector2d& direction) const {
  return room->IsFloor(position + direction) &&
         room->IsFloor(position + 2 * direction);
}

bool RandomRoomGenerator::IsValidTargetPosition(
    const Room* room, const math::Vector2d& position) const {
  if (!room->IsFloor(position) || !room->IsEmpty(position)) return false;

  return std::any_of(kDirections.begin(), kDirections.end(),
                     [this, room, position](math::Vector2d direction) {
                       return CanPull(room, position, direction);
                     });
}

const math::Vector2d& RandomRoomGenerator::RandomDirection(
    std::mt19937_64* rng) {
  std::uniform_int_distribution<int> dist(0, kDirections.size() - 1);
  return kDirections[dist(*rng)];
}

math::Vector2d RandomRoomGenerator::RandomPosition(int margin,
                                                   std::mt19937_64* rng) {
  return {
      std::uniform_int_distribution<int>(margin, width_ - margin - 1)(*rng),
      std::uniform_int_distribution<int>(margin, height_ - margin - 1)(*rng)};
}

void RandomRoomGenerator::AddRandomFloorPattern(const math::Vector2d& position,
                                                absl::Span<TileType> topology) {
  constexpr auto floor_pattern = absl::MakeConstSpan(kFloorPattern);
  std::uniform_int_distribution<int> dist(0, floor_pattern.size() - 1);
  AddFloorPattern(position, floor_pattern[dist(room_rng_)], topology);
}

void RandomRoomGenerator::AddFloorPattern(
    const math::Vector2d& position, absl::Span<const math::Vector2d> pattern,
    absl::Span<TileType> topology) {
  for (const auto& pattern_element : pattern) {
    const math::Vector2d target_pos = position + pattern_element;
    if (IsValidPosition(target_pos)) {
      topology[target_pos.x + target_pos.y * width_] = TileType::kFloor;
    }
  }
}

bool RandomRoomGenerator::AddRandomTargets(Room* room,
                                           absl::Span<TileType> topology) {
  int added_targets = 0;
  int retries = 0;
  while (added_targets < num_targets_) {
    const math::Vector2d pos =
        RandomPosition(room::kDefaultWallMargin, &positions_rng_);
    if (IsValidTargetPosition(room, pos)) {
      room->AddBox(pos);
      topology[pos.x + pos.y * width_] = TileType::kTarget;
      ++added_targets;
    }
    if (++retries >= room::kMaxTargetPlacementRetries) return false;
  }

  return true;
}

bool RandomRoomGenerator::AddPlayerRandomPosition(Room* room) {
  for (int retries = 0; retries < room::kMaxPlayerPlacementRetries; ++retries) {
    const math::Vector2d pos =
        RandomPosition(room::kDefaultWallMargin, &positions_rng_);
    if (room->IsFloor(pos) && room->IsEmpty(pos)) {
      room->SetPlayerPosition(pos);
      return true;
    }
  }
  return false;
}

}  // namespace deepmind::lab2d::pushbox
