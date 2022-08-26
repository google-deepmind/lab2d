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

#include "dmlab2d/lib/system/grid_world/grid.h"

#include <algorithm>
#include <cstddef>
#include <iterator>
#include <random>
#include <utility>
#include <vector>

#include "absl/log/log.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "dmlab2d/lib/system/grid_world/collections/shuffled_membership.h"
#include "dmlab2d/lib/system/grid_world/grid_shape.h"
#include "dmlab2d/lib/system/grid_world/handles.h"
#include "dmlab2d/lib/system/math/math2d.h"
#include "dmlab2d/lib/system/math/math2d_algorithms.h"

namespace deepmind::lab2d {
namespace {

// Calls `func` on elements in `queue` in order. If the call returns true the
// element is removed from the queue. Otherwise the elements will remain in the
// queue for future processing. `func` is allowed to add entries to `queue`
// while processing but will not be processed until next call.
template <typename T, typename F>
void ProcessQueue(std::vector<T>* queue, F func) {
  if (queue->empty()) {
    return;
  }
  std::vector<T> processing;
  std::swap(*queue, processing);
  processing.erase(std::remove_if(processing.begin(), processing.end(), func),
                   processing.end());
  // Func may have inserted new elements in queue.
  processing.insert(processing.end(), queue->begin(), queue->end());
  std::swap(*queue, processing);
}

math::Orientation2d PickOrientation(Grid::TeleportOrientation mode,
                                    math::Orientation2d original,
                                    math::Orientation2d target,
                                    std::mt19937_64* random) {
  switch (mode) {
    case Grid::TeleportOrientation::kMatchTarget:
      return target;
    case Grid::TeleportOrientation::kPickRandom:
      return static_cast<math::Orientation2d>(
          std::uniform_int_distribution<>(0, 3)(*random));
    case Grid::TeleportOrientation::kKeepOriginal:
      return original;
  }
  std::abort();
}

}  // namespace

Piece Grid::CreateInstance(State state, math::Transform2d transform) {
  if (state.IsEmpty()) return Piece();
  const World::StateData& state_data = world_.state_data(state);
  transform.position = GetShape().Normalised(transform.position);
  const CellIndex grid_position =
      shape_.TryToCellIndex(transform.position, state_data.layer);
  if (!grid_position.IsEmpty() && !grid_[grid_position].IsEmpty()) {
    return Piece();
  }
  const Piece piece =
      piece_data_.Create(state, state_data.layer, transform, frame_counter_);

  pieces_group_membership_.ChangeMembership(
      piece, {}, absl::MakeConstSpan(state_data.groups));
  if (!grid_position.IsEmpty()) {
    grid_[grid_position] = piece;
    SetSprite(grid_position, {state_data.sprite_handle, transform.orientation});
  }
  if (const auto& callback = callbacks_[state]) {
    callback->OnAdd(piece);
  }
  if (!grid_position.IsEmpty()) {
    TriggerOnEnterCallbacks(piece, transform.position);
  }
  return piece;
}

void Grid::ReleaseInstance(Piece piece) {
  if (piece.IsEmpty()) return;
  if (in_update_) {
    to_remove_.push_back(piece);
  } else {
    ReleaseInstanceActual(piece);
  }
}

void Grid::ReleaseInstanceActual(Piece piece) {
  action_queue_.erase(
      std::remove_if(action_queue_.begin(), action_queue_.end(),
                     [piece](const auto& action) {
                       if (piece == action.piece) {
                         return true;
                       }
                       if (auto* connect = absl::get_if<ActionConnect>(
                               &action.action_type)) {
                         return piece == connect->piece;
                       }
                       return false;
                     }),
      action_queue_.end());
  const PieceData& piece_data = piece_data_[piece];
  TriggerOnLeaveCallbacks(piece, piece_data.transform.position);
  const World::StateData& state_data = world_.state_data(piece_data.state);
  if (const auto& callback = callbacks_[piece_data.state]) {
    callback->OnRemove(piece);
  }
  pieces_group_membership_.ChangeMembership(
      piece, absl::MakeConstSpan(state_data.groups), {});
  const CellIndex grid_position =
      shape_.TryToCellIndex(piece_data.transform.position, piece_data.layer);
  if (!grid_position.IsEmpty()) {
    grid_[grid_position] = Piece();
    SetSprite(grid_position, {Sprite(), math::Orientation2d::kNorth});
  }

  DisconnectActual(piece);
  piece_data_.Release(piece);
}

void Grid::SetSprite(CellIndex cell, SpriteInstance sprite) {
  if (in_update_) {
    grid_render_[cell] = sprite;
  } else {
    set_sprite_queue_.push_back(SpriteAction{cell, sprite});
  }
}

void Grid::SetSpriteUntilNextUpdate(CellIndex cell, SpriteInstance sprite) {
  if (in_update_) {
    // Queue sprite change for end of update.
    temp_sprite_locations_immediate_.push_back(SpriteAction{cell, sprite});
  } else {
    // Set sprite and queue for removal at start of next frame.
    temp_sprite_locations_immediate_.push_back(
        SpriteAction{cell, grid_render_[cell]});
    grid_render_[cell] = sprite;
  }
}

void Grid::RunUpdaters(std::mt19937_64* random) {
  for (std::size_t i = 0; i < update_infos_.size(); ++i) {
    const Update update_handle(i);
    const auto& info = update_infos_[update_handle];
    if (info.group.IsEmpty()) {
      continue;
    }
    for (Piece piece :
         pieces_group_membership_[info.group].ShuffledElementsWithProbability(
             random, info.probability)) {
      const auto& piece_data = piece_data_[piece];
      if (frame_counter_ - piece_data.frame_created >= info.start_frame) {
        const auto& callback = callbacks_[piece_data.state];
        if (callback) {
          callback->OnUpdate(update_handle, piece,
                             frame_counter_ - piece_data.frame_created);
        }
      }
    }
  }
}

// When there are permanent sprites that are not on the grid_render_ yet. We
// need to remove all temporary sprites apply permanent sprites then re-apply
// temporary sprites. This will only occur rarely. (I.e. when sprites are
// created or released before rendering without calling update.)
void Grid::Repaint() {
  if (set_sprite_queue_.empty()) {
    return;
  }
  for (auto it = temp_sprite_locations_immediate_.rbegin();
       it != temp_sprite_locations_immediate_.rend(); ++it) {
    std::swap(grid_render_[it->position], it->instance);
  }
  for (auto it = temp_sprite_locations_.rbegin();
       it != temp_sprite_locations_.rend(); ++it) {
    std::swap(grid_render_[it->position], it->instance);
  }

  for (const auto& sprite_action : set_sprite_queue_) {
    grid_render_[sprite_action.position] = sprite_action.instance;
  }
  set_sprite_queue_.clear();

  for (auto& temp_sprite : temp_sprite_locations_) {
    std::swap(grid_render_[temp_sprite.position], temp_sprite.instance);
  }

  for (auto& temp_sprite : temp_sprite_locations_immediate_) {
    std::swap(grid_render_[temp_sprite.position], temp_sprite.instance);
  }
}

void Grid::DoUpdate(std::mt19937_64* random, int flush_count) {
  in_update_ = true;
  // Undo all temporary sprite rendering.
  for (auto it = temp_sprite_locations_immediate_.rbegin();
       it != temp_sprite_locations_immediate_.rend(); ++it) {
    std::swap(grid_render_[it->position], it->instance);
  }
  temp_sprite_locations_immediate_.clear();
  for (auto it = temp_sprite_locations_.rbegin();
       it != temp_sprite_locations_.rend(); ++it) {
    std::swap(grid_render_[it->position], it->instance);
  }
  temp_sprite_locations_.clear();

  for (auto& sprite_action : set_sprite_queue_) {
    grid_render_[sprite_action.position] = sprite_action.instance;
  }
  set_sprite_queue_.clear();

  RunUpdaters(random);

  ++frame_counter_;
  for (int i = 0; i < flush_count + 1; ++i) {
    if (action_queue_.empty()) {
      break;
    }
    ProcessQueue(&action_queue_, [this, random](const Action& action) {
      Piece piece = action.piece;
      return absl::visit(
          [this, random, piece](const auto& arg) -> bool {
            return this->ProcessAction(random, piece, arg);
          },
          action.action_type);
    });
    for (Piece piece : to_remove_) {
      ReleaseInstanceActual(piece);
    }
    to_remove_.clear();
  }

  for (auto& temp_sprite : temp_sprite_locations_) {
    std::swap(grid_render_[temp_sprite.position], temp_sprite.instance);
  }

  for (auto& temp_sprite : temp_sprite_locations_immediate_) {
    std::swap(grid_render_[temp_sprite.position], temp_sprite.instance);
  }
  in_update_ = false;
}

void Grid::SetSpriteImmediate(math::Transform2d trans, Layer layer,
                              Sprite sprite) {
  const CellIndex cell = shape_.ToCellIndex(trans.position, layer);
  if (!cell.IsEmpty()) {
    SetSpriteUntilNextUpdate(cell, SpriteInstance{sprite, trans.orientation});
  }
}

void Grid::SetCallback(State state, std::unique_ptr<StateCallback> callback) {
  if (!state.IsEmpty()) {
    callbacks_[state] = std::move(callback);
  }
}

bool Grid::TeleportToGroupActual(std::mt19937_64* random, Piece piece,
                                 State target_state, Group target_group,
                                 TeleportOrientation teleport_orientation) {
  PieceData& piece_data = piece_data_[piece];
  if (target_group.IsEmpty()) {
    return true;
  }
  const Layer target_layer = target_state.IsEmpty()
                                 ? piece_data.layer
                                 : world_.state_data(target_state).layer;

  const CellIndex current_cell =
      shape_.TryToCellIndex(piece_data.transform.position, piece_data.layer);
  CellIndex target_cell;
  math::Transform2d target_transform;
  auto& group = pieces_group_membership_[target_group];
  const auto* found_piece = group.ShuffledElementsFind(
      random, [this, target_layer, current_cell, &target_cell,
               &target_transform](Piece handle) {
        const auto& piece_data = piece_data_[handle];
        target_transform = piece_data.transform;
        target_cell =
            shape_.ToCellIndex(target_transform.position, target_layer);
        return !target_cell.IsEmpty() &&
               (grid_[target_cell].IsEmpty() || current_cell == target_cell);
      });
  if (found_piece == nullptr) {
    return false;
  }
  target_transform.orientation =
      PickOrientation(teleport_orientation, piece_data.transform.orientation,
                      target_transform.orientation, random);
  teleport_orientation = TeleportOrientation::kKeepOriginal;

  if (current_cell != target_cell) {
    if (!current_cell.IsEmpty()) {
      // Current is valid, swap from current to target.
      std::swap(grid_[target_cell], grid_[current_cell]);
      grid_render_[current_cell] = grid_render_[target_cell];
    } else {
      grid_[target_cell] = piece;
    }
  }

  target_state = !target_state.IsEmpty() ? target_state : piece_data.state;

  const World::StateData& target_state_data = world_.state_data(target_state);

  grid_render_[target_cell] = SpriteInstance{target_state_data.sprite_handle,
                                             target_transform.orientation};

  const State source_state = piece_data.state;
  const World::StateData& source_state_data = world_.state_data(source_state);
  TriggerOnLeaveCallbacks(piece, piece_data.transform.position);
  if (source_state != target_state) {
    if (const auto& callback = callbacks_[source_state]) {
      callback->OnRemove(piece);
    }
    pieces_group_membership_.ChangeMembership(
        piece, absl::MakeConstSpan(source_state_data.groups),
        absl::MakeConstSpan(target_state_data.groups));
    piece_data.transform = target_transform;
    piece_data.state = target_state;
    piece_data.frame_created = frame_counter_;
    piece_data.layer = target_state_data.layer;
    if (const auto& callback = callbacks_[target_state]) {
      callback->OnAdd(piece);
    }
  } else {
    piece_data.transform = target_transform;
  }
  TriggerOnEnterCallbacks(piece, piece_data.transform.position);
  return true;
}

bool Grid::SetStateActual(Piece piece, State target_state) {
  if (target_state.IsEmpty()) {
    return true;
  }
  auto& piece_data = piece_data_[piece];
  const State source_state = piece_data.state;
  math::Transform2d transform = piece_data.transform;
  const auto& source_state_data = world_.state_data(source_state);
  const auto& target_state_data = world_.state_data(target_state);

  const CellIndex target_cell =
      shape_.TryToCellIndex(transform.position, target_state_data.layer);
  if (target_state_data.layer != piece_data.layer) {
    const CellIndex current_cell =
        shape_.TryToCellIndex(transform.position, piece_data.layer);
    if (target_cell.IsEmpty()) {
      TriggerOnLeaveCallbacks(piece, piece_data.transform.position);
      // Target out of bounds, hide the piece.
      grid_[current_cell] = Piece();
      grid_render_[current_cell].handle = Sprite();
    } else if (!grid_[target_cell].IsEmpty()) {
      // Target occupied, cannot change state.
      return false;
    } else if (!current_cell.IsEmpty()) {
      TriggerOnLeaveCallbacks(piece, piece_data.transform.position);
      // Current is valid, swap from current to target.
      std::swap(grid_[target_cell], grid_[current_cell]);
      grid_render_[current_cell] = grid_render_[target_cell];
    } else {
      // No piece at current, target is clear, so create new piece at target.
      grid_[target_cell] = piece;
    }
  }

  if (!target_cell.IsEmpty()) {
    grid_render_[target_cell] = SpriteInstance{
        target_state_data.sprite_handle, piece_data.transform.orientation};
  }
  if (const auto& callback = callbacks_[source_state]) {
    callback->OnRemove(piece);
  }
  pieces_group_membership_.ChangeMembership(
      piece, absl::MakeConstSpan(source_state_data.groups),
      absl::MakeConstSpan(target_state_data.groups));
  piece_data.frame_created = frame_counter_;
  piece_data.state = target_state;
  piece_data.layer = target_state_data.layer;
  if (const auto& callback = callbacks_[target_state]) {
    callback->OnAdd(piece);
  }
  if (!target_cell.IsEmpty()) {
    TriggerOnEnterCallbacks(piece, piece_data.transform.position);
  }
  return true;
}

absl::Span<const SpriteInstance> Grid::AllSpriteInstances(
    math::Position2d pos) {
  Repaint();
  const CellIndex start_index = shape_.ToCellIndex(pos, Layer(0));
  return absl::MakeSpan(&grid_render_[start_index], world_.NumRenderLayers());
}

absl::Span<const Piece> Grid::AllPieceHandles(math::Position2d pos) const {
  const CellIndex start_index = shape_.ToCellIndex(pos, Layer(0));
  return absl::MakeSpan(&grid_[start_index], shape_.layer_count());
}

void Grid::UpdateRenderOrientation(const PieceData& piece_data) {
  const CellIndex cell =
      shape_.TryToCellIndex(piece_data.transform.position, piece_data.layer);
  if (!cell.IsEmpty()) {
    grid_render_[cell].orientation = piece_data.transform.orientation;
  }
}

void Grid::RotatePieceActual(Piece piece, math::Rotate2d rotate) {
  PieceData& piece_data = piece_data_[piece];
  piece_data.transform.orientation = piece_data.transform.orientation + rotate;
  UpdateRenderOrientation(piece_data);
}

void Grid::SetPieceOrientationActual(Piece piece,
                                     math::Orientation2d orientation) {
  PieceData& piece_data = piece_data_[piece];
  piece_data.transform.orientation = orientation;
  UpdateRenderOrientation(piece_data);
}

void Grid::TeleportPieceActual(std::mt19937_64* random, Piece piece,
                               math::Position2d position,
                               TeleportOrientation teleport_orientation) {
  position = GetShape().Normalised(position);
  PieceData& piece_data = piece_data_[piece];
  math::Orientation2d orientation =
      PickOrientation(teleport_orientation, piece_data.transform.orientation,
                      piece_data.transform.orientation, random);
  if (piece_data.layer.IsEmpty()) {
    if (shape_.InBounds(position)) {
      piece_data.transform = {position, orientation};
    }
    return;
  }
  LiftPiece(piece);
  math::Vector2d offset = position - piece_data.transform.position;

  auto [can_move, blocker] = CanPlacePiece(piece, offset, piece_data.layer);

  if (!can_move) {
    offset = math::Vector2d::Zero();
    orientation = piece_data.transform.orientation;
  }
  piece_data.transform.orientation = orientation;
  PlacePiece(piece, offset, piece_data.layer);
  if (!can_move) {
    if (auto& callback_mover = callbacks_[piece_data.state];
        callback_mover != nullptr) {
      callback_mover->OnBlocked(piece, blocker);
    }
  }
}

void Grid::LiftPiece(Piece piece) {
  // Lift all pieces off grid.
  VisitConnected(piece, [this](Piece handle) {
    auto& piece_data = piece_data_[handle];
    TriggerOnLeaveCallbacks(handle, piece_data.transform.position);
    const CellIndex current_cell =
        shape_.TryToCellIndex(piece_data.transform.position, piece_data.layer);
    if (!current_cell.IsEmpty()) {
      grid_[current_cell] = Piece();
      grid_render_[current_cell].handle = Sprite();
    }
  });
}

void Grid::PlacePiece(Piece piece, math::Vector2d offset, Layer layer) {
  // Place pieces in new location or return them to the original location.
  VisitConnected(piece, [this, offset, piece, layer](Piece handle) {
    auto& piece_data = piece_data_[handle];
    Layer piece_data_layer = piece == handle ? layer : piece_data.layer;
    piece_data.transform.position =
        GetShape().Normalised(piece_data.transform.position + offset);
    piece_data.layer = piece_data_layer;
    const CellIndex target_cell =
        shape_.TryToCellIndex(piece_data.transform.position, piece_data_layer);
    if (!target_cell.IsEmpty()) {
      grid_[target_cell] = handle;
      const auto& state_data = world_.state_data(piece_data.state);
      grid_render_[target_cell] = {state_data.sprite_handle,
                                   piece_data.transform.orientation};
      TriggerOnEnterCallbacks(handle, piece_data.transform.position);
    }
  });
}

std::pair<bool, Piece> Grid::CanPlacePiece(Piece piece, math::Vector2d offset,
                                           Layer layer) const {
  // Detect if move is possible.
  Piece blocker;
  bool cannot_move = AnyInConnected(
      piece, [this, offset, &blocker, piece, layer](Piece handle) {
        const auto& piece_data = piece_data_[handle];
        Layer piece_data_layer = piece == handle ? layer : piece_data.layer;
        const CellIndex current_cell = shape_.TryToCellIndex(
            piece_data.transform.position, piece_data_layer);
        if (current_cell.IsEmpty()) {
          return true;
        }

        const math::Position2d target = piece_data.transform.position + offset;
        // Is attempting to move off grid?
        if (!shape_.InBounds(target)) {
          return true;
        }
        if (!piece_data_layer.IsEmpty()) {
          const CellIndex target_cell =
              shape_.ToCellIndex(target, piece_data_layer);
          const Piece possible_blocker = grid_[target_cell];
          // Is blocked by another object>
          if (!possible_blocker.IsEmpty()) {
            blocker = possible_blocker;
            return true;
          }
        }
        return false;
      });
  return {!cannot_move, blocker};
}

void Grid::PushPieceActual(Piece piece, math::Orientation2d push_direction,
                           Perspective perspective) {
  auto& piece_data = piece_data_[piece];
  math::Vector2d direction =
      math::Vector2d::North() * (push_direction - math::Orientation2d::kNorth);
  if (perspective == Perspective::kPiece) {
    direction *=
        (piece_data.transform.orientation - math::Orientation2d::kNorth);
  }
  if (piece_data.layer.IsEmpty()) {
    math::Position2d new_position = piece_data.transform.position + direction;
    if (shape_.InBounds(new_position)) {
      piece_data.transform.position = new_position;
    } else {
      if (auto& callback_mover = callbacks_[piece_data.state];
          callback_mover != nullptr) {
        callback_mover->OnBlocked(piece, Piece());
      }
    }
    return;
  }
  // Lift all pieces off grid.
  LiftPiece(piece);

  // Detect if move is possible.
  auto [can_move, blocker] = CanPlacePiece(piece, direction, piece_data.layer);

  if (!can_move) {
    direction = math::Vector2d::Zero();
  }

  PlacePiece(piece, direction, piece_data.layer);

  if (!can_move) {
    if (auto& callback_mover = callbacks_[piece_data.state];
        callback_mover != nullptr) {
      callback_mover->OnBlocked(piece, blocker);
    }
  }
}

void Grid::TriggerOnEnterCallbacks(Piece piece, math::Position2d pos) {
  if (!shape_.InBounds(pos)) {
    return;
  }
  State source_state = piece_data_[piece].state;
  const World::StateData& source_state_data = world_.state_data(source_state);
  auto& callback_source = callbacks_[source_state];
  for (Piece target_handle : AllPieceHandles(pos)) {
    if (target_handle.IsEmpty() || target_handle == piece) continue;
    PieceData& target_piece_data = piece_data_[target_handle];
    const auto& target_state_data = world_.state_data(target_piece_data.state);
    const auto& callback_target = callbacks_[target_piece_data.state];
    if (callback_target != nullptr &&
        !source_state_data.contact_handle.IsEmpty()) {
      callback_target->OnEnter(source_state_data.contact_handle, target_handle,
                               piece);
    }
    if (!target_state_data.contact_handle.IsEmpty()) {
      if (callback_source != nullptr) {
        callback_source->OnEnter(target_state_data.contact_handle, piece,
                                 target_handle);
      }
    }
  }
}

void Grid::TriggerOnLeaveCallbacks(Piece piece, math::Position2d pos) {
  if (!shape_.InBounds(pos)) {
    return;
  }
  State source_state = piece_data_[piece].state;
  const World::StateData& source_state_data = world_.state_data(source_state);
  auto& callback_source = callbacks_[source_state];
  for (Piece target_handle : AllPieceHandles(pos)) {
    if (target_handle.IsEmpty() || target_handle == piece) continue;
    PieceData& target_piece_data = piece_data_[target_handle];
    const auto& target_state_data = world_.state_data(target_piece_data.state);
    const auto& callback_target = callbacks_[target_piece_data.state];
    if (callback_target != nullptr &&
        !source_state_data.contact_handle.IsEmpty()) {
      callback_target->OnLeave(source_state_data.contact_handle, target_handle,
                               piece);
    }
    if (!target_state_data.contact_handle.IsEmpty()) {
      if (callback_source != nullptr) {
        callback_source->OnLeave(target_state_data.contact_handle, piece,
                                 target_handle);
      }
    }
  }
}

void Grid::Render(math::Transform2d transform, const GridView& grid_view,
                  absl::Span<int> output_sprites) {
  if (output_sprites.empty()) {
    return;
  }
  Repaint();
  switch (GetShape().topology()) {
    case GridShape::Topology::kBounded:
      RenderBounded(transform, grid_view, output_sprites);
      return;
    case GridShape::Topology::kTorus:
      RenderTorus(transform, grid_view, output_sprites);
      return;
  }
  LOG(FATAL) << "Invalid topology: " << static_cast<int>(GetShape().topology());
}

struct GridToView {
  GridToView(math::Transform2d transform, const math::Size2d grid_size,
             const GridView& grid_view) {
    const GridWindow& grid_window = grid_view.GetWindow();
    const math::Size2d view_port_size = grid_window.size2d();
    switch (transform.orientation) {
      default:
      // x => x
      // y => y
      case math::Orientation2d::kNorth:
        first_x = transform.position.x - grid_window.left();
        last_x = transform.position.x + grid_window.right();
        first_y = transform.position.y - grid_window.forward();
        last_y = transform.position.y + grid_window.backward();
        span_x = 1;
        span_y = view_port_size.width;
        offset_x = first_x;
        offset_y = first_y;
        break;

      // y = x
      // x = -y
      case math::Orientation2d::kEast:
        first_x = transform.position.x - grid_window.backward();
        last_x = transform.position.x + grid_window.forward();
        first_y = transform.position.y - grid_window.left();
        last_y = transform.position.y + grid_window.right();
        span_x = -view_port_size.width;
        span_y = 1;
        offset_x = last_x;
        offset_y = first_y;
        break;

      // x = -x
      // y = -y
      case math::Orientation2d::kSouth:
        first_x = transform.position.x - grid_window.right();
        last_x = transform.position.x + grid_window.left();
        first_y = transform.position.y - grid_window.backward();
        last_y = transform.position.y + grid_window.forward();
        span_x = -1;
        span_y = -view_port_size.width;
        offset_x = last_x;
        offset_y = last_y;
        break;

      // y = -x
      // x = y
      case math::Orientation2d::kWest:
        first_x = transform.position.x - grid_window.forward();
        last_x = transform.position.x + grid_window.backward();
        first_y = transform.position.y - grid_window.right();
        last_y = transform.position.y + grid_window.left();
        span_x = view_port_size.width;
        span_y = -1;
        offset_x = first_x;
        offset_y = last_y;
        break;
    }
  }
  int first_x;   // Grid inclusive first x
  int last_x;    // Grid inclusive last x
  int first_y;   // Grid inclusive first y
  int last_y;    // Grid inclusive last y
  int span_x;    // Calculated such that view_x = first_x * span_x + offset_x
  int span_y;    // Calculated such that view_y = first_y * span_y + offset_y
  int offset_x;  // See span_x.
  int offset_y;  // See span_y.
};

void Grid::RenderTorus(math::Transform2d transform, const GridView& grid_view,
                       absl::Span<int> output_sprites) const {
  const int num_render_layers = grid_view.NumRenderLayers();
  const math::Size2d grid_size = shape_.GridSize2d();
  CHECK_EQ(output_sprites.size(), grid_view.NumCells())
      << "Incorrect output_sprites size.";

  GridToView grid_to_view(transform, grid_size, grid_view);

  const SpriteInstance* render_grid = &grid_render_[CellIndex(0)];
  const int layer_count = shape_.layer_count();

  for (int y = grid_to_view.first_y; y <= grid_to_view.last_y; ++y) {
    int view_y = (y - grid_to_view.offset_y) * grid_to_view.span_y;
    int grid_y = GetShape().ModuloHeight(y) * grid_size.width;
    for (int x = grid_to_view.first_x; x <= grid_to_view.last_x; ++x) {
      int view_x = (x - grid_to_view.offset_x) * grid_to_view.span_x;
      int view_pos = (view_y + view_x) * num_render_layers;
      int grid_pos = (grid_y + GetShape().ModuloWidth(x)) * layer_count;
      for (int i = 0; i < num_render_layers; ++i) {
        CHECK_LT(grid_pos, grid_render_.size());
        SpriteInstance instance = render_grid[grid_pos + i];
        instance.orientation =
            math::FromView(transform.orientation, instance.orientation);
        output_sprites[view_pos + i] = grid_view.ToSpriteId(instance);
      }
    }
  }
}

void Grid::RenderBounded(math::Transform2d transform, const GridView& grid_view,
                         absl::Span<int> output_sprites) const {
  const int num_render_layers = grid_view.NumRenderLayers();
  const math::Size2d grid_size = shape_.GridSize2d();
  CHECK_EQ(output_sprites.size(), grid_view.NumCells())
      << "Incorrect output_sprites size.";

  GridToView grid_to_view(transform, grid_size, grid_view);

  // All values are inclusive.
  //      first_inbounds_x
  //  first_x    |   last_inbounds_x
  //    |        |         |      last_x
  //    +---------------------------+ first_y
  //    |        |         |        |
  //    +--------+---------+--------+ first_inbounds_y
  //    |        |         |        |
  //    |        | Visible |        |
  //    |        |         |        |
  //    +--------+---------+--------+ last_inbounds_y
  //    |        |         |        |
  //    +---------------------------+ last_y
  int first_inbounds_x = std::max(0, grid_to_view.first_x);
  int last_inbounds_x = std::min(grid_size.width - 1, grid_to_view.last_x);
  int first_inbounds_y = std::max(0, grid_to_view.first_y);
  int last_inbounds_y = std::min(grid_size.height - 1, grid_to_view.last_y);
  if (grid_to_view.first_x != first_inbounds_x ||
      grid_to_view.last_x != last_inbounds_x ||
      grid_to_view.first_y != first_inbounds_y ||
      grid_to_view.last_y != last_inbounds_y) {
    const SpriteInstance clear = {grid_view.OutOfBoundsSprite(),
                                  transform.orientation};
    std::fill(output_sprites.begin(), output_sprites.end(),
              grid_view.ToSpriteId(clear));
  }

  const SpriteInstance* render_grid = &grid_render_[CellIndex(0)];
  const int layer_count = shape_.layer_count();
  for (int y = first_inbounds_y; y <= last_inbounds_y; ++y) {
    int view_y = (y - grid_to_view.offset_y) * grid_to_view.span_y;
    int grid_y = y * grid_size.width;
    for (int x = first_inbounds_x; x <= last_inbounds_x; ++x) {
      int view_x = (x - grid_to_view.offset_x) * grid_to_view.span_x;
      int view_pos = (view_y + view_x) * num_render_layers;
      int grid_pos = (grid_y + x) * layer_count;
      for (int i = 0; i < num_render_layers; ++i) {
        SpriteInstance instance = render_grid[grid_pos + i];
        instance.orientation =
            math::FromView(transform.orientation, instance.orientation);
        output_sprites[view_pos + i] = grid_view.ToSpriteId(instance);
      }
    }
  }
}

std::string Grid::ToString() {
  Repaint();
  std::string result;
  const math::Size2d grid_size = shape_.GridSize2d();
  result.resize(grid_size.Area() + grid_size.height, ' ');
  if (world_.NumRenderLayers() == 0) {
    for (int i = 0; i < grid_size.height; ++i) {
      result[(i + 1) * (grid_size.width + 1) - 1] = '\n';
    }
  } else {
    char* character = &result[0];
    for (int i = 0; i < grid_size.height; ++i) {
      for (int j = 0; j < grid_size.width; ++j) {
        for (SpriteInstance sprite : AllSpriteInstances({j, i})) {
          if (!sprite.handle.IsEmpty()) {
            const auto& sprite_name = world_.sprites().ToName(sprite.handle);
            if (!sprite_name.empty()) {
              *character = sprite_name.front();
            }
          }
        }
        ++character;
      }
      *character++ = '\n';
    }
  }
  return result;
}

// Returns whether the hit was blocked.
Grid::HitResponse Grid::DoHit(Piece instigator, Hit hit,
                              const math::Transform2d& trans,
                              const World::HitData& hit_data) {
  if (!shape_.InBounds(trans.position)) {
    return HitResponse::kBlocked;
  }

  bool blocked = false;
  // Hit every piece at x, y return whether any blocked.
  for (Piece target_handle : AllPieceHandles(trans.position)) {
    if (target_handle.IsEmpty()) continue;
    PieceData& target_piece_data = piece_data_[target_handle];
    const auto& callback = callbacks_[target_piece_data.state];
    bool on_hit = callback != nullptr &&
                  callback->OnHit(hit, target_handle, instigator) ==
                      HitResponse::kBlocked;
    blocked = blocked || on_hit;
  }

  if (!blocked && !hit_data.layer.IsEmpty() &&
      !hit_data.sprite_handle.IsEmpty()) {
    CellIndex sprite_pos = shape_.ToCellIndex(trans.position, hit_data.layer);
    temp_sprite_locations_.push_back(
        {sprite_pos, {hit_data.sprite_handle, trans.orientation}});
  }
  return blocked ? HitResponse::kBlocked : HitResponse::kContinue;
}

Grid::HitResponse Grid::CheckHitLineSegment(Piece instigator, Hit hit,
                                            const World::HitData& hit_data,
                                            math::Transform2d trans,
                                            int length) {
  math::Vector2d dir = math::Vector2d::FromOrientation(trans.orientation);
  for (int i = 0; i < length; ++i) {
    if (DoHit(instigator, hit, trans, hit_data) == HitResponse::kBlocked) {
      return i == 0 ? HitResponse::kBlocked : HitResponse::kContinue;
    }
    trans.position += dir;
  }
  return HitResponse::kContinue;
}

void Grid::HitBeamActual(Piece instigator, Hit hit, int length, int radius) {
  const auto& piece_data = piece_data_[instigator];
  math::Transform2d start = piece_data.transform;
  const CellIndex cell =
      shape_.TryToCellIndex(start.position, piece_data.layer);
  if (cell.IsEmpty()) {
    return;
  }
  const auto& hit_data = world_.hit_data(hit);
  const math::Rotate2d north_to_forward =
      start.orientation - math::Orientation2d::kNorth;
  const math::Vector2d forward = math::Vector2d::North() * north_to_forward;

  for (auto direction : {math::Vector2d::West(), math::Vector2d::East()}) {
    math::Vector2d sideways = direction * north_to_forward;
    for (int r = 1; r <= radius; ++r) {
      math::Transform2d trans = start;
      trans.position += r * sideways;
      if (CheckHitLineSegment(instigator, hit, hit_data, trans,
                              length - r + 1) == HitResponse::kBlocked) {
        break;
      }
    }
  }
  start.position += forward;
  CheckHitLineSegment(instigator, hit, hit_data, start, length);
}

void Grid::ConnectActual(Piece piece1_handle, Piece piece2_handle) {
  if (piece1_handle == piece2_handle) {
    return;
  }
  auto& piece1 = piece_data_[piece1_handle];
  auto& piece2 = piece_data_[piece2_handle];
  if (piece1.connect_next.IsEmpty() && piece2.connect_next.IsEmpty()) {
    piece1.connect_next = piece2_handle;
    piece1.connect_prev = piece2_handle;
    piece2.connect_next = piece1_handle;
    piece2.connect_prev = piece1_handle;
  } else if (piece1.connect_next.IsEmpty()) {
    Piece piece0_handle = piece2.connect_prev;
    auto& piece0 = piece_data_[piece0_handle];
    // Insert piece1 before piece2.
    piece1.connect_next = piece2_handle;
    piece1.connect_prev = piece0_handle;
    piece2.connect_prev = piece1_handle;
    piece0.connect_next = piece1_handle;
  } else if (piece2.connect_next.IsEmpty()) {
    Piece piece3_handle = piece1.connect_next;
    auto& piece3 = piece_data_[piece3_handle];
    // Insert piece2 after piece1
    piece1.connect_next = piece2_handle;
    piece2.connect_prev = piece1_handle;
    piece2.connect_next = piece3_handle;
    piece3.connect_prev = piece2_handle;
  } else {
    // Check if they are already connected.
    for (Piece next = piece1.connect_next; next != piece1_handle;
         next = piece_data_[next].connect_next) {
      if (next == piece2_handle) {
        return;
      }
    }
    // Insert piece2 ring before piece1 ring.
    Piece p1_prev_handle = piece1.connect_prev;
    auto& p1_prev = piece_data_[p1_prev_handle];
    Piece p2_next_handle = piece2.connect_next;
    auto& p2_next = piece_data_[p2_next_handle];
    p1_prev.connect_next = p2_next_handle;
    p2_next.connect_prev = p1_prev_handle;
    piece1.connect_prev = piece2_handle;
    piece2.connect_next = piece1_handle;
  }
}

void Grid::DisconnectAllActual(Piece piece) {
  auto& piece_data = piece_data_[piece];
  Piece piece_next_handle = piece_data.connect_next;
  if (piece_next_handle.IsEmpty()) {
    return;
  }

  for (Piece current_handle = piece;; current_handle = piece_next_handle) {
    auto& current = piece_data_[current_handle];
    piece_next_handle = current.connect_next;
    current.connect_next = Piece();
    current.connect_prev = Piece();
    if (piece_next_handle == piece) {
      break;
    }
  }
}

void Grid::DisconnectActual(Piece piece) {
  auto& piece_data = piece_data_[piece];
  Piece piece_prev_handle = piece_data.connect_prev;
  if (piece_prev_handle.IsEmpty()) {
    return;
  }
  Piece piece_next_handle = piece_data.connect_next;
  auto& piece_prev = piece_data_[piece_prev_handle];
  auto& piece_next = piece_data_[piece_next_handle];
  if (piece_next_handle != piece_prev_handle) {
    piece_prev.connect_next = piece_next_handle;
    piece_next.connect_prev = piece_prev_handle;
  } else {
    piece_prev.connect_next = Piece();
    piece_next.connect_prev = Piece();
  }
  piece_data_[piece].connect_next = Piece();
  piece_data_[piece].connect_prev = Piece();
}

absl::optional<Grid::FindPieceResult> Grid::RayCastDirection(
    Layer layer, math::Position2d start, math::Vector2d direction) const {
  const CellIndex start_cell = shape_.TryToCellIndex(start, layer);
  absl::optional<FindPieceResult> result{};
  if (start_cell.IsEmpty()) {
    result.emplace().position = start;
    return result;
  }
  math::Position2d previous = start;
  math::RayCastLine(
      start, start + direction,
      [this, layer, &result, &previous](math::Position2d position) {
        if (!shape_.InBounds(position)) {
          result.emplace().position = previous;
          return true;
        }
        previous = position;
        const CellIndex cell = shape_.ToCellIndex(position, layer);
        const Piece piece = grid_[cell];
        if (!piece.IsEmpty()) {
          auto& hit = result.emplace();
          hit.position = position;
          hit.piece = piece;
          return true;
        } else {
          return false;
        }
      });

  return result;
}

absl::optional<Grid::FindPieceResult> Grid::RayCast(
    Layer layer, math::Position2d start, math::Position2d end) const {
  return RayCastDirection(layer, start, GetShape().SmallestVector(start, end));
}

Piece Grid::GetPieceAtPosition(Layer layer, math::Position2d position) {
  const CellIndex cell = shape_.TryToCellIndex(position, layer);
  if (cell.IsEmpty()) {
    return Piece{};
  }
  return grid_[cell];
}

void Grid::FindPiece(math::Position2d position, Layer layer,
                     std::vector<FindPieceResult>* result) {
  const CellIndex cell = shape_.ToCellIndex(position, layer);
  const Piece piece = grid_[cell];
  if (!piece.IsEmpty()) {
    Grid::FindPieceResult& hit = result->emplace_back();
    hit.piece = piece;
    hit.position = position;
  }
}

std::vector<Grid::FindPieceResult> Grid::DiscFindAll(Layer layer,
                                                     math::Position2d center,
                                                     int radius) {
  std::vector<Grid::FindPieceResult> result;
  if (layer.IsEmpty() || radius < 0) {
    return result;
  }
  switch (GetShape().topology()) {
    case GridShape::Topology::kBounded:
      math::VisitDisc(center, radius,
                      [this, layer, &result](math::Position2d position) {
                        if (!shape_.InBounds(position)) {
                          return;
                        }
                        FindPiece(position, layer, &result);
                      });
      return result;
    case GridShape::Topology::kTorus:
      math::VisitDisc(center, radius,
                      [this, layer, &result](math::Position2d position) {
                        FindPiece(position, layer, &result);
                      });
      return result;
  }

  LOG(FATAL) << "Invalid topology " << static_cast<int>(GetShape().topology());
}

std::vector<Grid::FindPieceResult> Grid::DiamondFindAll(Layer layer,
                                                        math::Position2d center,
                                                        int radius) {
  std::vector<Grid::FindPieceResult> result;
  if (layer.IsEmpty() || radius < 0) {
    return result;
  }

  switch (GetShape().topology()) {
    case GridShape::Topology::kBounded:
      math::VisitDiamond(center, radius,
                         [this, layer, &result](math::Position2d position) {
                           if (!shape_.InBounds(position)) {
                             return;
                           }
                           FindPiece(position, layer, &result);
                         });
      return result;
    case GridShape::Topology::kTorus:
      math::VisitDiamond(center, radius,
                         [this, layer, &result](math::Position2d position) {
                           FindPiece(position, layer, &result);
                         });
      return result;
  }

  LOG(FATAL) << "Invalid topology " << static_cast<int>(GetShape().topology());
}

std::vector<Grid::FindPieceResult> Grid::RectangleFindAll(
    Layer layer, math::Position2d corner0, math::Position2d corner1) {
  std::vector<Grid::FindPieceResult> result{};
  if (layer.IsEmpty()) {
    return result;
  }
  switch (GetShape().topology()) {
    case GridShape::Topology::kBounded:
      math::VisitRectangleClamped(
          corner0, corner1, shape_.GridSize2d(),
          [layer, &result, this](math::Position2d position) {
            FindPiece(position, layer, &result);
          });
      return result;
    case GridShape::Topology::kTorus:
      math::VisitRectangle(corner0, corner1,
                           [layer, &result, this](math::Position2d position) {
                             FindPiece(position, layer, &result);
                           });
      return result;
  }
  LOG(FATAL) << "Invalid topology " << static_cast<int>(GetShape().topology());
}

}  // namespace deepmind::lab2d
