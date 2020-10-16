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

#include "dmlab2d/lua/table_ref.h"

#include "dmlab2d/lua/lua.h"
#include "dmlab2d/support/logging.h"

namespace deepmind::lab2d::lua {

TableRef::~TableRef() {
  if (lua_state_) {
    luaL_unref(lua_state_, LUA_REGISTRYINDEX, table_reference_);
  }
}

TableRef::TableRef() : TableRef(nullptr, 0) {}

// Private. 'table_reference' is now owned by this object.
TableRef::TableRef(lua_State* L, int table_reference)
    : lua_state_(L), table_reference_(table_reference) {}

TableRef::TableRef(TableRef&& other) noexcept
    : lua_state_(other.lua_state_), table_reference_(other.table_reference_) {
  other.lua_state_ = nullptr;
  other.table_reference_ = 0;
}

TableRef::TableRef(const TableRef& other)
    : lua_state_(other.lua_state_), table_reference_(0) {
  // Create our own internal reference.
  if (other.lua_state_) {
    other.PushTable();
    table_reference_ = luaL_ref(lua_state_, LUA_REGISTRYINDEX);
  }
}

bool TableRef::operator==(const TableRef& rhs) const {
  if (lua_state_ != rhs.lua_state_) {
    return false;
  } else if (lua_state_ == nullptr) {
    return true;
  } else if (table_reference_ == rhs.table_reference_) {
    return true;
  } else {
    PushTable();
    rhs.PushTable();
    bool equal = lua_rawequal(lua_state_, -1, -2);
    lua_pop(lua_state_, 2);
    return equal;
  }
}

std::size_t TableRef::ArraySize() const {
  PushTable();
  std::size_t count = ArrayLength(lua_state_, -1);
  lua_pop(lua_state_, 1);
  return count;
}

std::size_t TableRef::KeyCount() const {
  std::size_t result = 0;
  PushTable();
  if (lua_type(lua_state_, -1) == LUA_TUSERDATA) {
    if (lua_getmetatable(lua_state_, -1)) {
      lua_remove(lua_state_, -2);
    }
  }

  if (lua_type(lua_state_, -1) != LUA_TTABLE) {
    return 0;
  }

  lua_pushnil(lua_state_);
  while (lua_next(lua_state_, -2) != 0) {
    ++result;
    lua_pop(lua_state_, 1);
  }
  lua_pop(lua_state_, 1);
  return result;
}

TableRef& TableRef::operator=(TableRef other) {
  this->swap(other);
  return *this;
}

TableRef TableRef::Create(lua_State* L) {
  CHECK(L != nullptr) << "Creating a table with a null State.";
  lua_createtable(L, 0, 0);
  return TableRef(L, luaL_ref(L, LUA_REGISTRYINDEX));
}

void TableRef::PushTable() const {
  CHECK(!is_unbound()) << "Unbound TableRef";
  lua_rawgeti(lua_state_, LUA_REGISTRYINDEX, table_reference_);
}

void Push(lua_State* L, const TableRef& table) { table.PushTable(); }

ReadResult Read(lua_State* L, int idx, TableRef* table) {
  switch (lua_type(L, idx)) {
    case LUA_TTABLE:
    case LUA_TUSERDATA:
      lua_pushvalue(L, idx);
      *table = TableRef(L, luaL_ref(L, LUA_REGISTRYINDEX));
      return ReadFound();
    case LUA_TNIL:
    case LUA_TNONE:
      return ReadNotFound();
    default:
      return ReadTypeMismatch();
  }
}

}  // namespace deepmind::lab2d::lua
