// Copyright (C) 2017-2020 The DMLab2D Authors.
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

#ifndef DMLAB2D_SYSTEM_FILE_SYSTEM_LUA_FILE_SYSTEM_H_
#define DMLAB2D_SYSTEM_FILE_SYSTEM_LUA_FILE_SYSTEM_H_

#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/n_results_or.h"

namespace deepmind::lab2d {

// Returns file_system module. A pointer to FileSystem must exist in the up
// value.
lua::NResultsOr LuaFileSystemRequire(lua_State* L);

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_SYSTEM_FILE_SYSTEM_LUA_FILE_SYSTEM_H_
