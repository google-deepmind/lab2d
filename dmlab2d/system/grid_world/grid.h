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

#ifndef DMLAB2D_SYSTEM_GRID_WORLD_GRID_H_
#define DMLAB2D_SYSTEM_GRID_WORLD_GRID_H_

#include <memory>
#include <random>
#include <string>
#include <utility>
#include <vector>

#include "absl/types/any.h"
#include "absl/types/optional.h"
#include "absl/types/span.h"
#include "dmlab2d/system/grid_world/collections/fixed_handle_map.h"
#include "dmlab2d/system/grid_world/collections/object_pool.h"
#include "dmlab2d/system/grid_world/collections/shuffled_membership.h"
#include "dmlab2d/system/grid_world/grid_shape.h"
#include "dmlab2d/system/grid_world/grid_view.h"
#include "dmlab2d/system/grid_world/handles.h"
#include "dmlab2d/system/grid_world/sprite_instance.h"
#include "dmlab2d/system/grid_world/world.h"
#include "dmlab2d/system/math/math2d.h"

namespace deepmind::lab2d {

// Creates and stores entities on a grid.
class Grid {
 public:
  enum class Perspective { kGrid, kPiece };
  enum class HitResponse { kContinue, kBlocked };

  enum class TeleportOrientation {
    kMatchTarget,
    kKeepOriginal,
    kPickRandom,
  };

  struct FindPieceResult {
    math::Position2d position;
    Piece piece;
  };

  // Callbacks from events in the engine.
  class StateCallback {
   public:
    virtual ~StateCallback() = default;
    virtual void OnAdd(Piece piece) = 0;
    virtual void OnRemove(Piece piece) = 0;
    virtual void OnUpdate(Update update, Piece piece,
                          int num_frames_in_state) = 0;
    virtual void OnBlocked(Piece piece, Piece blocker) = 0;
    virtual void OnEnter(Contact contact, Piece piece, Piece instigator) = 0;
    virtual void OnLeave(Contact contact, Piece piece, Piece instigator) = 0;
    virtual HitResponse OnHit(Hit hit, Piece piece, Piece instigator) = 0;
  };

  // `world` is captured by reference and must out-last *this.
  Grid(const World& world, math::Size2d grid_size, GridShape::Topology topology)
      : world_(world),
        shape_(grid_size, world_.layers().NumElements(), topology),
        pieces_group_membership_(world_.groups().NumElements()),
        update_infos_(world_.updates().NumElements()),
        callbacks_(world_.states().NumElements()),
        grid_(shape_.GetCellCount()),
        grid_render_(shape_.GetCellCount()) {}

  Grid(Grid&&) = default;

  // Sets which pieces to update during `update`. `group` - The set of pieces to
  // update. `probability` - The probability of updating any individual piece
  // within the set. `start_frame` - The number of frames as state for any piece
  // before the piece begins updating.
  void SetUpdateInfo(Update update, Group group, double probability,
                     int start_frame) {
    auto& info = update_infos_[update];
    info.group = group;
    info.probability = probability;
    info.start_frame = start_frame;
  }

  void SetCallback(State state, std::unique_ptr<StateCallback> callback);

  // Returns a new piece in state `state` at `transform` if location and layer
  // are available, otherwise returns the empty handle.
  Piece CreateInstance(State state, math::Transform2d transform);

  void ReleaseInstance(Piece piece);

  void DoUpdate(std::mt19937_64* random, int flush_count = 128);

  // Overrides the sprite rendered at a location until the next call to
  // `DoUpdate`.
  void SetSpriteImmediate(math::Transform2d trans, Layer layer, Sprite sprite);

  // Queues rotating the piece in place.
  void RotatePiece(Piece piece, math::Rotate2d rotate) {
    action_queue_.push_back({piece, ActionRotate{rotate}});
  }

  // Queues setting the piece's orientation in place.
  void SetPieceOrientation(Piece piece, math::Orientation2d orientation) {
    action_queue_.push_back({piece, ActionSetOrientation{orientation}});
  }

