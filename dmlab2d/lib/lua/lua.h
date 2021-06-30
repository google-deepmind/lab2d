// Copyright (C) 2016-2019 The DMLab2D Authors.
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
// limitations under the License.//
// C++ header for Lua

#ifndef DMLAB2D_LIB_LUA_LUA_H_
#define DMLAB2D_LIB_LUA_LUA_H_

extern "C" {
#include "lauxlib.h"
#include "lua.h"
#include "lualib.h"
}

#include <cstddef>

namespace deepmind::lab2d::lua {

// This is equivalent to calling #table in Lua.
// The length is only well defined if all integer keys of the table are
// contiguous and start at 1. Non integer keys do not affect this value.
inline std::size_t ArrayLength(lua_State* L, int idx) {
#if LUA_VERSION_NUM == 501
  return lua_objlen(L, idx);
#elif LUA_VERSION_NUM == 502
  return lua_rawlen(L, idx);
#else
#error Only Luajit, Lua 5.1 and 5.2 are supported.
#endif
}

}  // namespace deepmind::lab2d::lua

#endif  // DMLAB2D_LIB_LUA_LUA_H_
