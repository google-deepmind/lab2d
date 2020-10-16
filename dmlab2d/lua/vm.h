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

#ifndef DMLAB2D_LUA_VM_H_
#define DMLAB2D_LUA_VM_H_

#include <cstddef>
#include <memory>
#include <string>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "dmlab2d/lua/lua.h"

namespace deepmind::lab2d::lua {
namespace internal {

struct EmbeddedLuaFile {
  const char* buff;
  std::size_t size;
};

struct EmbeddedClosure {
  lua_CFunction function;
  std::vector<void*> up_values;
};

struct Close {
  void operator()(lua_State* L) { lua_close(L); }
};

}  // namespace internal

class Vm {
 public:
  static Vm Create() {
    lua_State* L = luaL_newstate();
    luaL_openlibs(L);
    return Vm(L);
  }

  // Maintain unique_ptr interface.
  lua_State* get() { return lua_state_.get(); }
  const lua_State* get() const { return lua_state_.get(); }
  bool operator!=(const std::nullptr_t) const { return lua_state_ != nullptr; }
  bool operator==(const std::nullptr_t) const { return lua_state_ == nullptr; }

  // The following two functions add modules to the Lua VM. That is, when Lua
  // executes the expression "require 'foo'", it passes the string 'foo' to the
  // module searchers until one of them succeeds. Registering the name 'foo'
  // using one of these functions will make the lookup succeed for that name.
  //
  // If the module was registered with AddCModuleToSearchers, the function F is
  // called, and the call produces the value of the "require" expression. (It is
  // advisable for the call to return a single table on the stack.)
  // The upvalues will be available when the module is called.
  //
  // If the module was registerd with AddLuaModuleToSearchers, the script
  // contained in the string [buf, buf + size) is executed as if it were the
  // body of a single function. (It is advisable for the script to return a
  // single table.) Any errors in the script are propagated to the calling
  // script.
  void AddCModuleToSearchers(std::string module_name, lua_CFunction F,
                             std::vector<void*> up_values = {});

  void AddLuaModuleToSearchers(std::string module_name, const char* buf,
                               std::size_t size);

  // Add a path to be included in search when calling require.
  void AddPathToSearchers(absl::string_view path);

 private:
  // Takes ownership of lua_State.
  explicit Vm(lua_State* L);

  std::unique_ptr<lua_State, internal::Close> lua_state_;

  // These are unique_ptrs as the pointers are stored in upvalues for a Lua
  // module search function.
  std::unique_ptr<absl::flat_hash_map<std::string, internal::EmbeddedClosure>>
      embedded_c_modules_;
  std::unique_ptr<absl::flat_hash_map<std::string, internal::EmbeddedLuaFile>>
      embedded_lua_modules_;
};

inline Vm CreateVm() { return Vm::Create(); }

}  // namespace deepmind::lab2d::lua

#endif  // DMLAB2D_LUA_VM_H_