  // Queues a move of the piece in specified `push_direction`. If `perspective`
  // is `kPieceRelative` then a `push_direction` of kNorth is forward for the
  // piece otherwise the pespective is `kGridRelative` and the `push_direction`
  // is relative to the grid orientation.
  void PushPiece(Piece piece, math::Orientation2d push_direction,
                 Perspective perspective) {
    action_queue_.push_back({piece, ActionPush{push_direction, perspective}});
  }

  // Queues settting the piece's position. If the target position is off grid
  // then the piece will no longer be visible.
  void TeleportPiece(Piece piece, math::Position2d position,
                     TeleportOrientation orientation) {
    action_queue_.push_back({piece, ActionTeleport{position, orientation}});
  }

  // Queues a state change for processing during update. If the transition
  // requires a layer change and the layer is occupied the transition remains
  // in the queue until next time. If there is a subsequent transitions that
  // will take presidence.
  void SetState(Piece piece, State state) {
    action_queue_.push_back({piece, ActionSetState{state}});
  }

  // Queues a teleport of a piece to random non occupied position and layer
  // represented by pieces on a different layer belonging to a group specified
  // by `group_handle`. If `target_state` is not empty the piece will
  // change state when teleported. The teleport queue is processed during
  // `DoUpdate()` and may fail if there are no free locations available. If this
  // happens the teleport remains in the queue until processed. If there is a
  // subsequent calls they will take presidence.
  void TeleportToGroup(Piece piece, Group group, State state,
                       TeleportOrientation teleport_orientation) {
    action_queue_.push_back(
        {piece, ActionTeleportToGroup{state, group, teleport_orientation}});
  }

  const GridShape& GetShape() const { return shape_; }

  std::size_t PieceCountByGroup(Group group_handle) {
    return !group_handle.IsEmpty()
               ? pieces_group_membership_[group_handle].NumElements()
               : 0;
  }

  absl::Span<const Piece> PiecesByGroupShuffled(Group group_handle,
                                                std::mt19937_64* random) {
    if (group_handle.IsEmpty()) {
      return {};
    }
    return absl::MakeConstSpan(
        pieces_group_membership_[group_handle].ShuffledElements(random));
  }

  absl::Span<const Piece> PiecesByGroupShuffledWithMaxCount(
      Group group_handle, int max_count, std::mt19937_64* random) {
    if (group_handle.IsEmpty()) {
      return {};
    }
    return pieces_group_membership_[group_handle].ShuffledElementsWithMaxCount(
        random, max_count);
  }

  absl::Span<const Piece> PiecesByGroupShuffledWithProbabilty(
      Group group_handle, double probability, std::mt19937_64* random) {
    if (group_handle.IsEmpty()) {
      return {};
    }
    return pieces_group_membership_[group_handle]
        .ShuffledElementsWithProbability(random, probability);
  }

  // Returns first piece on grid in line from `start` to `end` not including
  // `start` if exists, otherwise the result will have an empty piece.
  // Line follows orthoganal traversal as described by math::RayCastLine.
  // If the ray leaves the grid then the result will have an empty piece
  // but the position will be the last valid location or start. When topology
  // is torus the position is converted to the nearest.
  absl::optional<FindPieceResult> RayCast(Layer layer, math::Position2d start,
                                          math::Position2d end) const;

  // Returns first piece on grid in line from `start` in direction `direction`
  // not including `start` if exists, otherwise the result will have an empty
  // piece. Line follows orthoganal traversal as described by
  // math::RayCastLine. If the ray leaves the grid then the result will have an
  // empty piece but the position will be the last valid location or
  // start.
  absl::optional<FindPieceResult> RayCastDirection(
      Layer layer, math::Position2d start, math::Vector2d Direction) const;

  // Returns piece at `position` and `layer`.
  Piece GetPieceAtPosition(Layer layer, math::Position2d position);

