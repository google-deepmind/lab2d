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

#ifndef DMLAB2D_LIB_LUA_BIND_H_
#define DMLAB2D_LIB_LUA_BIND_H_

#include <string>

#include "dmlab2d/lib/lua/lua.h"
#include "dmlab2d/lib/lua/n_results_or.h"

namespace deepmind::lab2d::lua {

// Binds a function that returns a NResultsOr to properly propagate errors.
// This is does not have C-language linkage but seems to work.
template <NResultsOr (&F)(lua_State*)>
int Bind(lua_State* L) {
  {
    NResultsOr result_or = F(L);
    if (result_or.ok()) {
      return result_or.n_results();
    } else {
      lua_pushlstring(L, result_or.error().data(), result_or.error().size());
    }
  }
  // "lua_error" performs a longjmp, which is not allowed in C++ except in
  // specific situations. We take care that no objects with non-trivial
  // destructors exist when lua_error is called.
  return lua_error(L);
}

}  // namespace deepmind::lab2d::lua

#endif  // DMLAB2D_LIB_LUA_BIND_H_
