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

#include "dmlab2d/system/math/lua/math2d.h"

#include <array>

#include "absl/strings/string_view.h"
#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/push.h"
#include "dmlab2d/lua/read.h"
#include "dmlab2d/lua/table_ref.h"
#include "dmlab2d/system/math/math2d.h"

namespace deepmind::lab2d::math {

void Push(lua_State* L, Rotate2d rotate) {
  lua::Push(L, static_cast<unsigned int>(rotate));
}

lua::ReadResult Read(lua_State* L, int index, Rotate2d* rotate) {
  int rotate_out;
  auto result = lua::Read(L, index, &rotate_out);
  if (!IsFound(result)) {
    return result;
  }
  *rotate = static_cast<Rotate2d>(static_cast<unsigned int>(rotate_out) % 4);
  return lua::ReadFound();
}

void Push(lua_State* L, Orientation2d orientation) {
  switch (orientation) {
    case Orientation2d::kNorth:
      lua::Push(L, "N");
      break;
    case Orientation2d::kEast:
      lua::Push(L, "E");
      break;
    case Orientation2d::kSouth:
      lua::Push(L, "S");
      break;
    case Orientation2d::kWest:
      lua::Push(L, "W");
      break;
  }
}

lua::ReadResult Read(lua_State* L, int index, Orientation2d* orientation) {
  absl::string_view orientation_out;
  auto result = lua::Read(L, index, &orientation_out);
  if (!IsFound(result)) return result;
  if (orientation_out.size() != 1) {
    return lua::ReadTypeMismatch();
  }
  switch (orientation_out[0]) {
    case 'N':
      *orientation = Orientation2d::kNorth;
      return lua::ReadFound();
    case 'E':
      *orientation = Orientation2d::kEast;
      return lua::ReadFound();
    case 'S':
      *orientation = Orientation2d::kSouth;
      return lua::ReadFound();
    case 'W':
      *orientation = Orientation2d::kWest;
      return lua::ReadFound();
  }
  return lua::ReadTypeMismatch();
}

void Push(lua_State* L, Vector2d vector2) {
  std::array<int, 2> two_values = {vector2.x, vector2.y};
  lua::Push(L, two_values);
}

lua::ReadResult Read(lua_State* L, int index, Vector2d* vector2) {
  std::array<int, 2> two_values;
  auto result = lua::Read(L, index, &two_values);
  if (!IsFound(result)) return result;
  *vector2 = Vector2d{two_values[0], two_values[1]};
  return lua::ReadFound();
}

void Push(lua_State* L, Position2d position) {
  std::array<int, 2> two_values = {position.x, position.y};
  lua::Push(L, two_values);
}

lua::ReadResult Read(lua_State* L, int index, Position2d* position) {
  std::array<int, 2> two_values;
  auto result = lua::Read(L, index, &two_values);
  if (!IsFound(result)) return result;
  *position = Position2d{two_values[0], two_values[1]};
  return lua::ReadFound();
}

void Push(lua_State* L, Transform2d transform) {
  auto table = lua::TableRef::Create(L);
  table.Insert("pos", transform.position);
  table.Insert("orientation", transform.orientation);
  lua::Push(L, table);
}

lua::ReadResult Read(lua_State* L, int index, Transform2d* transform) {
  lua::TableRef table;
  Transform2d transform_out;
  auto result = Read(L, index, &table);
  if (!IsFound(result)) return result;
  result = table.LookUp("pos", &transform_out.position);
  if (!IsFound(result)) return lua::ReadTypeMismatch();
  result = table.LookUp("orientation", &transform_out.orientation);
  if (!IsFound(result)) return lua::ReadTypeMismatch();
  *transform = transform_out;
  return lua::ReadFound();
}

void Push(lua_State* L, Size2d size) {
  auto table = lua::TableRef::Create(L);
  table.Insert("width", size.width);
  table.Insert("height", size.height);
  lua::Push(L, table);
}

lua::ReadResult Read(lua_State* L, int index, Size2d* size) {
  lua::TableRef table;
  Size2d size_out;
  auto result = Read(L, index, &table);
  if (!IsFound(result)) return result;
  result = table.LookUp("width", &size_out.width);
  if (!IsFound(result)) return lua::ReadTypeMismatch();
  result = table.LookUp("height", &size_out.height);
  if (!IsFound(result)) return lua::ReadTypeMismatch();
  *size = size_out;
  return lua::ReadFound();
}

}  // namespace deepmind::lab2d::math
