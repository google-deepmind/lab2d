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

#ifndef DMLAB2D_SYSTEM_GRID_WORLD_LUA_LUA_HANDLE_H_
#define DMLAB2D_SYSTEM_GRID_WORLD_LUA_LUA_HANDLE_H_

#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/push.h"
#include "dmlab2d/lua/read.h"
#include "dmlab2d/system/grid_world/collections/handle.h"

namespace deepmind::lab2d {

// Support Lua Read and Push.
template <typename Tag>
inline void Push(lua_State* L, Handle<Tag> handle) {
  if (handle.IsEmpty()) {
    lua_pushnil(L);
  } else {
    lua::Push(L, handle.Value());
  }
}

template <typename Tag>
inline lua::ReadResult Read(lua_State* L, int index, Handle<Tag>* handle) {
  if (lua_isnil(L, index)) {
    *handle = Handle<Tag>();
    return lua::ReadFound();
  } else {
    typename Handle<Tag>::ValueType value;
    auto result = lua::Read(L, index, &value);
    if (IsFound(result)) {
      *handle = Handle<Tag>(value);
    }
    return result;
  }
}

}  // namespace deepmind::lab2d
#endif  // DMLAB2D_SYSTEM_GRID_WORLD_LUA_LUA_HANDLE_H_
