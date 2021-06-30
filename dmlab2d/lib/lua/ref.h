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

#ifndef DMLAB2D_LIB_LUA_REF_H_
#define DMLAB2D_LIB_LUA_REF_H_

#include <utility>

#include "dmlab2d/lib/lua/call.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/push.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/support/logging.h"

namespace deepmind::lab2d::lua {

class Ref;

ReadResult Read(lua_State* L, int idx, Ref* ref);

void Push(lua_State* L, const Ref& ref);

// An object of type Ref, when bound, stores a reference to a Lua object. It
// releases that reference on destruction. When unbound it does not refer to or
// release anything.
class Ref final {
 public:
  // Creates an new Ref pointing at value.
  template <typename T>
  static Ref Create(lua_State* L, T value) {
    using lua::Push;
    Push(L, value);
    return Ref(L, luaL_ref(L, LUA_REGISTRYINDEX));
  }

  // Creates an unbound reference.
  Ref();

  // Transfers the reference held by other; other is left in an unbound state.
  Ref(Ref&& other) noexcept;

  // Creates a reference to the object referenced by `other`, if any.
  Ref(const Ref& other);

  Ref& operator=(Ref other);

  // See class documentation above.
  ~Ref();

  // Returns whether the two underlying refs are the same ("shallow
  // comparison"; does not compare contents).
  bool operator==(const Ref& rhs) const;

  bool operator!=(const Ref& rhs) const { return !(*this == rhs); }

  // Returns whether *this is unbound.
  bool is_unbound() const { return lua_state_ == nullptr; }

  friend void swap(Ref& r1, Ref& r2) {
    std::swap(r1.lua_state_, r2.lua_state_);
    std::swap(r1.reference_, r2.reference_);
  }

  template <typename T>
  ReadResult ToValue(T value) const {
    PushToStack();
    auto read_result = Read(lua_state_, -1, value);
    lua_pop(lua_state_, 1);
    return read_result;
  }

  // Gets the internal Lua state.
  lua_State* LuaState() { return lua_state_; }

  // Performs a Lua call on *this with args on the Lua stack. Returns the number
  // of results or an error.
  template <typename... Args>
  NResultsOr Call(Args&&... args) {
    PushToStack();
    (Push(lua_state_, std::forward<Args>(args)), ...);
    return lua::Call(lua_state_, sizeof...(args));
  }

  // Assigns `*ref` to a reference to the object in the Lua stack at `idx`, if
  // that object is not none or nil and returns `ReadFound()`. Otherwise returns
  // `NotFound`.
  friend ReadResult Read(lua_State* L, int idx, Ref* ref) {
    if (!lua_isnoneornil(L, idx)) {
      lua_pushvalue(L, idx);
      *ref = Ref(L, luaL_ref(L, LUA_REGISTRYINDEX));
      return ReadFound();
    } else {
      return ReadNotFound();
    }
  }

  // Push the reference of the object referred to by 'ref' onto the stack.
  // Precondition: !ref.is_unbound().
  friend void Push(lua_State* L, const Ref& ref) { ref.PushToStack(); }

 private:
  // Pushes a copy of the ref referenced by *this onto the Lua stack.
  // Precondition: !this->is_unbound()
  void PushToStack() const {
    CHECK(!is_unbound()) << "Unbound Ref";
    lua_rawgeti(lua_state_, LUA_REGISTRYINDEX, reference_);
  }

  Ref(lua_State* L, int ref_reference);
  lua_State* lua_state_;
  int reference_;
};

}  // namespace deepmind::lab2d::lua

#endif  // DMLAB2D_LIB_LUA_REF_H_
