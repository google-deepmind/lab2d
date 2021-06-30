// Copyright (C) 2018-2019 The DMLab2D Authors.
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

#ifndef DMLAB2D_LIB_LUA_STACK_RESETTER_H_
#define DMLAB2D_LIB_LUA_STACK_RESETTER_H_

#include "dmlab2d/lib/lua/lua.h"

namespace deepmind::lab2d::lua {

// On construction stores the current Lua stack position.
// On destruction returns Lua stack to the position it was constructed in.
//
// Example:
//
//  {
//    StackResetter stack_resetter(L);
//    PushLuaFunctionOnToStack();
//    auto result = Call(L, 0);
//    if (result.n_results() > 0) {
//      return true; // No need to call lua_pop(L, result.n_results());
//    }
//  }
class StackResetter {
 public:
  // 'L' is stored along with current stack size.
  explicit StackResetter(lua_State* L)
      : lua_state_(L), stack_size_(lua_gettop(lua_state_)) {}
  ~StackResetter() { lua_settop(lua_state_, stack_size_); }

  StackResetter& operator=(const StackResetter&) = delete;
  StackResetter(const StackResetter&) = delete;

 private:
  lua_State* lua_state_;
  int stack_size_;
};

}  // namespace deepmind::lab2d::lua

#endif  // DMLAB2D_LIB_LUA_STACK_RESETTER_H_