  // Returns all items in disc with center `center` and radius `radius`.
  std::vector<FindPieceResult> DiscFindAll(Layer layer, math::Position2d center,
                                           int radius);

  // Returns all items in diamond with center `center` and radius `radius`.
  std::vector<FindPieceResult> DiamondFindAll(Layer layer,
                                              math::Position2d center,
                                              int radius);

  // Returns all items in rectangle.
  std::vector<FindPieceResult> RectangleFindAll(Layer layer,
                                                math::Position2d corner0,
                                                math::Position2d corner1);

  Piece RandomPieceByGroup(Group group_handle, std::mt19937_64* random) {
    if (group_handle.IsEmpty()) {
      return Piece();
    }
    const auto& group = pieces_group_membership_[group_handle];
    if (group.IsEmpty()) {
      return Piece();
    }
    return group.RandomElement(random);
  }

  // Fires a beam from piece represented by `piece` in the direction
  // the piece is facing. The width of the beam will be `2 * radius + 1` and the
  // length in the middle of the beam will be `length`.
  //
  // In the following daigram
  // the piece '>' is facing east and the `radius` is 2 and the `length` is 7.
  // The '='' represent where the beam will travel.
  //  ======
  //  =======
  //  >=======
  //  =======
  //  ======
  //
  // Here are examples how the beams are blocked. ('.' represents where the
  // beam would have gone.)
  //  ===*..
  //  =======
  //  >===*...
  //  ==*....
  //  ======
  //
  //  ======
  //  =======
  //  >=======
  //  *......
  //  ......
  //
  //
  //  ======
  //  =======
  //  >*......
  //  =======
  //  ======
  //
  void HitBeam(Piece piece, Hit hit, int length, int radius) {
    action_queue_.push_back({piece, ActionHitBeam{hit, length, radius}});
  }

  // Returns all visible sprites at a specified location. The sprites are in
  // render order with the nearest sprite last.
  absl::Span<const SpriteInstance> AllSpriteInstances(math::Position2d pos);

  // Returns all piece handles on all layers at specified position. Empty cells
  // are represented by an empty PieceHandle.
  absl::Span<const Piece> AllPieceHandles(math::Position2d pos) const;

  // Render the grid at `transform` using `grid_view`'s window and
  // SpriteInstance conversion. `output_sprites.size()` must match
  // `grid_view.NumCells()`.
  void Render(math::Transform2d transform, const GridView& grid_view,
              absl::Span<int> output_sprites);

  // Returns a string representation of the rendered scene using the first
  // letter of each sprite name.
  std::string ToString();

  // Returns the transform of piece represented by `piece`.
  math::Transform2d GetPieceTransform(Piece piece) const {
    if (!piece.IsEmpty()) {
      return piece_data_[piece].transform;
    }
    return math::Transform2d{{-1, -1}, math::Orientation2d::kNorth};
  }

  const absl::any& GetUserState(Piece piece) const {
    return piece_data_[piece].user_state;
  }

  void SetUserState(Piece piece, absl::any any) {
    piece_data_[piece].user_state = std::move(any);
  }

  // Returns state of piece.
  State GetState(Piece piece) const {
    return !piece.IsEmpty() ? piece_data_[piece].state : State();
  }

  // Returns the layer of piece.
  Layer GetLayer(Piece piece) const {
    if (!piece.IsEmpty()) {
      return piece_data_[piece].layer;
    }
    return Layer();
  }

  // Returns number of frames the piece has existed as current state if
  // `piece` is not empty. Otherwise returns -1.
  int GetPieceFrames(Piece piece) const {
    return !piece.IsEmpty() ? frame_counter_ - piece_data_[piece].frame_created
                            : -1;
  }

  const World& GetWorld() const { return world_; }

