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

#ifndef DMLAB2D_SYSTEM_MATH_LUA_MATH_MATH2D_H_
#define DMLAB2D_SYSTEM_MATH_LUA_MATH_MATH2D_H_

#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/read.h"
#include "dmlab2d/system/math/math2d.h"

namespace deepmind::lab2d::math {

// Pushes an integer in units of right 90-degree turns.
void Push(lua_State* L, Rotate2d rotate);

// Reads an integer in units of right 90-degree turns.
lua::ReadResult Read(lua_State* L, int index, Rotate2d* rotate);

// Pushes facing direction as string 'N', 'E', 'S' or 'W'.
void Push(lua_State* L, Orientation2d orientation);

// Reads facing direction as string if one of 'N', 'E', 'S' or 'W'.
lua::ReadResult Read(lua_State* L, int index, Orientation2d* orientation);

// Pushes `vector2` as Lua array containing `vector2.x`, `vector2.y`.
void Push(lua_State* L, Vector2d vector2);

// Reads `vector2` from Lua array of two integers.
lua::ReadResult Read(lua_State* L, int index, Vector2d* vector2);

// Pushes `position` as Lua array containing `position.x`, `position.y`.
void Push(lua_State* L, Position2d position);

// Reads `position` from Lua array of two integers.
lua::ReadResult Read(lua_State* L, int index, Position2d* position);

// Pushes `transform` as Lua Table with keys 'pos' and 'orientation'.
void Push(lua_State* L, Transform2d transform);

// Reads `transform` from Lua table with keys 'pos' and 'orientation'.
lua::ReadResult Read(lua_State* L, int index, Transform2d* transform);

// Pushes `size` as Lua Table with keys 'width' and 'height'.
void Push(lua_State* L, Size2d size);

// Reads `size` from Lua table with keys 'width' and 'height'.
lua::ReadResult Read(lua_State* L, int index, Size2d* size);

}  // namespace deepmind::lab2d::math

#endif  // DMLAB2D_SYSTEM_MATH_LUA_MATH_MATH2D_H_
