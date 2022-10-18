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

#include "dmlab2d/lib/system/grid_world/lua/lua_grid.h"

#include <algorithm>
#include <memory>
#include <type_traits>
#include <utility>
#include <vector>

#include "absl/log/log.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "dmlab2d/lib/lua/call.h"
#include "dmlab2d/lib/lua/lua.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/lua/stack_resetter.h"
#include "dmlab2d/lib/lua/table_ref.h"
#include "dmlab2d/lib/system/grid_world/handles.h"
#include "dmlab2d/lib/system/grid_world/lua/lua_handle.h"
#include "dmlab2d/lib/system/grid_world/text_tools.h"
#include "dmlab2d/lib/system/grid_world/world.h"
#include "dmlab2d/lib/system/math/lua/math2d.h"
#include "dmlab2d/lib/system/math/math2d.h"
#include "dmlab2d/lib/system/random/lua/random.h"

namespace deepmind::lab2d {
namespace {

std::vector<Piece> PlaceGrid(const CharMap& characters,
                             absl::string_view layout, math::Position2d offset,
                             Grid* grid) {
  std::vector<Piece> pieces;
  layout = RemoveLeadingAndTrailingNewLines(layout);
  std::vector<absl::string_view> lines = absl::StrSplit(layout, '\n');
  int row = offset.y;
  for (auto line : lines) {
    int col = offset.x;
    for (auto character : line) {
      auto state = characters[character];
      if (!state.IsEmpty()) {
        Piece piece = grid->CreateInstance(
            state, math::Transform2d{{col, row}, math::Orientation2d::kNorth});
        if (!piece.IsEmpty()) {
          pieces.push_back(piece);
        }
      }
      ++col;
    }
    ++row;
  }
  return pieces;
}

class LuaStateCallback : public Grid::StateCallback {
 public:
  LuaStateCallback(lua::TableRef callback_table, const World& world,
                   lua::Ref lua_grid_ref)
      : callback_table_(std::move(callback_table)),
        on_hit_(world.hits().NumElements()),
        on_leave_(world.contacts().NumElements()),
        on_enter_(world.contacts().NumElements()),
        on_update_(world.updates().NumElements()),
        grid_ref_(std::move(lua_grid_ref)) {
    Legacy(world);
    on_add_ = CreateCallback(callback_table_, "onAdd");
    on_remove_ = CreateCallback(callback_table_, "onRemove");
    on_blocked_ = CreateCallback(callback_table_, "onBlocked");
    lua::TableRef on_contact;
    if (IsFound(callback_table_.LookUp("onContact", &on_contact))) {
      for (auto [contact, name] : world.contacts()) {
        lua::TableRef contact_table;
        if (IsFound(on_contact.LookUp(name, &contact_table))) {
          on_leave_[contact] = CreateCallback(contact_table, "leave");
          on_enter_[contact] = CreateCallback(contact_table, "enter");
        }
      }
    }
    lua::TableRef update_table;
    if (IsFound(callback_table_.LookUp("onUpdate", &update_table))) {
      for (auto [update, name] : world.updates()) {
        on_update_[update] =
            CreateCallback(update_table, world.update_functions(update));
      }
    }
    lua::TableRef hits;
    if (IsFound(callback_table_.LookUp("onHit", &hits))) {
      auto on_hit_callback_default = CallbackOrValue(false);
      for (auto [hit, name] : world.hits()) {
        on_hit_[hit] =
            CreateCallbackOrValue(hits, name, on_hit_callback_default);
      }
    }
  }

  void Legacy(const World& world) {
    for (auto [contact, name] : world.contacts()) {
      on_leave_[contact] =
          CreateCallback(callback_table_, absl::StrCat(name, "OnLeave"));
      on_enter_[contact] =
          CreateCallback(callback_table_, absl::StrCat(name, "OnEnter"));
    }
    for (auto [update, name] : world.updates()) {
      on_update_[update] = CreateCallback(
          callback_table_,
          absl::StrCat(world.update_functions(update), "Update"));
    }

    // Modern version of onHit.
    lua::TableRef is_table;
    if (!IsFound(callback_table_.LookUp("onHit", &is_table))) {
      auto on_hit_callback_default = CreateCallbackOrValue(
          callback_table_, "onHit", CallbackOrValue(false));

      for (auto [hit, name] : world.hits()) {
        on_hit_[hit] =
            CreateCallbackOrValue(callback_table_, absl::StrCat(name, "OnHit"),
                                  on_hit_callback_default);
      }
    }
  }

  ~LuaStateCallback() override = default;

  void OnAdd(Piece piece) override { on_add_.Call("onAdd", grid_ref_, piece); }

  void OnRemove(Piece piece) override {
    on_remove_.Call("onRemove", grid_ref_, piece);
  }

  void OnBlocked(Piece piece, Piece blocker) override {
    on_blocked_.Call("onBlocked", grid_ref_, piece, blocker);
  }

  void OnUpdate(Update update, Piece piece, int num_frames_in_state) override {
    on_update_[update].Call("OnUpdate", grid_ref_, piece, num_frames_in_state);
  }

  void OnEnter(Contact contact, Piece piece, Piece instigator) override {
    on_enter_[contact].Call("OnEnter", grid_ref_, piece, instigator);
  }

