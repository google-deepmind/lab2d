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

#include "dmlab2d/lua/ref.h"

#include "absl/utility/utility.h"
#include "dmlab2d/lua/lua.h"
#include "dmlab2d/support/logging.h"

namespace deepmind::lab2d::lua {

Ref::~Ref() {
  if (lua_state_) {
    luaL_unref(lua_state_, LUA_REGISTRYINDEX, reference_);
  }
}

Ref::Ref() : Ref(nullptr, 0) {}

// Private. 'reference' is now owned by this object.
Ref::Ref(lua_State* L, int reference) : lua_state_(L), reference_(reference) {}

Ref::Ref(Ref&& other) noexcept
    : lua_state_(absl::exchange(other.lua_state_, nullptr)),
      reference_(absl::exchange(other.reference_, 0)) {}

Ref::Ref(const Ref& other) : lua_state_(other.lua_state_), reference_(0) {
  // Create our own internal reference.
  if (other.lua_state_) {
    other.PushToStack();
    reference_ = luaL_ref(lua_state_, LUA_REGISTRYINDEX);
  }
}

bool Ref::operator==(const Ref& rhs) const {
  if (lua_state_ != rhs.lua_state_) {
    return false;
  } else if (lua_state_ == nullptr) {
    return true;
  } else if (reference_ == rhs.reference_) {
    return true;
  } else {
    PushToStack();
    rhs.PushToStack();
    bool equal = lua_rawequal(lua_state_, -1, -2);
    lua_pop(lua_state_, 2);
    return equal;
  }
}

Ref& Ref::operator=(Ref other) {
  swap(*this, other);
  return *this;
}

}  // namespace deepmind::lab2d::lua
