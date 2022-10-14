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

#include "dmlab2d/lib/lua/vm.h"

#include <utility>

#include "absl/strings/str_cat.h"
#include "dmlab2d/lib/lua/lua.h"

#if LUA_VERSION_NUM == 501
static constexpr char kSearcher[] = "loaders";
#elif LUA_VERSION_NUM == 502
static constexpr char kSearcher[] = "searchers";
#else
#error Only Lua 5.1 and 5.2 are supported
#endif

using deepmind::lab2d::lua::internal::EmbeddedClosure;
using deepmind::lab2d::lua::internal::EmbeddedLuaFile;

extern "C" {
static int PackageLoader(lua_State* L) {
  do {
    int upidx_c = lua_upvalueindex(1);
    int upidx_lua = lua_upvalueindex(2);
    if (!lua_islightuserdata(L, upidx_c) ||
        !lua_islightuserdata(L, upidx_lua)) {
      lua_pushstring(L, "Missing searchers");
      break;
    }

    auto* embedded_c_modules =
        static_cast<const absl::flat_hash_map<std::string, EmbeddedClosure>*>(
            lua_touserdata(L, upidx_c));
    auto* embedded_lua_modules =
        static_cast<const absl::flat_hash_map<std::string, EmbeddedLuaFile>*>(
            lua_touserdata(L, upidx_lua));

    if (lua_type(L, 1) != LUA_TSTRING) {
      // Allow other searchers to deal with this.
      lua_pushstring(L, "'required' called with a non-string argument!");
      return 1;
    }

    std::size_t length = 0;
    const char* result_cstr = lua_tolstring(L, 1, &length);
    std::string name(result_cstr, length);
    auto it = embedded_c_modules->find(name);
    if (it != embedded_c_modules->end()) {
      for (void* light_value_data : it->second.up_values) {
        lua_pushlightuserdata(L, light_value_data);
      }
      lua_pushcclosure(L, it->second.function, it->second.up_values.size());
      return 1;
    } else {
      auto it = embedded_lua_modules->find(name);
      if (it != embedded_lua_modules->end()) {
        if (luaL_loadbuffer(L, it->second.buff, it->second.size,
                            name.c_str())) {
          // Error message is on stack. Let caller deal with it.
          break;
        } else {
          return 1;
        }
      } else {
        // Allow other searchers to deal with this.
        lua_pushstring(L, "Not found internally!");
        return 1;
      }
    }
  } while (false);
  return lua_error(L);
}
}  // extern "C"

namespace deepmind::lab2d::lua {

void Vm::AddPathToSearchers(absl::string_view path) {
  lua_State* L = get();
  lua_getglobal(L, "package");
  lua_getfield(L, -1, "path");
  const std::string new_path = absl::StrCat(lua_tostring(L, -1),  //
                                            ";", path, "/?.lua",  //
                                            ";", path, "/?/init.lua");
  lua_pop(L, 1);
  lua_pushlstring(L, new_path.c_str(), new_path.length());
  lua_setfield(L, -2, "path");
  lua_pop(L, 1);
}

void Vm::AddCModuleToSearchers(std::string module_name, lua_CFunction F,
                               std::vector<void*> up_values) {
  (*embedded_c_modules_)[std::move(module_name)] = {F, std::move(up_values)};
}

void Vm::AddLuaModuleToSearchers(std::string module_name, const char* buf,
                                 std::size_t size) {
  (*embedded_lua_modules_)[std::move(module_name)] = {buf, size};
}

constexpr absl::string_view kInstallTraceback = R"lua(
local function _makeError(msg)
  local ESCAPE = string.char(27)
  local RED = ESCAPE .. '[0;31m'
  local CLEAR = ESCAPE .. '[0;0m'
  return string.format("%sERROR:%s %s", RED, CLEAR, msg)
end

local function _shorten(path)
  return string.match(path, 'runfiles/(.*)') or path
end

local function traceback(msg, level)
  local trace = {'\nstack trace-back:'}
  level = level or 1  -- Ignore this function.
  while true do
    level = level + 1
    local func = debug.getinfo(level, 'Sln')
    if func == nil then break end
    local loc = ''
    local src = func.source
    if src:sub(1, 1) ~= '=' then
      local source = _shorten(src)
      if #src >= 60 then
        msg = msg:gsub('...' .. src:sub(#src - 55, #src), source)
      end
      loc = string.format("%s:%d:", source, func.currentline)
    else
      loc = src:sub(2) .. ':'
    end
    if func.name then
       loc = loc .. string.format(' in function \'%s\'', func.name)
    end
    table.insert(trace, _makeError(loc))
  end

  return '\n' .. _makeError(msg) .. table.concat(trace, '\n')
end
debug._traceback = debug.traceback
debug.traceback = traceback
)lua";

Vm::Vm(lua_State* L)
    : lua_state_(L),
      embedded_c_modules_(
          new absl::flat_hash_map<std::string, EmbeddedClosure>()),
      embedded_lua_modules_(
          new absl::flat_hash_map<std::string, EmbeddedLuaFile>()) {
  lua_getglobal(L, "package");
  lua_getfield(L, -1, kSearcher);
  int array_size = ArrayLength(L, -1);
  for (int e = array_size + 1; e > 1; e--) {
    lua_rawgeti(L, -1, e - 1);
    lua_rawseti(L, -2, e);
  }

  lua_pushlightuserdata(L, embedded_c_modules_.get());
  lua_pushlightuserdata(L, embedded_lua_modules_.get());
  lua_pushcclosure(L, &PackageLoader, 2);
  lua_rawseti(L, -2, 1);
  lua_pop(L, 2);
  luaL_loadbuffer(L, kInstallTraceback.data(), kInstallTraceback.size(),
                  "InstallTraceback");
  lua_call(L, 0, 0);
}

}  // namespace deepmind::lab2d::lua