  void OnLeave(Contact contact, Piece piece, Piece instigator) override {
    on_leave_[contact].Call("OnLeave", grid_ref_, piece, instigator);
  }

  Grid::HitResponse OnHit(Hit hit, Piece piece, Piece instigator) override {
    return on_hit_[hit].Call("OnHit", grid_ref_, piece, instigator)
               ? Grid::HitResponse::kBlocked
               : Grid::HitResponse::kContinue;
  }

 private:
  struct Callback {
   public:
    Callback() = default;
    explicit Callback(lua::Ref ref) : func_ref_(std::move(ref)) {}
    template <typename... Args>
    void Call(absl::string_view func_name, Args&&... args) {
      if (!func_ref_.is_unbound()) {
        lua::StackResetter resetter(func_ref_.LuaState());
        lua::NResultsOr result = func_ref_.Call(args...);
        CHECK(result.ok()) << "Callback error while calling '" << func_name
                           << "': " << result.error();
      }
    }

   private:
    lua::Ref func_ref_;
  };

  static Callback CreateCallback(const lua::TableRef& callback_table,
                                 absl::string_view func_name) {
    lua::StackResetter resetter(callback_table.LuaState());
    callback_table.LookUpToStack(func_name);
    lua_State* L = callback_table.LuaState();
    lua::Ref ref;
    switch (lua_type(L, -1)) {
      case LUA_TNONE:
      case LUA_TNIL:
        return Callback(std::move(ref));
      case LUA_TTABLE:
      case LUA_TFUNCTION:
      case LUA_TUSERDATA:
        CHECK(IsFound(lua::Read(L, -1, &ref)))
            << "Invalid callback:" << func_name << " " << lua::ToString(L, -1);
        return Callback(std::move(ref));
      default:
        LOG(FATAL) << func_name << " - Invalid type:"
                   << " " << lua::ToString(L, -1);
    }
  }

  class CallbackOrValue {
   public:
    explicit CallbackOrValue() : value_(false) {}
    explicit CallbackOrValue(lua::Ref ref)
        : func_ref_(std::move(ref)), value_(false) {}
    explicit CallbackOrValue(bool value) : value_(value) {}
    template <typename... Args>
    bool Call(absl::string_view func_name, Args&&... args) {
      if (!func_ref_.is_unbound()) {
        lua_State* L = func_ref_.LuaState();
        lua::NResultsOr result = func_ref_.Call(args...);
        CHECK(result.ok()) << "Callback error while calling '" << func_name
                           << "': " << result.error();
        bool out_value = value_;
        if (result.n_results() > 0) {
          CHECK(!IsTypeMismatch(lua::Read(L, -1, &out_value)))
              << "Callback error while calling '" << func_name << "': "
              << "return value type mismatch! " << lua::ToString(L, -1);
          lua_settop(L, 0);
        }
        return out_value;
      } else {
        return value_;
      }
    }

   private:
    lua::Ref func_ref_;
    bool value_;
  };

  static CallbackOrValue CreateCallbackOrValue(
      const lua::TableRef& callback_table, absl::string_view func_name,
      CallbackOrValue def) {
    lua::StackResetter resetter(callback_table.LuaState());
    callback_table.LookUpToStack(func_name);
    lua_State* L = callback_table.LuaState();
    lua::Ref ref;
    switch (lua_type(L, -1)) {
      case LUA_TNONE:
      case LUA_TNIL:
        return def;
      case LUA_TBOOLEAN:
        return CallbackOrValue(lua_toboolean(L, 1));
      case LUA_TTABLE:
      case LUA_TFUNCTION:
      case LUA_TUSERDATA: {
        CHECK(IsFound(lua::Read(L, -1, &ref)))
            << "Invalid callback:" << func_name << " " << lua::ToString(L, -1);
        return CallbackOrValue(std::move(ref));
      }
      default:
        LOG(FATAL) << func_name << " - Invalid type:"
                   << " " << lua::ToString(L, -1);
    }
  }