  void Connect(Piece piece1, Piece piece2) {
    action_queue_.push_back({piece1, ActionConnect{piece2}});
  }
  void Disconnect(Piece piece) {
    action_queue_.push_back({piece, ActionDisconnect{}});
  }
  void DisconnectAll(Piece piece) {
    action_queue_.push_back({piece, ActionDisconnectAll{}});
  }

  template <typename Func>
  void VisitConnected(Piece piece, Func visit) const {
    Piece next = piece;
    do {
      visit(next);
      next = piece_data_[next].connect_next;
    } while (!next.IsEmpty() && next != piece);
  }

  template <typename Pred>
  bool AnyInConnected(Piece piece, Pred pred) const {
    Piece next = piece;
    do {
      if (pred(next)) {
        return true;
      }
      next = piece_data_[next].connect_next;
    } while (!next.IsEmpty() && next != piece);
    return false;
  }

 private:
  void RenderTorus(math::Transform2d transform, const GridView& grid_view,
                   absl::Span<int> output_sprites) const;
  void RenderBounded(math::Transform2d transform, const GridView& grid_view,
                     absl::Span<int> output_sprites) const;
  struct PieceData {
    PieceData() = default;  // Required by object pool.
    PieceData(State state, Layer layer, math::Transform2d transform,
              int frame_created)
        : state(state),
          layer(layer),
          transform(transform),
          frame_created(frame_created) {}
    State state;
    Layer layer;
    math::Transform2d transform;
    int frame_created;
    // Circular list of connected entities.
    Piece connect_next;
    Piece connect_prev;
    absl::any user_state;
  };

  struct UpdateInfo {
    Group group = Group();
    int start_frame = 0;
    double probability = 0.0;
  };

  struct SpriteAction {
    CellIndex position;
    SpriteInstance instance;
  };

  struct ActionRotate {
    math::Rotate2d rotate;
  };

  struct ActionConnect {
    Piece piece;
  };

  struct ActionDisconnect {};

  struct ActionDisconnectAll {};

  struct ActionPush {
    math::Orientation2d push_direction;
    Perspective perspective;
  };

  struct ActionTeleport {
    math::Position2d position;
    TeleportOrientation orientation;
  };

  struct ActionSetOrientation {
    math::Orientation2d orientation;
  };

  struct ActionSetState {
    State state;
  };

  struct ActionTeleportToGroup {
    State state;
    Group group;
    TeleportOrientation mode;
  };

  struct ActionHitBeam {
    Hit hit;
    int length;
    int radius;
  };

  using ActionType =
      absl::variant<ActionRotate, ActionPush, ActionTeleport,
                    ActionSetOrientation, ActionSetState, ActionTeleportToGroup,
                    ActionHitBeam, ActionConnect, ActionDisconnect,
                    ActionDisconnectAll>;
  struct Action {
    Piece piece;
    ActionType action_type;
  };
  // All `ProcessAction`s return whether the operation was completed. If the
  // actions is not completed it will be attempted in the next update.

  bool ProcessAction(std::mt19937_64*, Piece piece, ActionRotate action) {
    RotatePieceActual(piece, action.rotate);
    return true;
  }

  bool ProcessAction(std::mt19937_64*, Piece piece, ActionPush action) {
    PushPieceActual(piece, action.push_direction, action.perspective);
    return true;
  }

  bool ProcessAction(std::mt19937_64* random, Piece piece,
                     ActionTeleport action) {
    TeleportPieceActual(random, piece, action.position, action.orientation);
    return true;
  }

  bool ProcessAction(std::mt19937_64*, Piece piece,
                     ActionSetOrientation action) {
    SetPieceOrientationActual(piece, action.orientation);
    return true;
  }

  bool ProcessAction(std::mt19937_64*, Piece piece, ActionSetState action) {
    return SetStateActual(piece, action.state);
  }

  bool ProcessAction(std::mt19937_64* random, Piece piece,
                     ActionTeleportToGroup action) {
    return TeleportToGroupActual(random, piece, action.state, action.group,
                                 action.mode);
  }

