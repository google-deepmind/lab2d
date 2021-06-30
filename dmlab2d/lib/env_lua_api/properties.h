// Copyright (C) 2018-2019 The DMLab2D Authors.
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

#ifndef DMLAB2D_LIB_ENV_LUA_API_PROPERTIES_H_
#define DMLAB2D_LIB_ENV_LUA_API_PROPERTIES_H_

#include <string>
#include <utility>

#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/table_ref.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind::lab2d {

class Properties {
 public:
  // Returns property module with enums for return results.
  static lua::NResultsOr Module(lua_State* L);

  // Binds the properties to the Lua API.
  // Keeps a reference to the table for further calls.
  lua::NResultsOr BindApi(lua::TableRef script_table_ref) {
    script_table_ref_ = std::move(script_table_ref);
    return 0;
  }

  // Calls writeProperty on Lua API. Returns whether write was successful.
  EnvCApi_PropertyResult WriteProperty(const char* key, const char* value);

  // Calls readProperty on Lua API. Returns whether read was successful.
  EnvCApi_PropertyResult ReadProperty(const char* key, const char** value);

  // Calls listProperty on Lua API. Returns whether list was successful.
  EnvCApi_PropertyResult ListProperty(
      void* userdata, const char* list_key,
      void (*prop_callback)(void* userdata, const char* key,
                            EnvCApi_PropertyAttributes flags));

 private:
  lua::TableRef script_table_ref_;

  // Last property string storage.
  std::string property_storage_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_ENV_LUA_API_PROPERTIES_H_
