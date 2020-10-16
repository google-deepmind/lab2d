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

#include "dmlab2d/system/generators/pushbox/pushbox.h"

#include <algorithm>
#include <cstdint>
#include <random>
#include <stack>
#include <string>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/container/flat_hash_set.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "dmlab2d/system/generators/pushbox/constants.h"
#include "dmlab2d/system/generators/pushbox/random_room_generator.h"
#include "dmlab2d/system/generators/pushbox/room.h"
#include "dmlab2d/system/generators/pushbox/room_candidate_generator.h"

namespace deepmind::lab2d::pushbox {
namespace {

// Generates a starting point for a Pushbox level by reverse application of
// actions from a base position. Returns an error if it wasn't able to get
// to a room position with score > 0 or the room configuration with highest
// obtained score otherwise.
// The received random number generator (rng) is used for randomly ordering the
// set of applicable actions.
// The max_room_configs parameter indicates how many possible configurations
// from the base positions we may try.
// The max_action_depth parameter indicates the maximum length of the sequence
// of actions applied when exploring new room configurations (i.e. the search
// depth).
absl::optional<Room> ReverseSolveRoom(const Room& base_room,
                                      std::mt19937_64* rng,
                                      int max_room_configs,
                                      int max_action_depth) {
  // Set of rooms that we have already visited.
  absl::flat_hash_set<std::uint64_t> visited_rooms(
      generator::kVisitedRoomsBucketCount);

  // Keep track of the room with the highest score so far.
  double highest_score = 0;
  Room highest_score_room = base_room;

  // A list of pending rooms on which we haven't applied actions yet.
  std::stack<Room> pending_rooms;
  pending_rooms.push(base_room);

  RoomCandidateGenerator generator(base_room);

  std::vector<Room> room_candidates;
  while (!pending_rooms.empty() && visited_rooms.size() < max_room_configs) {
    // Get the next pending room to apply actions to.
    auto current_room = std::move(pending_rooms.top());
    pending_rooms.pop();

    room_candidates.clear();
    generator.GenerateRoomCandidates(current_room, &room_candidates);
    std::shuffle(room_candidates.begin(), room_candidates.end(), *rng);

    for (auto& new_room : room_candidates) {
      // Avoid going beyond the limit of applied actions.
      if (new_room.num_actions() >= max_action_depth) continue;

      // If we havent explore this room configuration yet add it to the
      // pending and visited room lists.
      auto hash = new_room.hash();
      auto iter_insert = visited_rooms.insert(hash);
      if (iter_insert.second) {
        new_room.ComputeScore();
        auto score = new_room.room_score();
        if (score > highest_score) {
          highest_score = score;
          highest_score_room = new_room;
        }
        pending_rooms.emplace(std::move(new_room));
      }
    }
  }

  generator.MovePlayerToRandomAccessiblePosition(rng, &highest_score_room);

  if (highest_score_room.room_score() == 0) return absl::nullopt;

  return highest_score_room;
}

}  // namespace

ResultOr GenerateLevel(const Settings& settings) {
  if (settings.height > generator::kMaxRoomSize)
    return ResultOr::Error(
        absl::StrFormat("Specified (height=%d) > (kMaxRoomSize=%d) ",
                        settings.height, generator::kMaxRoomSize));
  if (settings.width > generator::kMaxRoomSize)
    return ResultOr::Error(
        absl::StrFormat("Specified (width=%d) > (kMaxRoomSize=%d) ",
                        settings.width, generator::kMaxRoomSize));
  if (settings.num_boxes < generator::kMinBoxes)
    return ResultOr::Error(
        absl::StrFormat("Specified (numBoxes=%d) < (kMinBoxes=%d) ",
                        settings.num_boxes, generator::kMinBoxes));
  if (settings.room_steps < generator::kMinSteps)
    return ResultOr::Error(
        absl::StrFormat("Specified (roomSteps=%d) < (kMinSteps=%d) ",
                        settings.room_steps, generator::kMinSteps));

  std::mt19937_64 rng(settings.seed);
  std::uniform_int_distribution<std::uint32_t> seed_distribution;
  std::uint32_t room_seed = settings.room_seed.value_or(seed_distribution(rng));
  std::uint32_t targets_seed =
      settings.targets_seed.value_or(seed_distribution(rng));
  std::uint32_t actions_seed =
      settings.actions_seed.value_or(seed_distribution(rng));
  // Initialize the room generator.
  RandomRoomGenerator room_generator(
      settings.width, settings.height, settings.num_boxes, settings.room_steps,
      generator::kDirectionChangeRatio, room_seed, targets_seed);

  for (int topology_retries = 0;
       topology_retries < generator::kMaxRoomTopologies; ++topology_retries) {
    // Generate a new room topology (i.e. walls and floor).
    auto topology_or = room_generator.GenerateRoomTopology();
    if (!topology_or) {
      return ResultOr::Error("Max iterations when gernerating floor topology");
    }

    std::mt19937_64 mt_rng(actions_seed);
    for (int pos_retries = 0; pos_retries < generator::kMaxPositions;
         ++pos_retries) {
      // Start with a new player and box locations before every retry.
      auto status_or = room_generator.UpdateBoxAndPlayerPositions(
          absl::MakeSpan(*topology_or));
      // If failed to position the boxes and player try a different room.
      if (!status_or) break;
      auto base_room = std::move(status_or.value());

      // Try to reverse-solve the room to get a starting position for the level.
      status_or = ReverseSolveRoom(base_room, &mt_rng,
                                   generator::kMaxRoomConfigurations,
                                   generator::kMaxAppliedActions);

      // Return the generated room configuration.
      if (status_or) return ResultOr::Success(status_or.value().ToString());
    }
  }

  return ResultOr::Error("Maximum room generation retries reached.");
}

}  // namespace deepmind::lab2d::pushbox
