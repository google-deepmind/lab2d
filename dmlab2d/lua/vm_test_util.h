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
// A unit testing test fixture that creates a Lua VM and exposes the
// usual "L" pointer.
//
// Example usage:
//
//    using MyWidgetTest = lua::TestWithVm;
//
//    TEST_F(MyWidgetTest, Frob) {
//      int top = lua_top(L);  // "L" is available
//      // ...
//    }

#ifndef DMLAB2D_LUA_VM_TEST_UTIL_H_
#define DMLAB2D_LUA_VM_TEST_UTIL_H_

#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/vm.h"
#include "dmlab2d/support/logging.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d::lua {
namespace testing {

class TestWithVm : public ::testing::Test {
 private:
  Vm vm_;

 protected:
  TestWithVm() : vm_(Vm::Create()), L(vm_.get()) { CHECK_EQ(lua_gettop(L), 0); }

  ~TestWithVm() override = default;

  Vm* vm() { return &vm_; }

  lua_State* const L;
};

}  // namespace testing
}  // namespace deepmind::lab2d::lua

#endif  // DMLAB2D_LUA_VM_TEST_UTIL_H_
