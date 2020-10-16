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

#ifndef DMLAB2D_LUA_PUSH_H_
#define DMLAB2D_LUA_PUSH_H_

#include <array>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <string>
#include <type_traits>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "absl/types/variant.h"
#include "dmlab2d/lua/lua.h"

namespace deepmind::lab2d::lua {

inline void Push(lua_State* L, const absl::string_view value) {
  lua_pushlstring(L, value.data(), value.size());
}

inline void Push(lua_State* L, const char* value) {
  lua_pushlstring(L, value, std::strlen(value));
}

inline void Push(lua_State* L, lua_Number value) { lua_pushnumber(L, value); }

inline void Push(lua_State* L, lua_Integer value) { lua_pushinteger(L, value); }

inline void Push(lua_State* L, bool value) { lua_pushboolean(L, value); }

inline void Push(lua_State* L, lua_CFunction value) {
  lua_pushcfunction(L, value);
}

inline void Push(lua_State* L, void* value) { lua_pushlightuserdata(L, value); }

// Templated Push that takes any arithmetic values that don't have non-template
// overloads already.
//
template <typename T>
typename std::enable_if<std::is_floating_point<T>::value>::type Push(
    lua_State* L, T value) {
  Push(L, static_cast<lua_Number>(value));
}
template <typename T>
typename std::enable_if<std::is_integral<T>::value &&
                        !std::is_same<T, bool>::value>::type
Push(lua_State* L, T value) {
  Push(L, static_cast<lua_Integer>(value));
}

template <typename T, typename A>
void Push(lua_State* L, const std::vector<T, A>& values);

template <typename T, std::size_t N>
void Push(lua_State* L, const std::array<T, N>& values);

template <typename K, typename T, typename H, typename C, typename A>
void Push(lua_State* L, const absl::flat_hash_map<K, T, H, C, A>& values);

template <typename T>
void Push(lua_State* L, absl::Span<T> values);

template <typename... T>
void Push(lua_State* L, const absl::variant<T...>& value);

// End of public header, implementation details follow.

template <typename T>
void Push(lua_State* L, absl::Span<T> values) {
  lua_createtable(L, values.size(), 0);
  for (std::size_t i = 0; i < values.size(); ++i) {
    Push(L, i + 1);
    Push(L, values[i]);
    lua_settable(L, -3);
  }
}

template <typename T, typename A>
void Push(lua_State* L, const std::vector<T, A>& values) {
  Push(L, absl::MakeConstSpan(values));
}

template <typename T, std::size_t N>
void Push(lua_State* L, const std::array<T, N>& values) {
  Push(L, absl::MakeConstSpan(values));
}

template <typename K, typename T, typename H, typename C, typename A>
void Push(lua_State* L, const absl::flat_hash_map<K, T, H, C, A>& values) {
  lua_createtable(L, 0, values.size());
  for (const auto& pair : values) {
    Push(L, pair.first);
    Push(L, pair.second);
    lua_settable(L, -3);
  }
}

namespace internal {

struct PushVariant {
  template <typename T>
  void operator()(const T& value) const {
    Push(L, value);
  }

  void operator()(const absl::monostate) const { lua_pushnil(L); }

  lua_State* L;
};

}  // namespace internal

template <typename... T>
void Push(lua_State* L, const absl::variant<T...>& value) {
  absl::visit(internal::PushVariant{L}, value);
}

}  // namespace deepmind::lab2d::lua

#endif  // DMLAB2D_LUA_PUSH_H_
