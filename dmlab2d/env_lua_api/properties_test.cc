// Copyright (C) 2019 The DMLab2D Authors.
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
#include "dmlab2d/env_lua_api/properties.h"

#include "absl/strings/string_view.h"
#include "dmlab2d/lua/bind.h"
#include "dmlab2d/lua/call.h"
#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/n_results_or_test_util.h"
#include "dmlab2d/lua/push_script.h"
#include "dmlab2d/lua/table_ref.h"
#include "dmlab2d/lua/vm_test_util.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind::lab2d {
namespace {

using ::deepmind::lab2d::lua::testing::IsOkAndHolds;
using ::testing::Eq;
using ::testing::StrEq;

class PropertiesTest : public lua::testing::TestWithVm {
 protected:
  PropertiesTest() {
    vm()->AddCModuleToSearchers("system.properties",
                                &lua::Bind<Properties::Module>);
  }
};

constexpr char kPropertyApi[] = R"(
local properties = require 'system.properties'

local api = {
    _readWriteNumber = 10,
    _readOnlyString = 'Hello',
    _writeString = 'There',
    _emptyList = {},
}

function api:writeProperty(key, value)
  if key == '' then
    return properties.PERMISSION_DENIED
  end
  if key == 'readWriteNumber' then
    v = tonumber(value)
    if v ~= nil then
      self._readWriteNumber = v
      return properties.SUCCESS
    else
      return properties.INVALID_ARGUMENT
    end
  end
  if key == 'readOnlyString' then
    return properties.PERMISSION_DENIED
  end
  if key == 'writeString' then
    self._writeString = value
    return properties.SUCCESS
  end
  if key == 'emptyList' then
    return properties.PERMISSION_DENIED
  end
  return properties.NOT_FOUND
end

function api:readProperty(key)
  if key == '' then
    return properties.PERMISSION_DENIED
  end
  if key == 'readWriteNumber' then
    return tostring(self._readWriteNumber)
  end
  if key == 'readOnlyString' then
    return self._readOnlyString
  end
  if key == 'writeString' then
    return properties.PERMISSION_DENIED
  end
  if key == 'emptyList' then
    return properties.PERMISSION_DENIED
  end
  return properties.NOT_FOUND
end

function api:listProperty(key, callback)
  if key == '' then
    callback('readWriteNumber', 'rw')
    callback('readOnlyString', 'r')
    callback('writeString', 'w')
    callback('emptyList', 'l')
    return properties.SUCCESS
  end
  if key == 'emptyList' then
    return properties.SUCCESS
  end
  if key == 'readWriteNumber' or key == 'readOnlyString' or
      key == 'writeString' then
    return properties.PERMISSION_DENIED
  end
  return properties.NOT_FOUND
end

return api
)";