  lua::TableRef callback_table_;
  FixedHandleMap<Hit, CallbackOrValue> on_hit_;
  FixedHandleMap<Contact, Callback> on_leave_;
  FixedHandleMap<Contact, Callback> on_enter_;
  FixedHandleMap<Update, Callback> on_update_;
  Callback on_add_;
  Callback on_remove_;
  Callback on_blocked_;
  lua::Ref grid_ref_;
};

void PushFindPieceResults(lua_State* L,
                          const std::vector<Grid::FindPieceResult>& result) {
  lua_createtable(L, 0, result.size());
  for (const auto& hit : result) {
    Push(L, hit.piece);
    Push(L, hit.position);
    lua_settable(L, -3);
  }
}

}  // namespace

void LuaGrid::SubModule(lua::TableRef module) {
  auto teleport_orientation = module.CreateSubTable("TELEPORT_ORIENTATION");
  teleport_orientation.Insert(
      "MATCH_TARGET",
      static_cast<int>(Grid::TeleportOrientation::kMatchTarget));
  teleport_orientation.Insert(
      "KEEP_ORIGINAL",
      static_cast<int>(Grid::TeleportOrientation::kKeepOriginal));
  teleport_orientation.Insert(
      "PICK_RANDOM", static_cast<int>(Grid::TeleportOrientation::kPickRandom));
  auto topology = module.CreateSubTable("TOPOLOGY");
  topology.Insert("BOUNDED", static_cast<int>(GridShape::Topology::kBounded));
  topology.Insert("TORUS", static_cast<int>(GridShape::Topology::kTorus));
}

lua::NResultsOr LuaGrid::CreateGrid(lua_State* L, const World& world,
                                    lua::Ref world_ref) {
  lua::TableRef table;
  if (!IsFound(lua::Read(L, 2, &table))) {
    return "Must supply a table as first argument to grid construction!";
  }

  absl::string_view layout;
  if (IsTypeMismatch(table.LookUp("layout", &layout))) {
    return "layout must be a string.";
  }

  math::Size2d grid_size_override = {};
  if (IsTypeMismatch(table.LookUp("size", &grid_size_override))) {
    table.LookUpToStack("size");
    return absl::StrFormat("`size`(%s) {width = positive, height = positive}",
                           lua::ToString(L, -1));
  }

  math::Size2d grid_size = GetSize2dOfText(layout);
  grid_size.width = std::max(grid_size_override.width, grid_size.width);
  grid_size.height = std::max(grid_size_override.height, grid_size.height);

  if (grid_size.height == 0 || grid_size.width == 0) {
    return "Must supply string value for 'layout' or a positive size value for "
           "`size` {width = <positive>, height = <positive>}!";
  }

  absl::flat_hash_map<absl::string_view, lua::TableRef> state_callbacks;
  if (IsTypeMismatch(table.LookUp("stateCallbacks", &state_callbacks))) {
    return "Must supply state map value for 'stateCallbacks'!";
  }

  if (state_callbacks.empty() &&
      IsTypeMismatch(table.LookUp("typeCallbacks", &state_callbacks))) {
    return "Must supply state map value for 'stateCallbacks'!";
  }

  int topology_int = static_cast<int>(GridShape::Topology::kBounded);
  if (IsTypeMismatch(table.LookUp("topology", &topology_int))) {
    // Set to invalid topology for error message.
    topology_int = -1;
  }

  switch (topology_int) {
    case static_cast<int>(GridShape::Topology::kBounded):
    case static_cast<int>(GridShape::Topology::kTorus):
      break;
    default:
      return "Invalid topology must be one of "
             "grid_world.TOPOLOGY.TORUS "
             "grid_world.TOPOLOGY.BOUNDED.";
  }

  auto topology = static_cast<GridShape::Topology>(topology_int);

  LuaGrid* lua_grid =
      CreateObject(L, Grid(world, grid_size, topology), std::move(world_ref));
  Grid* grid = lua_grid->GetMutableGrid();
  lua::Ref lua_grid_ref;
  CHECK(IsFound(Read(L, -1, &lua_grid_ref))) << "Internal logic error!";
  for (auto& [state_name, table_ref] : state_callbacks) {
    State state = grid->GetWorld().states().ToHandle(state_name);
    if (!state.IsEmpty()) {
      grid->SetCallback(
          state, std::make_unique<LuaStateCallback>(
                     std::move(table_ref), grid->GetWorld(), lua_grid_ref));
    }
  }

  if (!layout.empty()) {
    absl::flat_hash_map<absl::string_view, absl::string_view>
        character_to_state_name;
    if (!IsFound(table.LookUp("stateMap", &character_to_state_name)) &&
        !IsFound(table.LookUp("typeMap", &character_to_state_name))) {
      return "When specifying `layout` you must also supply state map value "
             "for "
             "'stateMap'!";
    }
    CharMap character_to_state = {};
    for (const auto& [key, state_name] : character_to_state_name) {
      if (key.size() != 1) {
        return absl::StrCat("Key must be a single character found: '", key,
                            "'");
      }
      auto state = world.states().ToHandle(state_name);
      if (state.IsEmpty()) {
        return absl::StrCat("Cannot find state: '", state_name, "'");
      }
      character_to_state[key[0]] = state;
    }

    auto pieces = PlaceGrid(character_to_state, layout, {0, 0},
                            lua_grid->GetMutableGrid());
    lua::Push(L, pieces);
    return 2;
  }
  return 1;
}

void LuaGrid::Register(lua_State* L) {
  const Class::Reg methods[] = {
      {"__tostring", &Class::Member<&LuaGrid::ToString>},
      {"transform", &Class::Member<&LuaGrid::Transform>},
      {"position", &Class::Member<&LuaGrid::Position>},
      {"toRelativeDirection", &Class::Member<&LuaGrid::ToRelativeDirection>},
      {"toRelativePosition", &Class::Member<&LuaGrid::ToRelativePosition>},
      {"toAbsoluteDirection", &Class::Member<&LuaGrid::ToAbsoluteDirection>},
      {"toAbsolutePosition", &Class::Member<&LuaGrid::ToAbsolutePosition>},
      {"typeName", &Class::Member<&LuaGrid::GetState>},
      {"state", &Class::Member<&LuaGrid::GetState>},
      {"layer", &Class::Member<&LuaGrid::GetLayer>},
      {"userState", &Class::Member<&LuaGrid::GetUserState>},
      {"createLayout", &Class::Member<&LuaGrid::CreateLayout>},
      {"createPiece", &Class::Member<&LuaGrid::CreatePiece>},
      {"removePiece", &Class::Member<&LuaGrid::RemovePiece>},
      {"frames", &Class::Member<&LuaGrid::Frames>},
      {"rayCast", &Class::Member<&LuaGrid::RayCast>},
      {"rayCastDirection", &Class::Member<&LuaGrid::RayCastDirection>},
      {"queryPosition", &Class::Member<&LuaGrid::QueryPosition>},
      {"queryRectangle", &Class::Member<&LuaGrid::QueryRectangle>},
      {"queryDiamond", &Class::Member<&LuaGrid::QueryDiamond>},
      {"queryDisc", &Class::Member<&LuaGrid::QueryDisc>},
      {"groupCount", &Class::Member<&LuaGrid::GroupCount>},
      {"groupShuffled", &Class::Member<&LuaGrid::GroupShuffled>},
      {"groupShuffledWithCount",
       &Class::Member<&LuaGrid::GroupShuffledWithCount>},
      {"groupShuffledWithProbability",
       &Class::Member<&LuaGrid::GroupShuffledWithProbability>},
      {"groupRandom", &Class::Member<&LuaGrid::GroupRandom>},
      {"setUpdater", &Class::Member<&LuaGrid::SetUpdater>},
      {"moveAbs", &Class::Member<&LuaGrid::PushGridRelative>},
      {"moveRel", &Class::Member<&LuaGrid::PushPieceRelative>},
      {"teleport", &Class::Member<&LuaGrid::TeleportPiece>},
      {"turn", &Class::Member<&LuaGrid::RotatePiece>},
      {"setOrientation", &Class::Member<&LuaGrid::SetPieceOrientation>},
      {"hitBeam", &Class::Member<&LuaGrid::HitBeam>},
      {"setType", &Class::Member<&LuaGrid::SetState>},  // Legacy support.
      {"setState", &Class::Member<&LuaGrid::SetState>},
      {"setUserState", &Class::Member<&LuaGrid::SetUserState>},
      {"teleportToGroup", &Class::Member<&LuaGrid::TeleportToGroup>},
      {"update", &Class::Member<&LuaGrid::DoUpdate>},
      {"destroy", &Class::Member<&LuaGrid::Destroy>},
      {"connect", &Class::Member<&LuaGrid::Connect>},
      {"disconnect", &Class::Member<&LuaGrid::Disconnect>},
      {"disconnectAll", &Class::Member<&LuaGrid::DisconnectAll>},
  };
  Class::Register(L, methods);
}

LuaGrid::LuaGrid(Grid grid, lua::Ref world_ref)
    : grid_(std::move(grid)), world_ref_(std::move(world_ref)) {}

lua::NResultsOr LuaGrid::Destroy(lua_State* L) {
  grid_.reset();
  return 0;
}

lua::NResultsOr LuaGrid::CreateLayout(lua_State* L) {
  lua::TableRef table;
  Grid* grid = GetMutableGrid();
  const World& world = grid->GetWorld();

  if (!IsFound(lua::Read(L, 2, &table))) {
    return "Must supply a table as first argument to createLayout!";
  }

  absl::string_view layout;
  if (!IsFound(table.LookUp("layout", &layout))) {
    return "Must supply string value for 'layout'!";
  }

  absl::flat_hash_map<absl::string_view, absl::string_view>
      character_to_state_name;
  if (!IsFound(table.LookUp("stateMap", &character_to_state_name)) &&
      !IsFound(table.LookUp("typeMap", &character_to_state_name))) {
    return "You must also supply state map value for 'stateMap'!";
  }
  CharMap character_to_state = {};
  for (const auto& [key, state_name] : character_to_state_name) {
    if (key.size() != 1) {
      return absl::StrCat("Key must be a single character found: '", key, "'");
    }
    auto state = world.states().ToHandle(state_name);
    if (state.IsEmpty()) {
      return absl::StrCat("Cannot find state: '", state_name, "'");
    }
    character_to_state[key[0]] = state;
  }

  math::Position2d offset{};
  if (IsTypeMismatch(table.LookUp("offset", &offset))) {
    return "Offset must be a pair of integers";
  }

  auto pieces = PlaceGrid(character_to_state, layout, offset, grid);
  lua::Push(L, pieces);
  return 1;
}

lua::NResultsOr LuaGrid::Transform(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece)) || piece.IsEmpty()) {
    return "Arg 1 must be valid piece!";
  }

  Push(L, grid_->GetPieceTransform(piece));
  return 1;
}

