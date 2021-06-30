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

#ifndef DMLAB2D_LIB_LUA_CLASS_H_
#define DMLAB2D_LIB_LUA_CLASS_H_

#include <new>
#include <string>
#include <utility>

#include "absl/strings/str_cat.h"
#include "dmlab2d/lib/lua/lua.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/push.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/support/logging.h"

namespace deepmind::lab2d::lua {

// Helper class template to expose C++ classes to Lua. To expose a C++ class X
// to Lua, X should derive from lua::Class<X> and provide a name for the
// metatable which Lua uses to identify this class:
//
//   static const char* ClassName() {
//     return "deepmind.lab.UniqueClassName";
//   }
//
// Classes deriving from a specialization of Class shall be final.
//
// Member functions, as well as static factory functions, need to be registered
// with a running Lua VM separately. Non-static member functions need to have
// type "NResultsOr(lua_State*)", and for a given member function f, the
// function Member<&X::f> should be passed to the static Register function for
// registration with the Lua VM. When called from Lua, the bound function first
// attempts to load the class instance from the Lua stack, checking its
// metatable name, and then invokes the corresponding member function on it (but
// see below for ways to customize this behaviour).
//
// Example:
//
//   class X : public lua::Class<X> {
//     static const char* ClassName() { return "example.X"; }
//
//     // Optional validation in some higher sense.
//     bool IsValidObject();
//
//     static Register(lua_State* L) {
//       const lua::Class::Reg methods[] = {"foo", Member<&X::f>};
//       lua::Class::Register(L, methods);
//     }
//
//     LuaNResultsOr f(lua_State* L);
//   };
//
// The host code should call "X::Register(L);" to register the class and its
// member functions. It should also push an object of type X to the Lua stack,
// or register a factory function that can be used from Lua to create instances.
// Given an instace "x" in Lua, the code "x:foo(a, b, c)" calls the instance's
// member function "f", and the arguments are available on the Lua stack. The
// function should pop off the arguments, place its results on the Lua stack,
// and return the number of results, or an error.
//
// The optional IsValidObject function can be defined in X to add further
// checking of the validity of an object.
template <typename T>
class Class {
 public:
  // Allocates memory for and creates a new instance of this class, forwarding
  // arguments to the constructor. The newly allocated instance will be on top
  // of the Lua stack, and a pointer to the class is returned from this
  // function.
  template <typename... Args>
  static inline T* CreateObject(lua_State* L, Args&&... args);

  struct Reg {
    const char* first;
    lua_CFunction second;
  };

  template <typename Regs>
  static inline void Register(lua_State* L, const Regs& members);

  // Reads non-null T* from the Lua stack if the stack contains userdata at the
  // given position, the name of the userdata's metadata is T::ClassName() and
  // the intstance reports itself as valid. Otherwise returns nullptr.
  static T* ReadObject(lua_State* L, int idx) {
    if (T* t = lua::ReadUDT<T>(L, idx, T::ClassName());
        t != nullptr && t->IsValidObject()) {
      return t;
    } else {
      return nullptr;
    }
  }

  // Portability note. This has not got C-language linkage but seems to work.
  template <NResultsOr (T::*Function)(lua_State*)>
  static inline int Member(lua_State* L);

  // Matches lua::Read signature for reading as part of larger structures. Must
  // be called without the 'lua' namespace as it must use argument dependent
  // lookup.
  friend ReadResult Read(lua_State* L, int idx, T** out) {
    if (lua_isnoneornil(L, idx)) {
      return ReadNotFound();
    }
    if (T* t = ReadObject(L, idx)) {
      *out = t;
      return ReadFound();
    } else {
      return ReadTypeMismatch();
    }
  }

 private:
  // Customisation point to allow classes to check whether an object is valid
  // in a domain-specific sense.
  static constexpr bool IsValidObject() { return true; }

  // Destroys this class by calling the destructor, invoked from the Lua "__gc"
  // method.
  static int Destroy(lua_State* L) {
    // May perform longjmp, which is not allowed in C++ except in
    // specific situations. We take care that no objects with non-trivial
    // destructors exist if lua_error is called.
    std::destroy_at(static_cast<T*>(luaL_checkudata(L, 1, T::ClassName())));
    return 0;
  }
};

template <typename T>
template <typename... Args>
T* Class<T>::CreateObject(lua_State* L, Args&&... args) {
  void* lua_node_memory = lua_newuserdata(L, sizeof(T));
  luaL_getmetatable(L, T::ClassName());
  CHECK(!lua_isnil(L, -1)) << T::ClassName() << " has not been registered.";
  lua_setmetatable(L, -2);
  return ::new (lua_node_memory) T(std::forward<Args>(args)...);
}

template <typename T>
template <typename Regs>
void Class<T>::Register(lua_State* L, const Regs& members) {
  luaL_newmetatable(L, T::ClassName());

  // Push __index function pointing at self.
  lua_pushvalue(L, -1);
  lua_setfield(L, -2, "__index");

  lua_pushcfunction(L, &Class::Destroy);
  lua_setfield(L, -2, "__gc");

  for (const auto& member : members) {
    Push(L, member.first);
    // Place method name in up-value to aid diagnostics.
    lua_pushvalue(L, -1);
    lua_pushcclosure(L, member.second, 1);
    lua_settable(L, -3);
  }

  lua_pop(L, 1);
}

// This is the Class member version of Member (lua/bind.h).
template <typename T>
template <NResultsOr (T::*Function)(lua_State*)>
int Class<T>::Member(lua_State* L) {
  // May perform longjmp, which is not allowed in C++ except in specific
  // situations. We take care that no objects with non-trivial destructors exist
  // if lua_error is called.
  T* t = static_cast<T*>(luaL_checkudata(L, 1, T::ClassName()));
  {
    if (t->IsValidObject()) {
      auto result_or = (*t.*Function)(L);
      if (result_or.ok()) {
        return result_or.n_results();
      } else {
        Push(L, absl::StrCat("[", T::ClassName(), ".",
                             lua::ToString(L, lua_upvalueindex(1)), "] - ",
                             result_or.error()));
      }
    } else {
      Push(L, absl::StrCat("Trying to access invalidated object of type: '",
                           T::ClassName(), "' with method '",
                           lua::ToString(L, lua_upvalueindex(1)), "'."));
    }
  }
  // "lua_error" performs a longjmp, which is not allowed in C++ except in
  // specific situations. We take care that no objects with non-trivial
  // destructors exist when lua_error is called.
  return lua_error(L);
}

}  // namespace deepmind::lab2d::lua

#endif  // DMLAB2D_LIB_LUA_CLASS_H_