  bool ProcessAction(std::mt19937_64* random, Piece piece,
                     ActionHitBeam action) {
    HitBeamActual(/*instigator=*/piece,
                  /*hit=*/action.hit,
                  /*length=*/action.length, /*radius=*/action.radius);
    return true;
  }

  bool ProcessAction(std::mt19937_64*, Piece piece, ActionConnect action) {
    ConnectActual(piece, action.piece);
    return true;
  }

  bool ProcessAction(std::mt19937_64*, Piece piece, ActionDisconnect action) {
    DisconnectActual(piece);
    return true;
  }

  bool ProcessAction(std::mt19937_64*, Piece piece,
                     ActionDisconnectAll action) {
    DisconnectAllActual(piece);
    return true;
  }

  void Repaint();

  void LiftPiece(Piece piece);
  // Returns whether a piece can be placed with offset and layer and blocking
  // piece if encountered.
  std::pair<bool, Piece> CanPlacePiece(Piece piece, math::Vector2d offset,
                                       Layer layer) const;

  void PlacePiece(Piece piece, math::Vector2d offset, Layer layer);

  void RunUpdaters(std::mt19937_64* random);

  void ConnectActual(Piece piece1, Piece piece2);
  void DisconnectActual(Piece piece);
  void DisconnectAllActual(Piece piece);

  void ReleaseInstanceActual(Piece piece);

  void TeleportPieceActual(std::mt19937_64* random, Piece piece,
                           math::Position2d position,
                           TeleportOrientation teleport_orientation);

  void PushPieceActual(Piece piece, math::Orientation2d push_direction,
                       Perspective perspective);

  void RotatePieceActual(Piece piece, math::Rotate2d rotate);
  void SetPieceOrientationActual(Piece piece, math::Orientation2d orientation);

  bool TeleportToGroupActual(std::mt19937_64* random, Piece piece,
                             State target_state, Group target_group,
                             TeleportOrientation teleport_orientation);
  bool SetStateActual(Piece piece, State target_state);
  void UpdateRenderOrientation(const PieceData& piece);

  void TriggerOnEnterCallbacks(Piece piece, math::Position2d pos);
  void TriggerOnLeaveCallbacks(Piece piece, math::Position2d pos);

  void HitBeamActual(Piece instigator, Hit hit, int length, int radius);

  // Hits until all points until blocked from `trans.position` to `length` steps
  // forwards (`trans.orientation` relative.) Returns the hit response of the
  // first position only.
  HitResponse CheckHitLineSegment(Piece instigator, Hit hit,
                                  const World::HitData& hit_data,
                                  math::Transform2d trans, int length);

  // Returns whether the hit was blocked.
  HitResponse DoHit(Piece instigator, Hit hit, const math::Transform2d& trans,
                    const World::HitData& hit_data);

  void SetSprite(CellIndex cell, SpriteInstance sprite);
  void SetSpriteUntilNextUpdate(CellIndex cell, SpriteInstance sprite);

  // Position and layer must be valid and within the grid.
  void FindPiece(math::Position2d position, Layer layer,
                 std::vector<FindPieceResult>* result);

  const World& world_;
  GridShape shape_;

  ShuffledMembership<Group, Piece> pieces_group_membership_;
  FixedHandleMap<Update, UpdateInfo> update_infos_;

  ObjectPool<Piece, PieceData> piece_data_;
  FixedHandleMap<State, std::unique_ptr<StateCallback>> callbacks_;
  FixedHandleMap<CellIndex, Piece> grid_;
  FixedHandleMap<CellIndex, SpriteInstance> grid_render_;
  int frame_counter_ = 0;

  std::vector<Action> action_queue_;
  std::vector<SpriteAction> set_sprite_queue_;

  std::vector<SpriteAction> temp_sprite_locations_;
  std::vector<SpriteAction> temp_sprite_locations_immediate_;
  std::vector<Piece> to_remove_;
  bool in_update_ = false;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_SYSTEM_GRID_WORLD_GRID_H_
