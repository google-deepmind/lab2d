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

#include "dmlab2d/lib/system/grid_world/lua/lua_world.h"

#include <string>
#include <utility>
#include <vector>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "dmlab2d/lib/lua/bind.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/lua/ref.h"
#include "dmlab2d/lib/lua/table_ref.h"
#include "dmlab2d/lib/system/grid_world/handles.h"
#include "dmlab2d/lib/system/grid_world/lua/lua_grid.h"
#include "dmlab2d/lib/system/grid_world/lua/lua_grid_view.h"
#include "dmlab2d/lib/system/grid_world/world.h"

namespace deepmind::lab2d {
namespace {

// Reads object at index into `type` and returns empty string, if successful.
// Otherwise returns an error message.
std::string ReadStateArg(lua_State* L, int index, World::StateArg* type) {
  lua::TableRef table;
  if (!IsFound(lua::Read(L, index, &table))) {
    return "State must be a table";
  }

  if (IsTypeMismatch(table.LookUp("layer", &type->layer))) {
    return "'layer' must be a string.";
  }

  if (IsTypeMismatch(table.LookUp("sprite", &type->sprite))) {
    return "'sprite' must be a string.";
  }

  if (IsTypeMismatch(table.LookUp("groups", &type->group_names))) {
    return "'groups' must be an array of strings.";
  }

  if (IsTypeMismatch(table.LookUp("contact", &type->contact))) {
    return "'contact' must be a string.";
  }

  return "";
}

std::string ReadHitTypeArg(lua_State* L, int index, World::HitArg* type) {
  lua::TableRef table;
  if (!IsFound(lua::Read(L, index, &table))) {
    return "Type must be a table";
  }

  if (IsTypeMismatch(table.LookUp("layer", &type->layer))) {
    return "'layer' must be a string.";
  }

  if (IsTypeMismatch(table.LookUp("sprite", &type->sprite))) {
    return "'sprite' must be a string.";
  }

  return "";
}

lua::ReadResult ReadOrders(const lua::TableRef& orders,
                           std::vector<World::UpdateOrder>* update_orders) {
  for (int i = 1, e = orders.ArraySize(); i <= e; ++i) {
    std::string value;
    World::UpdateOrder update_order;
    if (!IsFound(orders.LookUp(i, &update_order.name))) {
      lua::TableRef name_function;
      auto read_result = orders.LookUp(i, &name_function);
      if (!IsFound(read_result)) {
        return lua::ReadTypeMismatch();
      }
      if (!IsFound(name_function.LookUp("name", &update_order.name))) {
        return lua::ReadTypeMismatch();
      }
      if (!IsFound(name_function.LookUp("func", &update_order.function))) {
        return lua::ReadTypeMismatch();
      }
    }
    update_orders->push_back(std::move(update_order));
  }
  return lua::ReadFound();
}

}  // namespace

LuaWorld::LuaWorld(const World::Args& args) : world_(args) {}

void LuaWorld::Register(lua_State* L) {
  const typename Class::Reg methods[] = {
      {"createGrid", &Class::Member<&LuaWorld::CreateGrid>},
      {"createView", &Class::Member<&LuaWorld::CreateLayerView>},
      {"spriteNames", &Class::Member<&LuaWorld::SpriteNames>},
  };
  Class::Register(L, methods);
  LuaGrid::Register(L);
  LuaGridView::Register(L);
}

lua::NResultsOr LuaWorld::Create(lua_State* L) {
  World::Args args = {};
  lua::TableRef table;
  if (!IsFound(lua::Read(L, 1, &table))) {
    return "Type must be a table";
  }

  if (!IsFound(table.LookUp("renderOrder", &args.render_order))) {
    return "'renderOrder' must be an array of strings";
  }
  lua::TableRef order;
  auto update_order_read = table.LookUp("updateOrder", &order);
  if (IsTypeMismatch(update_order_read) ||
      (IsFound(update_order_read) &&
       IsTypeMismatch(ReadOrders(order, &args.update_order)))) {
    lua_pop(L, 1);
    return "'updateOrder' must be an array of strings or {name = name, "
           "func = function}";
  }
  lua_pop(L, 1);

  if (IsTypeMismatch(table.LookUp("customSprites", &args.custom_sprites))) {
    return "'customSprites' must be an array of strings";
  }

  lua::TableRef state_table;
  if (!IsFound(table.LookUp("states", &state_table)) &&
      !IsFound(table.LookUp("types", &state_table))) {
    return "'states' must be a table of states";
  }
  auto state_keys = state_table.Keys<std::string>();
  for (auto& key : state_keys) {
    World::StateArg type = {};
    state_table.LookUpToStack(key);
    std::string error = ReadStateArg(L, -1, &args.states[key]);
    lua_pop(L, 1);
    if (!error.empty()) {
      return absl::StrCat("states - Error parsing key: '", key, "' - ", error);
    }
  }

  if (IsTypeMismatch(
          table.LookUp("outOfBoundsSprite", &args.out_of_bounds_sprite))) {
    return "'outOfBoundsSprite' must a string";
  }

  if (IsTypeMismatch(
          table.LookUp("outOfViewSprite", &args.out_of_view_sprite))) {
    return "'outOfViewSprite' must a string";
  }

  lua::TableRef hit_table;
  auto hit_result = table.LookUp("hits", &hit_table);
  if (IsTypeMismatch(hit_result)) {
    return "'hits' must be a table of types";
  }
  if (IsFound(hit_result)) {
    auto hit_keys = hit_table.Keys<std::string>();
    for (auto& key : hit_keys) {
      World::HitArg hit = {};
      hit_table.LookUpToStack(key);
      auto error = ReadHitTypeArg(L, -1, &args.hits[key]);
      lua_pop(L, 1);
      if (!error.empty()) {
        return absl::StrCat("hits - Error parsing key: '", key, "' - ", error);
      }
    }
  }

  LuaWorld::CreateObject(L, std::move(args));
  return 1;
}

lua::NResultsOr LuaWorld::CreateGrid(lua_State* L) {
  lua::Ref world_ref;
  CHECK(Read(L, 1, &world_ref)) << "Internal error!";
  return LuaGrid::CreateGrid(L, world_, std::move(world_ref));
}

lua::NResultsOr LuaWorld::CreateLayerView(lua_State* L) {
  return LuaGridView::CreateLayerView(L, world_);
}

int LuaWorld::Module(lua_State* L) {
  Register(L);
  auto table = lua::TableRef::Create(L);
  table.Insert("World", &lua::Bind<LuaWorld::Create>);
  LuaGrid::SubModule(table);
  lua::Push(L, table);
  return 1;
}

lua::NResultsOr LuaWorld::SpriteNames(lua_State* L) {
  std::vector<std::string> sprite_names;
  sprite_names.reserve(world_.sprites().NumElements() * 4 + 1);
  sprite_names.emplace_back();
  for (const auto& name : world_.sprites().Names()) {
    sprite_names.push_back(absl::StrCat(name, ".N"));
    sprite_names.push_back(absl::StrCat(name, ".E"));
    sprite_names.push_back(absl::StrCat(name, ".S"));
    sprite_names.push_back(absl::StrCat(name, ".W"));
  }
  lua::Push(L, sprite_names);
  return 1;
}

}  // namespace deepmind::lab2d