lua::NResultsOr LuaGrid::Position(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece)) || piece.IsEmpty()) {
    return "Arg 1 must be valid piece!";
  }
  Push(L, grid_->GetPieceTransform(piece).position);
  return 1;
}

lua::NResultsOr LuaGrid::GetUserState(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece)) || piece.IsEmpty()) {
    return "Arg 1 must be valid piece!";
  }
  const auto& user_state = grid_->GetUserState(piece);
  if (user_state.has_value()) {
    Push(L, absl::any_cast<lua::Ref>(user_state));
  } else {
    lua_pushnil(L);
  }
  return 1;
}

lua::NResultsOr LuaGrid::SetUserState(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece)) || piece.IsEmpty()) {
    return "Arg 1 must be valid piece!";
  }
  if (lua_isnoneornil(L, 3)) {
    grid_->SetUserState(piece, absl::any());
    return 0;
  }
  lua::Ref ref;
  if (!IsFound(Read(L, 3, &ref))) {
    return "Arg 2 must be a value!";
  }
  grid_->SetUserState(piece, ref);
  return 0;
}

lua::NResultsOr LuaGrid::GetState(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece)) || piece.IsEmpty()) {
    return "Arg 1 must be valid piece!";
  }

  lua::Push(L, grid_->GetWorld().states().ToName(grid_->GetState(piece)));
  return 1;
}

