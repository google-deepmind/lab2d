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

#include "dmlab2d/lib/lua/push_script.h"

#include <string>
#include <utility>

#include "absl/strings/str_cat.h"
#include "absl/strings/string_view.h"
#include "dmlab2d/lib/lua/read.h"

namespace deepmind::lab2d::lua {

NResultsOr PushScript(lua_State* L, absl::string_view script,
                      const char* script_name) {
  if (luaL_loadbuffer(L, script.data(), script.size(), script_name)) {
    std::string error;
    if (!IsFound(Read(L, -1, &error))) error = "Failed to retrieve error!";
    return std::move(error);
  }
  return 1;
}

NResultsOr PushScriptFile(lua_State* L, const char* filename) {
  int error = luaL_loadfile(L, filename);
  if (error == LUA_ERRFILE) {
    return absl::StrCat("Failed to open file '", filename, "'");
  } else if (error != 0) {
    std::string error;
    if (!IsFound(Read(L, -1, &error))) error = "Failed to retrieve error!";
    return std::move(error);
  }
  return 1;
}

}  // namespace deepmind::lab2d::lua
