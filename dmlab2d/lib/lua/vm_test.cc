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

#include <vector>

#include "absl/strings/str_cat.h"
#include "dmlab2d/lib/lua/call.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/n_results_or_test_util.h"
#include "dmlab2d/lib/lua/push_script.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/lua/table_ref.h"
#include "dmlab2d/lib/util/test_srcdir.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d::lua {
namespace {

using ::deepmind::lab2d::lua::testing::IsOkAndHolds;

TEST(VmTest, TestVm) {
  std::vector<Vm> vms;

  for (int i = 0; i != 10; ++i) {
    vms.push_back(CreateVm());
  }

  for (std::size_t i = 0; i != vms.size(); ++i) {
    lua_State* L = vms[i].get();
    EXPECT_EQ(0, lua_gettop(L));
    lua_pushstring(L, "Hello");
    lua_pushnumber(L, i);
  }

  for (std::size_t i = 0; i != vms.size(); ++i) {
    lua_State* L = vms[i].get();
    EXPECT_EQ(2, lua_gettop(L));
    EXPECT_STREQ("Hello", lua_tostring(L, 1));
    EXPECT_EQ(static_cast<int>(i), lua_tointeger(L, 2));
  }
}

constexpr char kUseModule[] = R"(
local mod = require 'test.module'
return mod.hello
)";

int CModule(lua_State* L) {
  auto table = TableRef::Create(L);
  table.Insert("hello", 11);
  Push(L, table);
  return 1;
}

TEST(VmTest, TestEmbedC) {
  Vm vm = CreateVm();
  vm.AddCModuleToSearchers("test.module", CModule);
  auto* L = vm.get();

  ASSERT_THAT(PushScript(L, kUseModule, "kUseModule"), IsOkAndHolds(1))
      << "Missing script";

  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1)) << "Missing result";

  int val;
  ASSERT_TRUE(IsFound(lua::Read(L, -1, &val)));
  EXPECT_EQ(11, val);
}

int CModuleUpValue(lua_State* L) {
  int* up1 = static_cast<int*>(lua_touserdata(L, lua_upvalueindex(1)));
  int* up2 = static_cast<int*>(lua_touserdata(L, lua_upvalueindex(2)));
  auto table = TableRef::Create(L);
  table.Insert("hello", *up1 / *up2);
  Push(L, table);
  return 1;
}

TEST(VmTest, TestEmbedCClosure) {
  Vm vm = CreateVm();
  int val1 = 55;
  int val2 = 5;
  vm.AddCModuleToSearchers("test.module", CModuleUpValue, {&val1, &val2});

  auto* L = vm.get();
  ASSERT_THAT(PushScript(L, kUseModule, "kUseModule"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));

  int val;
  ASSERT_TRUE(IsFound(lua::Read(L, -1, &val)));
  EXPECT_EQ(11, val);
}

constexpr char kLuaModule[] = R"(
local mod = { hello = 11 }
return mod
)";

TEST(VmTest, TestEmbedLua) {
  Vm vm = CreateVm();
  vm.AddLuaModuleToSearchers("test.module", kLuaModule, sizeof(kLuaModule) - 1);
  auto* L = vm.get();

  ASSERT_THAT(PushScript(L, kUseModule, "kUseModule"), IsOkAndHolds(1))
      << "Missing script";

  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1)) << "Missing result";

  int val;
  ASSERT_TRUE(IsFound(lua::Read(L, -1, &val)));
  EXPECT_EQ(11, val);
}

constexpr char kUsePath[] = R"(
local mod = require 'module'
return mod.hello
)";

TEST(VmTest, TestLuaPath) {
  Vm vm = CreateVm();
  vm.AddPathToSearchers(absl::StrCat(
      util::TestSrcDir(), "/org_deepmind_lab2d/dmlab2d/lib/lua/vm_test_data"));

  auto* L = vm.get();

  ASSERT_THAT(PushScript(L, kUsePath, "kUsePath"), IsOkAndHolds(1))
      << "Missing script";

  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1)) << "Missing result";

  int val;
  ASSERT_TRUE(IsFound(lua::Read(L, -1, &val)));
  EXPECT_EQ(11, val);
}

constexpr char kUsePathInit[] = R"(
local mod = require 'module_as_dir'
return mod.hello
)";

TEST(VmTest, TestLuaPathInit) {
  Vm vm = CreateVm();
  vm.AddPathToSearchers(absl::StrCat(
      util::TestSrcDir(), "/org_deepmind_lab2d/dmlab2d/lib/lua/vm_test_data"));

  auto* L = vm.get();

  ASSERT_THAT(PushScript(L, kUsePathInit, "kUsePathInit"), IsOkAndHolds(1))
      << "Missing script";

  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1)) << "Missing result";

  int val;
  ASSERT_TRUE(IsFound(lua::Read(L, -1, &val)));
  EXPECT_EQ(12, val);
}

}  // namespace
}  // namespace deepmind::lab2d::lua