lua::NResultsOr LuaGrid::GetLayer(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece)) || piece.IsEmpty()) {
    return "Arg 1 must be valid piece!";
  }
  auto layer = grid_->GetLayer(piece);
  if (!layer.IsEmpty()) {
    lua::Push(L, grid_->GetWorld().layers().ToName(layer));
  } else {
    lua_pushnil(L);
  }
  return 1;
}

lua::NResultsOr LuaGrid::SetState(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece)) || piece.IsEmpty()) {
    return "Arg 1 must be piece!";
  }
  absl::string_view state_name;
  if (!IsFound(lua::Read(L, 3, &state_name))) {
    return "Arg 2 must be a valid state!";
  }
  State state = grid_->GetWorld().states().ToHandle(state_name);
  if (state.IsEmpty() && !state_name.empty()) {
    return "Arg 2 must be a valid state name or empty.!";
  }
  if (piece.IsEmpty()) {
    return 0;
  }
  grid_->SetState(piece, state);
  return 0;
}

lua::NResultsOr LuaGrid::TeleportToGroup(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece)) || piece.IsEmpty()) {
    return "Arg 1 must be piece!";
  }

  absl::string_view group_name;
  if (!IsFound(lua::Read(L, 3, &group_name))) {
    return "Arg 2 must be a group name!";
  }
  auto group = grid_->GetWorld().groups().ToHandle(group_name);
  if (group.IsEmpty()) {
    return absl::StrCat("Arg 2 must be a valid group name! provided: '",
                        group_name, "'");
  }

  absl::string_view state_name;
  if (IsTypeMismatch(lua::Read(L, 4, &state_name))) {
    return "Arg 3 must be state name!";
  }

  State state;
  if (!state_name.empty()) {
    state = grid_->GetWorld().states().ToHandle(state_name);
    if (state.IsEmpty()) {
      return absl::StrCat("Arg 3 must be a valid state name! provided: '",
                          state_name, "'");
    }
  }
  int teleport_orientation =
      static_cast<int>(Grid::TeleportOrientation::kPickRandom);
  if (IsTypeMismatch(lua::Read(L, 5, &teleport_orientation)) ||
      teleport_orientation < 0 || teleport_orientation > 2) {
    return "Arg 4 must be omitted or one of "
           "grid_world.TELEPORT_ORIENTATION.MATCH_TARGET "
           "grid_world.TELEPORT_ORIENTATION.KEEP_ORIGINAL "
           "grid_world.TELEPORT_ORIENTATION.PICK_RANDOM";
  }
  Grid::TeleportOrientation mode =
      static_cast<Grid::TeleportOrientation>(teleport_orientation);
  grid_->TeleportToGroup(piece, group, state, mode);
  return 0;
}

lua::NResultsOr LuaGrid::CreatePiece(lua_State* L) {
  absl::string_view state_name;
  if (!IsFound(lua::Read(L, 2, &state_name))) {
    return "Arg 1 must be state!";
  }

  State state = grid_->GetWorld().states().ToHandle(state_name);
  if (state.IsEmpty()) {
    return absl::StrCat("Not a valid state: ''", state_name, "''");
  }

  math::Transform2d transform;
  if (!IsFound(Read(L, 3, &transform))) {
    return "Arg 2 must be a valid transform! E.g {pos = {10, 20}, orientation "
           "= 'N'}";
  }

  Push(L, grid_->CreateInstance(state, transform));
  return 1;
}

lua::NResultsOr LuaGrid::RemovePiece(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece))) {
    return "Arg 1 must be piece!";
  }
  if (!piece.IsEmpty()) {
    grid_->ReleaseInstance(piece);
  }
  return 0;
}

lua::NResultsOr LuaGrid::Frames(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece)) || piece.IsEmpty()) {
    return "Arg 1 must be a valid piece!";
  }

  lua::Push(L, grid_->GetPieceFrames(piece));
  return 1;
}

lua::NResultsOr LuaGrid::TeleportPiece(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece))) {
    return "Arg 1 must be piece!";
  }

  math::Position2d position;
  if (!IsFound(Read(L, 3, &position))) {
    return "Arg 2 must be an absolute grid position {<x>, <y>}";
  }

  int teleport_orientation =
      static_cast<int>(Grid::TeleportOrientation::kPickRandom);
  if (IsTypeMismatch(lua::Read(L, 4, &teleport_orientation)) ||
      teleport_orientation < 0 || teleport_orientation > 2) {
    return "Arg 3 must be omitted or one of "
           "grid_world.TELEPORT_ORIENTATION.MATCH_TARGET "
           "grid_world.TELEPORT_ORIENTATION.KEEP_ORIGINAL "
           "grid_world.TELEPORT_ORIENTATION.PICK_RANDOM";
  }
  grid_->TeleportPiece(
      piece, position,
      static_cast<Grid::TeleportOrientation>(teleport_orientation));
  return 0;
}

