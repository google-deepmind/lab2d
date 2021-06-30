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

#include "dmlab2d/lib/system/generators/pushbox/room_candidate_generator.h"

#include <utility>

#include "dmlab2d/lib/support/logging.h"

namespace deepmind::lab2d::pushbox {

RoomCandidateGenerator::RoomCandidateGenerator(const Room& base_room)
    : width_(base_room.width()),
      height_(base_room.height()),
      last_visited_index_(std::numeric_limits<int>::min()),
      actions_{{{{{-1, 0}, true}, -1},
                {{{1, 0}, true}, 1},
                {{{0, -1}, true}, -width_},
                {{{0, 1}, true}, width_}}},
      layout_(base_room.width() * base_room.height(), last_visited_index_) {
  auto iter = layout_.begin();
  for (int j = 0; j < base_room.height(); ++j) {
    for (int i = 0; i < base_room.width(); ++i, ++iter) {
      if (base_room.IsWall({i, j})) {
        *iter = kWall;
      } else if (base_room.IsTarget({i, j})) {
        target_locations_.insert(j * width_ + i);
      }
    }
  }
}

void RoomCandidateGenerator::GenerateRoomCandidates(
    const Room& room, std::vector<Room>* candidates) {
  // Increment last visited index so the layout can be reused again without
  // cleaning up.
  ++last_visited_index_;
  CHECK_LT(last_visited_index_, kBox);

  SetBoxPositions(room.GetBoxes());

  // Flood fill room layout with last_visited_index so we can find all
  // accessible positions.
  FloodFillRoom(room.GetPlayerPosition());

  for (const auto& box : room.GetBoxes()) {
    int box_location = location(box.position());
    for (const auto& action : actions_) {
      if (layout_[box_location + action.offset] == last_visited_index_ &&
          layout_[box_location + 2 * action.offset] == last_visited_index_) {
        int player_location = box_location + action.offset;
        Room room_candidate = room;

        room_candidate.SetPlayerPosition(
            {player_location % width_, player_location / width_});
        room_candidate.ApplyAction(action.player_action);

        candidates->emplace_back(std::move(room_candidate));
      }
    }
  }

  // Clean box positions in the room layout so it can be reused for other
  // rooms.
  ClearBoxPositions(room.GetBoxes());
}

void RoomCandidateGenerator::MovePlayerToRandomAccessiblePosition(
    std::mt19937_64* rng, Room* room) {
  // Increment last visited index so the layout can be reused again without
  // cleaning up.
  ++last_visited_index_;
  CHECK_LT(last_visited_index_, kBox);

  SetBoxPositions(room->GetBoxes());

  // Flood fill room layout with last_visited_index so we can find all
  // accessible positions.
  FloodFillRoom(room->GetPlayerPosition());

  math::Vector2d random_position = FindRandomAccessbilePosition(rng);
  room->SetPlayerPosition(random_position);

  // Clean box positions in the room layout so it can be reused for other
  // rooms.
  ClearBoxPositions(room->GetBoxes());
}

int RoomCandidateGenerator::location(const math::Vector2d& position) const {
  return position.x + position.y * width_;
}

void RoomCandidateGenerator::SetBoxPositions(const std::vector<Box>& boxes) {
  for (const auto& box : boxes) {
    layout_[location(box.position())] = kBox;
  }
}

void RoomCandidateGenerator::ClearBoxPositions(const std::vector<Box>& boxes) {
  for (const auto& box : boxes) {
    // Mark all box locations as not visited to another room boxes could be
    // put into the layout.
    layout_[location(box.position())] = last_visited_index_;
  }
}

void RoomCandidateGenerator::FloodFillRoom(
    const math::Vector2d& player_position) {
  flood_fill_candidates_.clear();
  next_flood_fill_candidates_.clear();

  layout_[location(player_position)] = last_visited_index_;
  flood_fill_candidates_.push_back(location(player_position));

  while (!flood_fill_candidates_.empty()) {
    for (int location : flood_fill_candidates_) {
      for (const auto& action : actions_) {
        int new_location = location + action.offset;
        if (layout_[new_location] < last_visited_index_) {
          layout_[new_location] = last_visited_index_;
          next_flood_fill_candidates_.push_back(new_location);
        }
      }
    }
    flood_fill_candidates_.clear();
    flood_fill_candidates_.swap(next_flood_fill_candidates_);
  }
}

math::Vector2d RoomCandidateGenerator::FindRandomAccessbilePosition(
    std::mt19937_64* rng) const {
  std::vector<int> accessible_locations;
  accessible_locations.reserve(width_ * height_);

  int location_inx = 0;
  for (int cell : layout_) {
    if (cell == last_visited_index_ &&
        target_locations_.find(location_inx) == target_locations_.end()) {
      accessible_locations.push_back(location_inx);
    }
    ++location_inx;
  }

  CHECK(!accessible_locations.empty());
  int location = accessible_locations[std::uniform_int_distribution<int>(
      0, accessible_locations.size() - 1)(*rng)];
  return {location % width_, location / width_};
}

}  // namespace deepmind::lab2d::pushbox
