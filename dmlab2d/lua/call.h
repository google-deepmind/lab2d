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

#ifndef DMLAB2D_LUA_CALL_H_
#define DMLAB2D_LUA_CALL_H_

#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/n_results_or.h"

namespace deepmind::lab2d::lua {

// Calls a Lua function. The Lua stack should be arranged such a way that
// there is a function and nargs items after it:
//
//     ... [function] [arg1] [arg2] ... [argn]
//
// Returns the number of results or an error. (Backtrace generation can be
// suppressed by setting with_traceback to false.)
//
// [-(nargs + 1), +(nresults|0), -]
NResultsOr Call(lua_State* L, int nargs, bool with_traceback = true);

}  // namespace deepmind::lab2d::lua

#endif  // DMLAB2D_LUA_CALL_H_