lua::NResultsOr LuaGrid::RotatePiece(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece))) {
    return "Arg 1 must be piece!";
  }

  math::Rotate2d turn = math::Rotate2d::k0;
  if (!IsFound(Read(L, 3, &turn))) {
    return "Arg 2 must be one of: 0, 1, 2, or 3.";
  }
  grid_->RotatePiece(piece, turn);
  return 0;
}

lua::NResultsOr LuaGrid::SetPieceOrientation(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece))) {
    return "Arg 1 must be piece!";
  }

  math::Orientation2d orientation;
  if (!IsFound(Read(L, 3, &orientation))) {
    return "Arg 2 must be one of 'N', 'E', 'S' or 'W'!";
  }
  grid_->SetPieceOrientation(piece, orientation);
  return 0;
}

lua::NResultsOr LuaGrid::PushGridRelative(lua_State* L) {
  return PushPiece(L, Grid::Perspective::kGrid);
}
lua::NResultsOr LuaGrid::PushPieceRelative(lua_State* L) {
  return PushPiece(L, Grid::Perspective::kPiece);
}

lua::NResultsOr LuaGrid::PushPiece(lua_State* L,
                                   Grid::Perspective perspective) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece))) {
    return "Arg 1 must be a piece!";
  }

  math::Orientation2d orientation;
  if (!IsFound(Read(L, 3, &orientation))) {
    return "Arg 2 must be one of 'N', 'E', 'S' or 'W'!";
  }
  grid_->PushPiece(piece, orientation, perspective);
  return 0;
}

lua::NResultsOr LuaGrid::ToRelativeDirection(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece))) {
    return "Arg 1 must be a piece!";
  }
  math::Transform2d transform = grid_->GetPieceTransform(piece);
  math::Vector2d direction;

  if (!IsFound(Read(L, 3, &direction))) {
    return "Arg 2 must be a valid direction vector.";
  }
  Push(L, transform.ToRelativeSpace(direction));
  return 1;
}

lua::NResultsOr LuaGrid::ToAbsoluteDirection(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece))) {
    return "Arg 1 must be a piece!";
  }
  math::Transform2d transform = grid_->GetPieceTransform(piece);
  math::Vector2d direction;

  if (!IsFound(Read(L, 3, &direction))) {
    return "Arg 2 must be a valid direction vector.";
  }
  Push(L, transform.ToAbsoluteSpace(direction));
  return 1;
}

lua::NResultsOr LuaGrid::ToRelativePosition(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece))) {
    return "Arg 1 must be a piece!";
  }
  math::Transform2d transform = grid_->GetPieceTransform(piece);
  math::Position2d pos;
  if (!IsFound(Read(L, 3, &pos))) {
    return "Arg 2 must be a valid position.";
  }

  Push(L, transform.ToRelativeSpace(pos));
  return 1;
}

lua::NResultsOr LuaGrid::ToAbsolutePosition(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece))) {
    return "Arg 1 must be a piece!";
  }
  math::Transform2d transform = grid_->GetPieceTransform(piece);
  math::Position2d pos;
  if (!IsFound(Read(L, 3, &pos))) {
    return "Arg 2 must be a valid position.";
  }

  Push(L, transform.ToAbsoluteSpace(pos));
  return 1;
}

// Returns 3 values.
// Whether the ray hit something.
// Piece it hit and location.
lua::NResultsOr LuaGrid::RayCast(lua_State* L) {
  absl::string_view layer_string;
  if (!IsFound(lua::Read(L, 2, &layer_string))) {
    return "Arg 1 must be a layer name";
  }
  Layer layer = grid_->GetWorld().layers().ToHandle(layer_string);
  math::Position2d start;
  if (!IsFound(Read(L, 3, &start))) {
    return "Arg 2 must be a valid position.";
  }
  math::Position2d end;
  if (!IsFound(Read(L, 4, &end))) {
    return "Arg 3 must be a valid position.";
  }

  auto result = grid_->RayCast(layer, start, end);
  lua::Push(L, result.has_value());
  if (result.has_value()) {
    Push(L, result->piece);
    Push(L, result->position);
  } else {
    lua_pushnil(L);
    Push(L, end);
  }
  return 3;
}

// Returns 3 values.
// Whether the ray hit something.
// Piece it hit and location.
lua::NResultsOr LuaGrid::RayCastDirection(lua_State* L) {
  absl::string_view layer_string;
  if (!IsFound(lua::Read(L, 2, &layer_string))) {
    return "Arg 1 must be a layer name";
  }
  Layer layer = grid_->GetWorld().layers().ToHandle(layer_string);
  math::Position2d start;
  if (!IsFound(Read(L, 3, &start))) {
    return "Arg 2 must be a valid start position.";
  }
  math::Vector2d direction;
  if (!IsFound(Read(L, 4, &direction))) {
    return "Arg 3 must be a valid direction vector.";
  }

  auto result = grid_->RayCastDirection(layer, start, direction);
  lua::Push(L, result.has_value());
  if (result.has_value()) {
    Push(L, result->piece);
    Push(L, result->position - start);
  } else {
    lua_pushnil(L);
    Push(L, direction);
  }
  return 3;
}

