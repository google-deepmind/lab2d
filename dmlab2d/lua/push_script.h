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

#ifndef DMLAB2D_LUA_PUSH_SCRIPT_H_
#define DMLAB2D_LUA_PUSH_SCRIPT_H_

#include <cstddef>
#include <cstring>
#include <string>

#include "absl/strings/string_view.h"
#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/n_results_or.h"

namespace deepmind::lab2d::lua {

// Push a script onto stack ready for calling. If script has a syntax error it
// returns that instead.
NResultsOr PushScript(lua_State* L, absl::string_view script,
                      const char* script_name);

inline NResultsOr PushScript(lua_State* L, absl::string_view script,
                             const std::string& script_name) {
  return PushScript(L, script, script_name.c_str());
}

// Loads a script from file and onto the stack ready for calling. If the file
// cannot be loaded or has a syntax error that error is returned instead.
NResultsOr PushScriptFile(  //
    lua_State* L,           //
    const char* filename);

inline NResultsOr PushScriptFile(  //
    lua_State* L,                  //
    const std::string& filename) {
  return PushScriptFile(L, filename.c_str());
}

}  // namespace deepmind::lab2d::lua

#endif  // DMLAB2D_LUA_PUSH_SCRIPT_H_