TEST_F(PropertiesTest, TestRead) {
  ASSERT_THAT(lua::PushScript(L, kPropertyApi, "kPropertyApi"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef table;
  ASSERT_TRUE(IsFound(Read(L, 1, &table)));
  lua_settop(L, 0);
  Properties properties;
  ASSERT_THAT(properties.BindApi(table), IsOkAndHolds(0));
  const char* value = "";
  EXPECT_THAT(properties.ReadProperty("", &value),
              Eq(EnvCApi_PropertyResult_PermissionDenied));
  EXPECT_THAT(properties.ReadProperty("emptyList", &value),
              Eq(EnvCApi_PropertyResult_PermissionDenied));
  EXPECT_THAT(properties.ReadProperty("readWriteNumber", &value),
              Eq(EnvCApi_PropertyResult_Success));
  EXPECT_THAT(value, StrEq("10"));
  EXPECT_THAT(properties.ReadProperty("readOnlyString", &value),
              Eq(EnvCApi_PropertyResult_Success));
  EXPECT_THAT(value, StrEq("Hello"));
  EXPECT_THAT(properties.ReadProperty("writeString", &value),
              Eq(EnvCApi_PropertyResult_PermissionDenied));
  EXPECT_THAT(properties.ReadProperty("notExist", &value),
              Eq(EnvCApi_PropertyResult_NotFound));
}

TEST_F(PropertiesTest, TestWrite) {
  ASSERT_THAT(lua::PushScript(L, kPropertyApi, "kPropertyApi"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef table;
  ASSERT_TRUE(IsFound(Read(L, 1, &table)));
  lua_settop(L, 0);
  Properties properties;
  ASSERT_THAT(properties.BindApi(table), IsOkAndHolds(0));
  EXPECT_THAT(properties.WriteProperty("", "Hello"),
              Eq(EnvCApi_PropertyResult_PermissionDenied));
  EXPECT_THAT(properties.WriteProperty("emptyList", "Hello"),
              Eq(EnvCApi_PropertyResult_PermissionDenied));

  EXPECT_THAT(properties.WriteProperty("readWriteNumber", "Blah"),
              Eq(EnvCApi_PropertyResult_InvalidArgument));

  EXPECT_THAT(properties.WriteProperty("readWriteNumber", "10"),
              Eq(EnvCApi_PropertyResult_Success));
  int int_value = 0;
  EXPECT_TRUE(table.LookUp("_readWriteNumber", &int_value));
  EXPECT_THAT(int_value, Eq(10));

  EXPECT_THAT(properties.WriteProperty("readOnlyString", ""),
              Eq(EnvCApi_PropertyResult_PermissionDenied));

  EXPECT_THAT(properties.WriteProperty("writeString", "Anything"),
              Eq(EnvCApi_PropertyResult_Success));

  absl::string_view value;
  EXPECT_TRUE(table.LookUp("_writeString", &value));
  EXPECT_THAT(value, Eq("Anything"));

  EXPECT_THAT(properties.WriteProperty("notExists", "Anything"),
              Eq(EnvCApi_PropertyResult_NotFound));
}

TEST_F(PropertiesTest, TestList) {
  ASSERT_THAT(lua::PushScript(L, kPropertyApi, "kPropertyApi"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef table;
  ASSERT_TRUE(IsFound(Read(L, 1, &table)));
  lua_settop(L, 0);
  Properties properties;
  ASSERT_THAT(properties.BindApi(table), IsOkAndHolds(0));

  struct Results {
    std::vector<std::string> names;
    std::vector<EnvCApi_PropertyAttributes> attributes;
  };

  auto callback = +[](void* userdata, const char* key,
                      EnvCApi_PropertyAttributes attributes) {
    auto* results_ptr = static_cast<Results*>(userdata);
    results_ptr->names.emplace_back(key);
    results_ptr->attributes.emplace_back(attributes);
  };

  Results all;
  EXPECT_THAT(properties.ListProperty(&all, "", callback),
              Eq(EnvCApi_PropertyResult_Success));

  EXPECT_THAT(all.names,
              ::testing::ElementsAre("readWriteNumber", "readOnlyString",
                                     "writeString", "emptyList"));
  EXPECT_THAT(all.attributes,
              ::testing::ElementsAre(EnvCApi_PropertyAttributes_ReadWritable,
                                     EnvCApi_PropertyAttributes_Readable,
                                     EnvCApi_PropertyAttributes_Writable,
                                     EnvCApi_PropertyAttributes_Listable));

  Results empty;
  EXPECT_THAT(properties.ListProperty(&empty, "emptyList", callback),
              Eq(EnvCApi_PropertyResult_Success));

  EXPECT_THAT(empty.names, ::testing::IsEmpty());
  EXPECT_THAT(empty.attributes, ::testing::IsEmpty());

  Results results_denied;
  EXPECT_THAT(
      properties.ListProperty(&results_denied, "readWriteNumber", callback),
      Eq(EnvCApi_PropertyResult_PermissionDenied));
  EXPECT_THAT(results_denied.names, ::testing::IsEmpty());
  EXPECT_THAT(results_denied.attributes, ::testing::IsEmpty());

  EXPECT_THAT(
      properties.ListProperty(&results_denied, "readOnlyString", callback),
      Eq(EnvCApi_PropertyResult_PermissionDenied));
  EXPECT_THAT(results_denied.names, ::testing::IsEmpty());
  EXPECT_THAT(results_denied.attributes, ::testing::IsEmpty());

  EXPECT_THAT(properties.ListProperty(&results_denied, "writeString", callback),
              Eq(EnvCApi_PropertyResult_PermissionDenied));
  EXPECT_THAT(results_denied.names, ::testing::IsEmpty());
  EXPECT_THAT(results_denied.attributes, ::testing::IsEmpty());

  EXPECT_THAT(properties.ListProperty(&results_denied, "notExists", callback),
              Eq(EnvCApi_PropertyResult_NotFound));
}

}  // namespace
}  // namespace deepmind::lab2d
