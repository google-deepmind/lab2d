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

#include "dmlab2d/lua/class.h"

#include <array>
#include <string>
#include <utility>

#include "dmlab2d/lua/bind.h"
#include "dmlab2d/lua/call.h"
#include "dmlab2d/lua/n_results_or_test_util.h"
#include "dmlab2d/lua/push_script.h"
#include "dmlab2d/lua/read.h"
#include "dmlab2d/lua/table_ref.h"
#include "dmlab2d/lua/vm.h"
#include "dmlab2d/lua/vm_test_util.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d::lua {
namespace {

using ::deepmind::lab2d::lua::testing::IsOkAndHolds;
using ::deepmind::lab2d::lua::testing::StatusIs;
using ::testing::AllOf;
using ::testing::HasSubstr;

// Simple demo class to test and demonstrate the functionality of Class.
class Bar final : public Class<Bar> {
  friend class Class;
  static const char* ClassName() { return "system.Bar"; }

 public:
  static void Register(TableRef module, lua_State* L) {
    const Class::Reg methods[] = {};
    Class::Register(L, methods);
    module.Insert("Bar", Bar::CreateBar);
  }

  static int CreateBar(lua_State* L) {
    Class::CreateObject(L);
    return 1;
  }
};

class Foo final : public Class<Foo> {
 public:
  explicit Foo(std::string name) : name_(std::move(name)) {}

  // Foo(name): returns a new Foo object with the given name.
  static NResultsOr CreateFoo(lua_State* L) {
    std::string name;
    switch (Read(L, 1, &name).Value()) {
      case ReadResult::kFound:
        Class::CreateObject(L, std::move(name));
        return 1;
      case ReadResult::kNotFound:
        return std::string("Missing string arg1 when constructing: ") +
               ClassName();
      case ReadResult::kTypeMismatch:
      default:
        return std::string(
                   "Type missmatch arg1 is not a string when constructing: ") +
               ClassName();
    }
  }

  // Returns whatever was provided as the first argument, twice.
  NResultsOr Duplicate(lua_State* L) {
    lua_pushvalue(L, -1);
    return 2;
  }

  // Returns the name of the object.
  NResultsOr Name(lua_State* L) {
    Push(L, name_);
    return 1;
  }

  // Returns an error message.
  NResultsOr Error(lua_State* L) { return "Something went wrong!"; }

  static void Register(TableRef module, lua_State* L) {
    const Class::Reg methods[] = {
        {"duplicate", Member<&Foo::Duplicate>},  //
        {"name", Member<&Foo::Name>},            //
        {"error", Member<&Foo::Error>},          //
    };
    Class::Register(L, methods);
    module.Insert("Foo", Bind<Foo::CreateFoo>);
  }

  const std::string& name() const { return name_; }

 protected:
  friend class Class;

  static const char* ClassName() { return "system.Foo"; }

 private:
  std::string name_;
};

static int RequireFooBar(lua_State* L) {
  TableRef module = TableRef::Create(L);
  Foo::Register(module, L);
  Bar::Register(module, L);
  Push(L, module);
  return 1;
}

using ClassTest = testing::TestWithVm;

TEST_F(ClassTest, TestName) {
  TableRef module = TableRef::Create(L);
  Foo::Register(module, L);
  Push(L, "Hello");
  Foo::CreateFoo(L);
  lua_getfield(L, -1, "name");
  lua_pushvalue(L, -2);
  int error = lua_pcall(L, 1, 1, 0);
  ASSERT_EQ(error, 0) << "Reason - " << lua_tostring(L, -1);
  std::string result;
  ASSERT_TRUE(IsFound(Read(L, -1, &result)));
  EXPECT_EQ("Hello", result);
}

TEST_F(ClassTest, TestDuplicate) {
  TableRef module = TableRef::Create(L);
  Foo::Register(module, L);
  Push(L, "Hello");
  Foo::CreateFoo(L);
  lua_getfield(L, -1, "duplicate");
  lua_pushvalue(L, -2);
  Push(L, "There");
  int error = lua_pcall(L, 2, 2, 0);
  ASSERT_EQ(error, 0) << "Reason - " << lua_tostring(L, -1);
  std::string result0;
  std::string result1;
  ASSERT_TRUE(IsFound(Read(L, -1, &result0)));
  ASSERT_TRUE(IsFound(Read(L, -2, &result1)));
  EXPECT_EQ(result0, result1);
}

TEST_F(ClassTest, TestRead) {
  TableRef module = TableRef::Create(L);
  Foo::Register(module, L);
  Push(L, "Hello");
  Foo::CreateFoo(L);
  Foo* foo = Foo::ReadObject(L, -1);
  ASSERT_TRUE(foo != nullptr);
  EXPECT_EQ("Hello", foo->name());
}

constexpr char kScript2[] = R"(
local test_module = require 'test_module'
return {
    test_module.Foo('Hello'),
    test_module.Foo('Hello2'),
}
)";

TEST_F(ClassTest, TestRead2) {
  vm()->AddCModuleToSearchers("test_module", RequireFooBar);
  ASSERT_THAT(PushScript(L, kScript2, "kScript2"), IsOkAndHolds(1));
  ASSERT_THAT(Call(L, 0), IsOkAndHolds(1));
  std::array<Foo*, 2> results;
  ASSERT_TRUE(IsFound(Read(L, 1, &results)));
  EXPECT_EQ("Hello", results[0]->name());
  EXPECT_EQ("Hello2", results[1]->name());
}

constexpr char kScript[] = R"(
local test_module = require 'test_module'
return test_module.Foo('hello')
)";

TEST_F(ClassTest, TestModule) {
  vm()->AddCModuleToSearchers("test_module", RequireFooBar);
  ASSERT_THAT(PushScript(L, kScript, "kScript"), IsOkAndHolds(1));
  ASSERT_THAT(Call(L, 0), IsOkAndHolds(1));
  Foo* foo = Foo::ReadObject(L, 1);
  ASSERT_TRUE(foo != nullptr);
  EXPECT_EQ("hello", foo->name());
}

constexpr char kScriptMethodError[] = R"(
local test_module = require 'test_module'
local foo = test_module.Foo('Hello')
return foo:error()
)";

TEST_F(ClassTest, MethodErrorMessage) {
  vm()->AddCModuleToSearchers("test_module", RequireFooBar);
  ASSERT_THAT(PushScript(L, kScriptMethodError, "kScriptMethodError"),
              IsOkAndHolds(1));
  ASSERT_THAT(Call(L, 0), StatusIs(AllOf(HasSubstr("system.Foo.error"),
                                         HasSubstr("Something went wrong!"))));
}

constexpr char kScriptCallError[] = R"(
local test_module = require 'test_module'
local foo = test_module.Foo('Hello')
local bar = test_module.Bar()
-- Calling with '.'' instead of ':'
return foo.duplicate(bar)
)";

TEST_F(ClassTest, CallErrorMessage) {
  vm()->AddCModuleToSearchers("test_module", RequireFooBar);
  ASSERT_THAT(PushScript(L, kScriptCallError, "kScriptCallError"),
              IsOkAndHolds(1));
  ASSERT_THAT(Call(L, 0),
              StatusIs(AllOf(HasSubstr("system.Foo"), HasSubstr("userdata"))));
}

}  // namespace
}  // namespace deepmind::lab2d::lua