lua::NResultsOr LuaGrid::QueryPosition(lua_State* L) {
  absl::string_view layer_string;
  if (!IsFound(lua::Read(L, 2, &layer_string))) {
    return "Arg 1 must be a layer name";
  }
  Layer layer = grid_->GetWorld().layers().ToHandle(layer_string);
  math::Position2d position;
  if (!IsFound(Read(L, 3, &position))) {
    return "Arg 2 must be a valid position.";
  }
  Piece result = grid_->GetPieceAtPosition(layer, position);
  if (result.IsEmpty()) {
    return 0;
  }
  Push(L, result);
  return 1;
}

lua::NResultsOr LuaGrid::QueryRectangle(lua_State* L) {
  absl::string_view layer_string;
  if (!IsFound(lua::Read(L, 2, &layer_string))) {
    return "Arg 1 must be a layer name";
  }
  Layer layer = grid_->GetWorld().layers().ToHandle(layer_string);
  math::Position2d position0;
  if (!IsFound(Read(L, 3, &position0))) {
    return "Arg 2 must be a valid position.";
  }
  math::Position2d position1;
  if (!IsFound(Read(L, 4, &position1))) {
    return "Arg 3 must be a valid position.";
  }
  PushFindPieceResults(L, grid_->RectangleFindAll(layer, position0, position1));
  return 1;
}

lua::NResultsOr LuaGrid::QueryDiamond(lua_State* L) {
  absl::string_view layer_string;
  if (!IsFound(lua::Read(L, 2, &layer_string))) {
    return "Arg 1 must be a layer name";
  }
  Layer layer = grid_->GetWorld().layers().ToHandle(layer_string);
  math::Position2d position;
  if (!IsFound(Read(L, 3, &position))) {
    return "Arg 2 must be a valid position.";
  }
  int radius;
  if (!IsFound(lua::Read(L, 4, &radius)) || radius < 0) {
    return "Arg 3 must be a non-negative radius.";
  }
  PushFindPieceResults(L, grid_->DiamondFindAll(layer, position, radius));
  return 1;
}

lua::NResultsOr LuaGrid::QueryDisc(lua_State* L) {
  absl::string_view layer_string;
  if (!IsFound(lua::Read(L, 2, &layer_string))) {
    return "Arg 1 must be a layer name";
  }
  Layer layer = grid_->GetWorld().layers().ToHandle(layer_string);
  math::Position2d position;
  if (!IsFound(Read(L, 3, &position))) {
    return "Arg 2 must be a valid position.";
  }
  int radius;
  if (!IsFound(lua::Read(L, 4, &radius)) || radius < 0) {
    return "Arg 3 must be a non-negative radius.";
  }

  PushFindPieceResults(L, grid_->DiscFindAll(layer, position, radius));
  return 1;
}

lua::NResultsOr LuaGrid::GroupCount(lua_State* L) {
  absl::string_view group_name;
  if (!IsFound(lua::Read(L, 2, &group_name))) {
    return "Arg 2 must be a group name.";
  }
  Group group = grid_->GetWorld().groups().ToHandle(group_name);
  if (group.IsEmpty()) {
    lua::Push(L, 0);
    return 1;
  }

  lua::Push(L, grid_->PieceCountByGroup(group));
  return 1;
}

lua::NResultsOr LuaGrid::GroupRandom(lua_State* L) {
  auto* random = LuaRandom::ReadObject(L, 2);
  if (!random) {
    return "Arg 1 must be a random number generator.";
  }
  absl::string_view group_name;
  if (!IsFound(lua::Read(L, 3, &group_name))) {
    return "Arg 2 must be a group name.";
  }
  Group group = grid_->GetWorld().groups().ToHandle(group_name);
  if (group.IsEmpty()) {
    return absl::StrCat("Arg 2 must be a *valid* group name. '", group_name,
                        "'");
  }

  Push(L, grid_->RandomPieceByGroup(group, random->GetPrbg()));
  return 1;
}

lua::NResultsOr LuaGrid::GroupShuffled(lua_State* L) {
  auto* random = LuaRandom::ReadObject(L, 2);
  if (!random) {
    return "Arg 1 must be a random number generator.";
  }
  absl::string_view group_name;
  if (!IsFound(lua::Read(L, 3, &group_name))) {
    return "Arg 2 must be a group name.";
  }
  Group group = grid_->GetWorld().groups().ToHandle(group_name);
  if (group.IsEmpty()) {
    return absl::StrCat("Arg 2 must be a *valid* group name. '", group_name,
                        "'");
  }

  const auto& pieces = grid_->PiecesByGroupShuffled(group, random->GetPrbg());
  lua::Push(L, pieces);
  return 1;
}

