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

#include "dmlab2d/lib/env_lua_api/properties.h"

#include <string>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/match.h"
#include "absl/strings/string_view.h"
#include "dmlab2d/lib/lua/bind.h"
#include "dmlab2d/lib/lua/call.h"
#include "dmlab2d/lib/lua/lua.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/push.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/lua/stack_resetter.h"
#include "dmlab2d/lib/lua/table_ref.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind::lab2d {
namespace {

EnvCApi_PropertyResult ProcessResult(lua_State* L, lua::NResultsOr result,
                                     absl::string_view call) {
  if (result.n_results() == 0) {
    LOG_IF(ERROR, !result.ok()) << "[" << call << "] - " << result.error();
    return EnvCApi_PropertyResult_PermissionDenied;
  }
  int prop_result;
  if (IsFound(lua::Read(L, 1, &prop_result))) {
    switch (prop_result) {
      case EnvCApi_PropertyResult_Success:
        return EnvCApi_PropertyResult_Success;
      case EnvCApi_PropertyResult_NotFound:
        return EnvCApi_PropertyResult_NotFound;
      case EnvCApi_PropertyResult_PermissionDenied:
        return EnvCApi_PropertyResult_PermissionDenied;
      case EnvCApi_PropertyResult_InvalidArgument:
        return EnvCApi_PropertyResult_InvalidArgument;
    }
  }

  LOG(ERROR) << "[" << call
             << "] - "
                "- Invalid return type from write! "
                "Must return integer in range [0, 3] - "
                "0 (properties.SUCCESS), "
                "1 (properties.NOT_FOUND), "
                "2 (properties.PERMISSION_DENIED), "
                "3 (properties.INVALID_ARGUMENT)";
  return EnvCApi_PropertyResult_PermissionDenied;
}

struct PropertyListCallbackData {
  void* userdata;
  void (*call)(void* userdata, const char* key,
               EnvCApi_PropertyAttributes flags);
};

lua::NResultsOr PropertyListCallBackFunction(lua_State* L) {
  auto* callback_data = static_cast<PropertyListCallbackData*>(
      lua_touserdata(L, lua_upvalueindex(1)));
  std::string key;
  absl::string_view mode;

  if (!IsFound(lua::Read(L, 1, &key))) {
    return "[propertyList.callback] - Missing Arg 1 - Key";
  }
  if (!IsFound(lua::Read(L, 2, &mode))) {
    return "[propertyList.callback] -  Missing Arg 2 - Mode";
  }

  for (auto c : mode) {
    switch (c) {
      case 'w':
      case 'r':
      case 'l':
        break;
      default:
        return "Type mismatch mode must in the format [r][w][l] of: "
               " 'r' - read-only,"
               " 'w' - write-only,"
               " 'l' - listable";
    }
  }

  int flags = 0;
  if (absl::StrContains(mode, 'w')) {
    flags |= EnvCApi_PropertyAttributes_Writable;
  }
  if (absl::StrContains(mode, 'r')) {
    flags |= EnvCApi_PropertyAttributes_Readable;
  }
  if (absl::StrContains(mode, 'l')) {
    flags |= EnvCApi_PropertyAttributes_Listable;
  }
  if (flags != 0) {
    callback_data->call(callback_data->userdata, key.c_str(),
                        static_cast<EnvCApi_PropertyAttributes>(flags));
  }
  return 0;
}

}  // namespace

lua::NResultsOr Properties::Module(lua_State* L) {
  auto module = lua::TableRef::Create(L);
  module.Insert("SUCCESS", static_cast<int>(EnvCApi_PropertyResult_Success));
  module.Insert("NOT_FOUND", static_cast<int>(EnvCApi_PropertyResult_NotFound));
  module.Insert("PERMISSION_DENIED",
                static_cast<int>(EnvCApi_PropertyResult_PermissionDenied));
  module.Insert("INVALID_ARGUMENT",
                static_cast<int>(EnvCApi_PropertyResult_InvalidArgument));
  lua::Push(L, module);
  return 1;
}

EnvCApi_PropertyResult Properties::WriteProperty(const char* key,
                                                 const char* value) {
  lua_State* L = script_table_ref_.LuaState();
  lua::StackResetter stack_resetter(L);
  script_table_ref_.PushMemberFunction("writeProperty");
  if (lua_isnil(L, -2)) {
    return EnvCApi_PropertyResult_NotFound;
  }
  lua::Push(L, key);
  lua::Push(L, value);
  return ProcessResult(L, lua::Call(L, 3), "writeProperty");
}

EnvCApi_PropertyResult Properties::ReadProperty(const char* key,
                                                const char** value) {
  lua_State* L = script_table_ref_.LuaState();
  lua::StackResetter stack_resetter(L);
  script_table_ref_.PushMemberFunction("readProperty");
  if (lua_isnil(L, -2)) {
    return EnvCApi_PropertyResult_NotFound;
  }
  lua::Push(L, key);
  auto result = lua::Call(L, 2);
  if (result.n_results() == 1) {
    if (IsFound(lua::Read(L, 1, &property_storage_))) {
      *value = property_storage_.c_str();
      return EnvCApi_PropertyResult_Success;
    }
  }

  *value = "";
  return ProcessResult(L, result, "readProperty");
}

EnvCApi_PropertyResult Properties::ListProperty(
    void* userdata, const char* list_key,
    void (*prop_callback)(void* userdata, const char* key,
                          EnvCApi_PropertyAttributes flags)) {
  lua_State* L = script_table_ref_.LuaState();
  lua::StackResetter stack_resetter(L);
  script_table_ref_.PushMemberFunction("listProperty");
  if (lua_isnil(L, -2)) {
    return EnvCApi_PropertyResult_NotFound;
  }
  lua::Push(L, list_key);
  PropertyListCallbackData callback_data{userdata, prop_callback};
  lua_pushlightuserdata(L, &callback_data);
  lua_pushcclosure(L, lua::Bind<PropertyListCallBackFunction>, 1);
  return ProcessResult(L, lua::Call(L, 3), "listProperty");
}

}  // namespace deepmind::lab2d
