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
// limitations under the License.
//
////////////////////////////////////////////////////////////////////////////////

#include "dmlab2d/lib/lua/call.h"

#include <string>
#include <utility>

#include "absl/log/check.h"
#include "dmlab2d/lib/lua/push.h"
#include "dmlab2d/lib/lua/read.h"

namespace deepmind::lab2d::lua {

extern "C" {
static int traceback(lua_State* L) {
  if (!lua_isstring(L, 1)) return 1;
  lua_getglobal(L, "debug");
  if (!lua_istable(L, -1)) {
    lua_pop(L, 1);
    return 1;
  }
  lua_getfield(L, -1, "traceback");
  if (!lua_isfunction(L, -1)) {
    lua_pop(L, 2);
    return 1;
  }
  lua_pushvalue(L, 1);
  lua_pushinteger(L, 2);
  lua_call(L, 2, 1);
  return 1;
}
}  // extern "C"

NResultsOr Call(lua_State* L, int nargs, bool with_traceback) {
  CHECK_GE(nargs, 0) << "Invalid number of arguments: " << nargs;
  int err_stackpos = 0;
  if (with_traceback) {
    err_stackpos = lua_gettop(L) - nargs;
    Push(L, traceback);
    lua_insert(L, err_stackpos);
  }
  if (lua_pcall(L, nargs, LUA_MULTRET, err_stackpos) != 0) {
    std::string error;
    if (!IsFound(Read(L, -1, &error))) {
      error = "Failed to retrieve error!";
    }
    if (with_traceback) {
      lua_remove(L, err_stackpos);
    }
    lua_pop(L, 1);
    return std::move(error);
  } else {
    if (with_traceback) {
      lua_remove(L, err_stackpos);
    }
    return lua_gettop(L) - err_stackpos + 1;
  }
}

}  // namespace deepmind::lab2d::lua