lua::NResultsOr LuaGrid::HitBeam(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece))) {
    return "Arg 1 must be piece!";
  }

  absl::string_view hit_name;
  if (!IsFound(lua::Read(L, 3, &hit_name))) {
    return "Arg 2 must be a hit name";
  }

  Hit hit = grid_->GetWorld().hits().ToHandle(hit_name);
  if (hit.IsEmpty()) {
    return absl::StrCat("Arg 2 is not a hit name: '", hit_name, "'");
  }

  int length = -1;
  if (!IsFound(lua::Read(L, 4, &length))) {
    return "Arg 3 must be hit distance";
  }

  int radius = 0;
  if (!IsFound(lua::Read(L, 5, &radius))) {
    return "Arg 4 must be hit radius";
  }
  radius = std::min(radius, length);
  grid_->HitBeam(piece, hit, length, radius);
  return 0;
}

lua::NResultsOr LuaGrid::GroupShuffledWithCount(lua_State* L) {
  auto* random = LuaRandom::ReadObject(L, 2);
  if (!random) {
    return "Arg 1 must be a random number generator.";
  }
  absl::string_view group_name;
  if (!IsFound(lua::Read(L, 3, &group_name))) {
    return "Arg 2 must be a group name.";
  }
  Group group = grid_->GetWorld().groups().ToHandle(group_name);
  if (group.IsEmpty()) {
    return absl::StrCat("Arg 2 must be a *valid* group name. '", group_name,
                        "'");
  }

  int count = 0;
  if (!IsFound(lua::Read(L, 4, &count))) {
    return "Arg 3 must be a max count of elements to return.";
  }

  const auto& pieces =
      grid_->PiecesByGroupShuffledWithMaxCount(group, count, random->GetPrbg());
  lua::Push(L, pieces);
  return 1;
}

lua::NResultsOr LuaGrid::GroupShuffledWithProbability(lua_State* L) {
  auto* random = LuaRandom::ReadObject(L, 2);
  if (!random) {
    return "Arg 1 must be a random number generator.";
  }
  absl::string_view group_name;
  if (!IsFound(lua::Read(L, 3, &group_name))) {
    return "Arg 2 must be a group name.";
  }
  Group group = grid_->GetWorld().groups().ToHandle(group_name);
  if (group.IsEmpty()) {
    return absl::StrCat("Arg 2 must be a *valid* group name. '", group_name,
                        "'");
  }

  double probability = 1.0;
  if (!IsFound(lua::Read(L, 4, &probability))) {
    return "Arg 3 must be a probability.";
  }

  const auto& pieces = grid_->PiecesByGroupShuffledWithProbabilty(
      group, probability, random->GetPrbg());
  lua::Push(L, pieces);
  return 1;
}

lua::NResultsOr LuaGrid::DoUpdate(lua_State* L) {
  auto* random = LuaRandom::ReadObject(L, 2);
  if (!random) {
    return "Arg 1 must be a random number generator.";
  }
  int flush_count = 128;
  if (IsTypeMismatch(lua::Read(L, 3, &flush_count)) || flush_count < 0) {
    return "Arg 2 (flush_count) must be an integer >= 0";
  }
  grid_->DoUpdate(random->GetPrbg(), flush_count);
  return 0;
}

lua::NResultsOr LuaGrid::ToString(lua_State* L) {
  lua::Push(L, grid_->ToString());
  return 1;
}

lua::NResultsOr LuaGrid::SetUpdater(lua_State* L) {
  lua::TableRef table;
  if (!IsFound(Read(L, 2, &table))) {
    return "Must be called with table.";
  }

  absl::string_view update_name;
  if (!IsFound(table.LookUp("update", &update_name))) {
    return "'update' must be a string";
  }

  auto update = grid_->GetWorld().updates().ToHandle(update_name);
  if (update.IsEmpty()) {
    return absl::StrCat("'update' invalid update name: ", update_name);
  }

  absl::string_view group_name;
  if (!IsFound(table.LookUp("group", &group_name))) {
    return "'group' must be a string";
  }

  auto group = grid_->GetWorld().groups().ToHandle(group_name);
  if (group.IsEmpty()) {
    return absl::StrCat("'group' invalid group name: ", group_name);
  }

  double probability = 1.0;
  if (IsTypeMismatch(table.LookUp("probability", &probability)) ||
      std::isnan(probability)) {
    return "'probability' must be a number";
  }

  int start_frame = 0;
  if (IsTypeMismatch(table.LookUp("startFrame", &start_frame))) {
    return "'start_frame' must be a number";
  }

  grid_->SetUpdateInfo(update, group, probability, start_frame);
  return 0;
}

lua::NResultsOr LuaGrid::Connect(lua_State* L) {
  Piece piece1;
  Piece piece2;
  if (!IsFound(Read(L, 2, &piece1))) {
    return "Arg 1 must be piece!";
  }
  if (!IsFound(Read(L, 3, &piece2))) {
    return "Arg 2 must be piece!";
  }
  grid_->Connect(piece1, piece2);
  return 0;
}
lua::NResultsOr LuaGrid::Disconnect(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece))) {
    return "Arg 1 must be piece!";
  }
  grid_->Disconnect(piece);
  return 0;
}
lua::NResultsOr LuaGrid::DisconnectAll(lua_State* L) {
  Piece piece;
  if (!IsFound(Read(L, 2, &piece))) {
    return "Arg 1 must be piece!";
  }
  grid_->DisconnectAll(piece);
  return 0;
}

}  // namespace deepmind::lab2d
