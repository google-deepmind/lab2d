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

#include "dmlab2d/lib/system/generators/pushbox/room.h"

#include <algorithm>
#include <iterator>
#include <memory>
#include <string>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/str_cat.h"
#include "dmlab2d/lib/system/generators/pushbox/constants.h"

namespace deepmind::lab2d::pushbox {

namespace {

// Returns a char to represent a room's tile based on its type and content.
char GetTileChar(const TileType& tile, bool contains_box,
                 bool contains_player) {
  if (contains_player) {
    return room::kPlayerChar;
  } else if (contains_box) {
    return tile == TileType::kTarget ? room::kBoxTagetChar : room::kBoxChar;
  } else {
    // The tile is empty, so it can be a wall '*', floor ' ' or target 'X'.
    switch (tile) {
      case TileType::kWall:
        return room::kWallChar;
        break;
      case TileType::kFloor:
        return room::kFloorChar;
        break;
      case TileType::kTarget:
        return room::kTagetChar;
        break;
    }
  }
  LOG(FATAL) << "Unexpected tile type.";
}

}  // namespace

Room::Room(int width, int height, absl::Span<const TileType> topology,
           absl::Span<const std::uint64_t> zobrist_bitstrings)
    : width_(width),
      height_(height),
      cell_count_(width * height),
      topology_(topology),
      zobrist_bitstrings_(zobrist_bitstrings),
      zobrist_hash_(0),
      player_({}),
      num_actions_(0),
      last_box_index_(-1),
      moved_box_changes_(0),
      room_score_(0) {
  DCHECK_EQ(topology_.size(), width_ * height_);
  DCHECK_EQ(zobrist_bitstrings.size(), width_ * height_ * 2);
  zobrist_hash_ ^= zobrist_bitstrings_[0];
}

std::string Room::ToString() const {
  std::string room_string;
  room_string.reserve(topology_.size() + height_);
  for (int tile_idx = 0; tile_idx < topology_.size(); tile_idx++) {
    if (tile_idx > 0 && tile_idx % width_ == 0)
      absl::StrAppend(&room_string, "\n");
    math::Vector2d position{tile_idx % width_, tile_idx / width_};
    bool contains_player = ContainsPlayer(position);
    bool contains_box = ContainsBox(position);
    room_string.push_back(
        GetTileChar(topology_[tile_idx], contains_box, contains_player));
  }
  return room_string;
}

void Room::ZobristAddOrRemovePiece(const math::Vector2d& position,
                                   EntityLayer layer) {
  int location = position.x + position.y * width_;
  zobrist_hash_ ^=
      zobrist_bitstrings_[location + static_cast<int>(layer) * cell_count_];
}

TileType Room::GetTileType(const math::Vector2d& position) const {
  return topology_[position.x + position.y * width_];
}

bool Room::IsWall(const math::Vector2d& position) const {
  return GetTileType(position) == TileType::kWall;
}

bool Room::IsFloor(const math::Vector2d& position) const {
  return GetTileType(position) == TileType::kFloor;
}

bool Room::IsTarget(const math::Vector2d& position) const {
  return GetTileType(position) == TileType::kTarget;
}

bool Room::IsEmpty(const math::Vector2d& position) const {
  return !(ContainsPlayer(position) || ContainsBox(position));
}

bool Room::ContainsPlayer(const math::Vector2d& position) const {
  return player_.position() == position;
}

bool Room::ContainsBox(const math::Vector2d& position) const {
  return std::any_of(boxes_.begin(), boxes_.end(), [position](const Box& box) {
    return box.position() == position;
  });
}

void Room::SetPlayerPosition(const math::Vector2d& position) {
  DCHECK(player_.position() == position || IsEmpty(position));
  ZobristAddOrRemovePiece(player_.position(), EntityLayer::kPlayer);
  ZobristAddOrRemovePiece(position, EntityLayer::kPlayer);
  player_.set_position(position);
}

void Room::AddBox(const math::Vector2d& position) {
  DCHECK(IsEmpty(position));

  ZobristAddOrRemovePiece(position, EntityLayer::kBox);

  boxes_.push_back(Box(position));
}

void Room::ApplyAction(const Action& action) {
  // Move the player to its new position.
  const auto initial_player_position = player_.position();
  ApplyPlayerAction(initial_player_position, action);

  // If pulling, move the box to the original player position.
  if (action.pull) {
    const auto& box_position = initial_player_position - action.direction;
    MoveBox(box_position, action.direction);
  }

  ++num_actions_;
}

float Room::ComputeScore() {
  room_score_ = 0;
  // Avoid generating a room with the player or a box lying on a target.
  if (PlayerOnTarget() || BoxOnTarget()) {
    return room_score_;
  }

  float total_displacement = 0;
  for (const auto& box : boxes_) {
    total_displacement += box.Displacement();
  }
  room_score_ = moved_box_changes_ * total_displacement;
  return room_score_;
}

// Returns whether there is any box on top of any target.
bool Room::BoxOnTarget() {
  for (const auto& box : boxes_)
    if (IsTarget(box.position())) return true;

  return false;
}

void Room::ApplyPlayerAction(const math::Vector2d& origin,
                             const Action& action) {
  const auto& target = origin + action.direction;
  DCHECK(IsEmpty(target) && ContainsPlayer(origin));

  // Change the position of the player.
  SetPlayerPosition(target);
}

void Room::MoveBox(const math::Vector2d& origin,
                   const math::Vector2d& direction) {
  const auto& target = origin + direction;
  DCHECK(IsEmpty(target) && ContainsBox(origin));

  auto it = std::find_if(
      boxes_.begin(), boxes_.end(),
      [origin](const Box& box) { return box.position() == origin; });

  CHECK(it != boxes_.end());

  it->set_position(target);
  it->AddMove();
  int box_idx = std::distance(boxes_.begin(), it);

  if (last_box_index_ != box_idx) {
    last_box_index_ = box_idx;
    ++moved_box_changes_;
  }

  ZobristAddOrRemovePiece(origin, EntityLayer::kBox);
  ZobristAddOrRemovePiece(target, EntityLayer::kBox);
}

}  // namespace deepmind::lab2d::pushbox
