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

#include "dmlab2d/lib/system/tensor/lua/tensor.h"

#include <cstdint>
#include <cstring>
#include <fstream>
#include <random>
#include <tuple>

#include "absl/strings/string_view.h"
#include "dmlab2d/lib/lua/bind.h"
#include "dmlab2d/lib/lua/call.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/n_results_or_test_util.h"
#include "dmlab2d/lib/lua/push_script.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/lua/table_ref.h"
#include "dmlab2d/lib/lua/vm.h"
#include "dmlab2d/lib/lua/vm_test_util.h"
#include "dmlab2d/lib/util/default_read_only_file_system.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {

using ::deepmind::lab2d::lua::testing::IsOkAndHolds;
using ::deepmind::lab2d::lua::testing::StatusIs;
using ::testing::AllOf;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::HasSubstr;
using ::testing::IsEmpty;

class LuaTensorTest : public lua::testing::TestWithVm {
 protected:
  LuaTensorTest() {
    LuaRandom::Register(L);
    void* default_fs = const_cast<DeepMindReadOnlyFileSystem*>(
        util::DefaultReadOnlyFileSystem());
    vm()->AddCModuleToSearchers(
        "system.sys_random", &lua::Bind<LuaRandom::Require>,
        {&prbg_, reinterpret_cast<void*>(static_cast<std::uintptr_t>(0))});
    tensor::LuaTensorRegister(L);
    vm()->AddCModuleToSearchers("system.tensor", tensor::LuaTensorConstructors,
                                {default_fs});
  }
  std::mt19937_64 prbg_;
};

constexpr absl::string_view kLuaTensorCreate = R"(
local tensor = require 'system.tensor'
local tensor_types = {
    "ByteTensor",
    "CharTensor",
    "Int16Tensor",
    "Int32Tensor",
    "Int64Tensor",
    "FloatTensor",
    "DoubleTensor"
}

local result = {}
for i, tensor_type in ipairs(tensor_types) do
  result[tensor_type] = tensor[tensor_type](3, 3)
  local k = 0
  result[tensor_type]:apply(function(val)
    assert(val == 0)
    k = k + 1
    return k
  end)
  assert(result[tensor_type]:type() == "tensor." .. tensor_type)
end
return result
)";

TEST_F(LuaTensorTest, CreateTensor) {
  ASSERT_THAT(lua::PushScript(L, kLuaTensorCreate, "kLuaTensorCreate"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  {
    lua::TableRef tableref;
    ASSERT_TRUE(lua::Read(L, -1, &tableref));
    lua_pop(L, 1);
    tableref.LookUpToStack("DoubleTensor");
  }
  auto* tensor_double = tensor::LuaTensor<double>::ReadObject(L, -1);
  int counter = 0;
  tensor_double->tensor_view().ForEach([&counter](double val) {
    ++counter;
    EXPECT_DOUBLE_EQ(counter, val);
    return true;
  });
}

constexpr absl::string_view kLuaTensorCreateScalar = R"(
local tensor = require 'system.tensor'
local tensor_types = {
    "ByteTensor",
    "CharTensor",
    "Int16Tensor",
    "Int32Tensor",
    "Int64Tensor",
    "FloatTensor",
    "DoubleTensor"
}

local result = {}
for i, tensor_type in ipairs(tensor_types) do
  result[tensor_type] = tensor[tensor_type]()
  assert(result[tensor_type]:val() == 0)
  result[tensor_type]:val(i)
end
return result
)";

TEST_F(LuaTensorTest, CreateTensorScalar) {
  ASSERT_THAT(
      lua::PushScript(L, kLuaTensorCreateScalar, "kLuaTensorCreateScalar"),
      IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef tableref;
  ASSERT_TRUE(lua::Read(L, -1, &tableref));
  lua_pop(L, 1);

  tensor::LuaTensor<std::uint8_t>* uint8_tensor = nullptr;
  ASSERT_TRUE(IsFound(tableref.LookUp("ByteTensor", &uint8_tensor)));
  EXPECT_THAT(uint8_tensor->tensor_view().shape(), IsEmpty());
  EXPECT_THAT(uint8_tensor->tensor_view().num_elements(), Eq(1));

  tensor::LuaTensor<std::int8_t>* int8_tensor = nullptr;
  ASSERT_TRUE(IsFound(tableref.LookUp("CharTensor", &int8_tensor)));
  EXPECT_THAT(int8_tensor->tensor_view().shape(), IsEmpty());
  EXPECT_THAT(int8_tensor->tensor_view().num_elements(), Eq(1));

  tensor::LuaTensor<std::int16_t>* int16_tensor = nullptr;
  ASSERT_TRUE(IsFound(tableref.LookUp("Int16Tensor", &int16_tensor)));
  EXPECT_THAT(int16_tensor->tensor_view().shape(), IsEmpty());
  EXPECT_THAT(int16_tensor->tensor_view().num_elements(), Eq(1));

  tensor::LuaTensor<std::int32_t>* int32_tensor = nullptr;
  ASSERT_TRUE(IsFound(tableref.LookUp("Int32Tensor", &int32_tensor)));
  EXPECT_THAT(int32_tensor->tensor_view().shape(), IsEmpty());
  EXPECT_THAT(int32_tensor->tensor_view().num_elements(), Eq(1));

  tensor::LuaTensor<std::int64_t>* int64_tensor = nullptr;
  ASSERT_TRUE(IsFound(tableref.LookUp("Int64Tensor", &int64_tensor)));
  EXPECT_THAT(int64_tensor->tensor_view().shape(), IsEmpty());
  EXPECT_THAT(int64_tensor->tensor_view().num_elements(), Eq(1));

  tensor::LuaTensor<float>* float_tensor = nullptr;
  ASSERT_TRUE(IsFound(tableref.LookUp("FloatTensor", &float_tensor)));
  EXPECT_THAT(float_tensor->tensor_view().shape(), IsEmpty());
  EXPECT_THAT(float_tensor->tensor_view().num_elements(), Eq(1));

  tensor::LuaTensor<double>* double_tensor = nullptr;
  ASSERT_TRUE(IsFound(tableref.LookUp("DoubleTensor", &double_tensor)));
  EXPECT_THAT(double_tensor->tensor_view().shape(), IsEmpty());
  EXPECT_THAT(double_tensor->tensor_view().num_elements(), Eq(1));
}

constexpr absl::string_view kLuaTensorCreateEmpty = R"(
local tensor = require 'system.tensor'
local tensor_types = {
    "ByteTensor",
    "CharTensor",
    "Int16Tensor",
    "Int32Tensor",
    "Int64Tensor",
    "FloatTensor",
    "DoubleTensor"
}

local result = {}
for i, tensor_type in ipairs(tensor_types) do
  result[tensor_type] = tensor[tensor_type](0)
end
return result
)";

TEST_F(LuaTensorTest, CreateTensorEmpty) {
  ASSERT_THAT(
      lua::PushScript(L, kLuaTensorCreateEmpty, "kLuaTensorCreateEmpty"),
      IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef tableref;
  ASSERT_TRUE(lua::Read(L, -1, &tableref));
  lua_pop(L, 1);

  tensor::LuaTensor<std::uint8_t>* uint8_tensor = nullptr;
  ASSERT_TRUE(IsFound(tableref.LookUp("ByteTensor", &uint8_tensor)));
  EXPECT_THAT(uint8_tensor->tensor_view().shape(), ElementsAre(0));
  EXPECT_THAT(uint8_tensor->tensor_view().num_elements(), Eq(0));

  tensor::LuaTensor<std::int8_t>* int8_tensor = nullptr;
  ASSERT_TRUE(IsFound(tableref.LookUp("CharTensor", &int8_tensor)));
  EXPECT_THAT(int8_tensor->tensor_view().shape(), ElementsAre(0));
  EXPECT_THAT(int8_tensor->tensor_view().num_elements(), Eq(0));

  tensor::LuaTensor<std::int16_t>* int16_tensor = nullptr;
  ASSERT_TRUE(IsFound(tableref.LookUp("Int16Tensor", &int16_tensor)));
  EXPECT_THAT(int16_tensor->tensor_view().shape(), ElementsAre(0));
  EXPECT_THAT(int16_tensor->tensor_view().num_elements(), Eq(0));

  tensor::LuaTensor<std::int32_t>* int32_tensor = nullptr;
  ASSERT_TRUE(IsFound(tableref.LookUp("Int32Tensor", &int32_tensor)));
  EXPECT_THAT(int32_tensor->tensor_view().shape(), ElementsAre(0));
  EXPECT_THAT(int32_tensor->tensor_view().num_elements(), Eq(0));

  tensor::LuaTensor<std::int64_t>* int64_tensor = nullptr;
  ASSERT_TRUE(IsFound(tableref.LookUp("Int64Tensor", &int64_tensor)));
  EXPECT_THAT(int64_tensor->tensor_view().shape(), ElementsAre(0));
  EXPECT_THAT(int64_tensor->tensor_view().num_elements(), Eq(0));

  tensor::LuaTensor<float>* float_tensor = nullptr;
  ASSERT_TRUE(IsFound(tableref.LookUp("FloatTensor", &float_tensor)));
  EXPECT_THAT(float_tensor->tensor_view().shape(), ElementsAre(0));
  EXPECT_THAT(float_tensor->tensor_view().num_elements(), Eq(0));

  tensor::LuaTensor<double>* double_tensor = nullptr;
  ASSERT_TRUE(IsFound(tableref.LookUp("DoubleTensor", &double_tensor)));
  EXPECT_THAT(double_tensor->tensor_view().shape(), ElementsAre(0));
  EXPECT_THAT(double_tensor->tensor_view().num_elements(), Eq(0));
}

constexpr absl::string_view kLuaTensorRange = R"(
local tensor = require 'system.tensor'

local v0, v1, v2, v3, v4
v0 = tensor.DoubleTensor{range = {5}}
v1 = tensor.DoubleTensor{range = {2, 5}}
v2 = tensor.DoubleTensor{range = {2, 5, 0.5}}
v3 = tensor.DoubleTensor{range = {2, 4.9, 0.5}}
v4 = tensor.DoubleTensor{range = {2, -1, -1}}
assert(v0 == tensor.DoubleTensor{1, 2, 3, 4, 5})
assert(v1 == tensor.DoubleTensor{2, 3, 4, 5})
assert(v2 == tensor.DoubleTensor{2, 2.5, 3, 3.5, 4, 4.5, 5})
assert(v3 == tensor.DoubleTensor{2, 2.5, 3, 3.5, 4, 4.5})
assert(v4 == tensor.DoubleTensor{2, 1, 0, -1})
)";

TEST_F(LuaTensorTest, TensorRange) {
  ASSERT_THAT(lua::PushScript(L, kLuaTensorRange, "kLuaTensorRange"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kInvalidLuaTensorRange = R"(
local tensor = require 'system.tensor'

local v0 = tensor.DoubleTensor{range={1, -5}}
)";

TEST_F(LuaTensorTest, InvalidLuaTensorRange) {
  ASSERT_THAT(lua::PushScript(L, kInvalidLuaTensorRange,

                              "kInvalidLuaTensorRange"),
              IsOkAndHolds(1));
  auto result = lua::Call(L, 0);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.error(), HasSubstr("Invalid Tensor range."));
}

constexpr absl::string_view kValueOps = R"(
local tensor = require 'system.tensor'

local v0, v1
v0 = tensor.ByteTensor{{1, 2}, {3, 4}}
assert(v0:select(1, 1) == tensor.ByteTensor{1, 2})
assert(v0:select(1, 2) == tensor.ByteTensor{3, 4})
assert(v0:select(2, 1) == tensor.ByteTensor{1, 3})
assert(v0:select(2, 2) == tensor.ByteTensor{2, 4})
v0 = tensor.ByteTensor(3, 3):fill(1):add(1)
v1 = tensor.ByteTensor(3, 3):fill(3)
assert(v0 ~= v1)
v0:add(1)
assert(v0 == v1)
v0 = tensor.ByteTensor{{1, 2}, {3, 4}}
local k = 0
v1 = tensor.ByteTensor(2,2):apply(function(val)
    k = k + 1
    return k
  end)
assert(v0 == v1)
assert(v0(1, 1):val() == 1)
assert(v0(1, 2):val() == 2)
assert(v0(2, 1):val() == 3)
assert(v0(2, 2):val() == 4)
)";

TEST_F(LuaTensorTest, ValueOps) {
  ASSERT_THAT(lua::PushScript(L, kValueOps, "kValueOps"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kShape = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local shape = bt:shape()
assert(shape[1] == 3)
assert(shape[2] == 2)
assert(#shape == 2)
)";

TEST_F(LuaTensorTest, Shape) {
  ASSERT_THAT(lua::PushScript(L, kShape, "kShape"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kClone = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
return bt, bt:clone()
)";

TEST_F(LuaTensorTest, Clone) {
  ASSERT_THAT(lua::PushScript(L, kClone, "kClone"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  auto* bt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -2);
  auto* bt_clone = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -1);
  ASSERT_TRUE(bt != nullptr);
  ASSERT_TRUE(bt_clone != nullptr);
  EXPECT_TRUE(bt->tensor_view().storage() != bt_clone->tensor_view().storage());
  EXPECT_TRUE(bt->tensor_view() == bt_clone->tensor_view());
}

constexpr absl::string_view kSum = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}}
return bt:sum()
)";

TEST_F(LuaTensorTest, Sum) {
  ASSERT_THAT(lua::PushScript(L, kSum, "kSum"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  double sum;
  ASSERT_TRUE(IsFound(lua::Read(L, 1, &sum)));
  EXPECT_EQ(sum, 10);
}

constexpr absl::string_view kProduct = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}}
return bt:product()
)";

TEST_F(LuaTensorTest, Product) {
  ASSERT_THAT(lua::PushScript(L, kProduct, "kProduct"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  double prod;
  ASSERT_TRUE(IsFound(lua::Read(L, 1, &prod)));
  EXPECT_EQ(prod, 24.0);
}

constexpr absl::string_view kLengthSquared = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}}
return bt:lengthSquared()
)";

TEST_F(LuaTensorTest, LengthSquared) {
  ASSERT_THAT(lua::PushScript(L, kLengthSquared, "kLengthSquared"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  double length_sqr;
  ASSERT_TRUE(IsFound(lua::Read(L, 1, &length_sqr)));
  EXPECT_EQ(length_sqr, 1 + 4 + 9 + 16);
}

constexpr absl::string_view kDotProduct = R"(
local tensor = require 'system.tensor'
local t1 = tensor.Int32Tensor{1, 2, 3, 4}
local t2 = tensor.Int32Tensor{-1, 2, -3, 4}
return t1:dot(t2)
)";

TEST_F(LuaTensorTest, DotProduct) {
  ASSERT_THAT(lua::PushScript(L, kDotProduct, "kDotProduct"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  double result;
  ASSERT_TRUE(IsFound(lua::Read(L, 1, &result)));
  EXPECT_EQ(result, -1 + 4 - 9 + 16);
}

constexpr absl::string_view kTranspose = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local transpose = tensor.ByteTensor{{1, 3, 5}, {2, 4, 6}}
assert(bt:transpose(1, 2) == transpose)
return bt, bt:transpose(1, 2)
)";

TEST_F(LuaTensorTest, Transpose) {
  ASSERT_THAT(lua::PushScript(L, kTranspose, "kTranspose"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  auto* bt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -2);
  auto* bt_alt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -1);
  ASSERT_TRUE(bt != nullptr);
  ASSERT_TRUE(bt_alt != nullptr);
  EXPECT_TRUE(bt->tensor_view().storage() == bt_alt->tensor_view().storage());
}

constexpr absl::string_view kIsContiguous = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local transpose = bt:transpose(1, 2)
assert(bt:isContiguous())
assert(not transpose:isContiguous())
transpose:clone():isContiguous()
)";

TEST_F(LuaTensorTest, IsContiguous) {
  ASSERT_THAT(lua::PushScript(L, kIsContiguous, "kIsContiguous"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kSelect = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local select = tensor.ByteTensor{2, 4, 6}
assert (bt:select(2, 2) == select)
return bt, bt:select(2, 2)
)";

TEST_F(LuaTensorTest, Select) {
  ASSERT_THAT(lua::PushScript(L, kSelect, "kSelect"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  auto* bt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -2);
  auto* bt_alt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -1);
  ASSERT_TRUE(bt != nullptr);
  ASSERT_TRUE(bt_alt != nullptr);
  EXPECT_TRUE(bt->tensor_view().storage() == bt_alt->tensor_view().storage());
}

constexpr absl::string_view kNarrow = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local narrow = tensor.ByteTensor{{3, 4}, {5, 6}}
assert (bt:narrow(1, 2, 2) == narrow)
return bt, bt:narrow(1, 2, 2)
)";

TEST_F(LuaTensorTest, Narrow) {
  ASSERT_THAT(lua::PushScript(L, kNarrow, "kNarrow"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  auto* bt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -2);
  auto* bt_alt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -1);
  ASSERT_TRUE(bt != nullptr);
  ASSERT_TRUE(bt_alt != nullptr);
  EXPECT_TRUE(bt->tensor_view().storage() == bt_alt->tensor_view().storage());
}

constexpr absl::string_view kReverse = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local narrow = tensor.ByteTensor{{3, 4}, {5, 6}}
assert (bt:reverse(1) == tensor.ByteTensor{{5, 6}, {3, 4}, {1, 2}})
assert (bt:reverse(2) == tensor.ByteTensor{{2, 1}, {4, 3}, {6, 5}})
assert (bt:reverse(2):reverse(1) == tensor.ByteTensor{{6, 5}, {4, 3}, {2, 1}})
)";

TEST_F(LuaTensorTest, Reverse) {
  ASSERT_THAT(lua::PushScript(L, kReverse, "kReverse"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kApply = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local btApply = bt:apply(function(val) return val + 3 end)
local apply = tensor.ByteTensor{{4, 5}, {6, 7}, {8, 9}}
assert (btApply == apply)
)";

TEST_F(LuaTensorTest, Apply) {
  ASSERT_THAT(lua::PushScript(L, kApply, "kApply"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kApplyIndexed = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local btApply = bt:applyIndexed(function(val, index)
  return index[1] * index[2] + val
end)
local apply = tensor.ByteTensor{{2, 4}, {5, 8}, {8, 12}}
assert (btApply == apply)
)";

TEST_F(LuaTensorTest, ApplyIndexed) {
  ASSERT_THAT(lua::PushScript(L, kApplyIndexed, "kApplyIndexed"),
              IsOkAndHolds(1));

  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kFill = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local btApply = bt:fill(10)
local apply = tensor.ByteTensor{{10, 10}, {10, 10}, {10, 10}}
assert (btApply == apply)
)";

TEST_F(LuaTensorTest, Fill) {
  ASSERT_THAT(lua::PushScript(L, kFill, "kFill"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kFillTable = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local btApply = bt:fill{10, 12}
local apply = tensor.ByteTensor{{10, 12}, {10, 12}, {10, 12}}
assert (btApply == apply)
)";

TEST_F(LuaTensorTest, FillTable) {
  ASSERT_THAT(lua::PushScript(L, kFillTable, "kFillTable"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kVal = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor(3, 3)
assert(bt(2, 2):val() == 0)
assert(bt(2, 2):val(10) == 10)
assert (bt == tensor.ByteTensor{{0, 0, 0}, {0, 10, 0}, {0, 0, 0}})
)";

TEST_F(LuaTensorTest, kVal) {
  ASSERT_THAT(lua::PushScript(L, kVal, "kVal"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kScalarOp = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
assert (bt:mul(2) == tensor.ByteTensor{{2, 4}, {6, 8}, {10, 12}})
assert (bt:add(2) == tensor.ByteTensor{{4, 6}, {8, 10}, {12, 14}})
assert (bt:div(2) == tensor.ByteTensor{{2, 3}, {4, 5}, {6, 7}})
assert (bt:sub(2) == tensor.ByteTensor{{0, 1}, {2, 3}, {4, 5}})
)";

TEST_F(LuaTensorTest, kScalarOp) {
  ASSERT_THAT(lua::PushScript(L, kScalarOp, "kScalarOp"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kScalarOpTable = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
assert (bt:mul{2, 4} == tensor.ByteTensor{{2, 8}, {6, 16}, {10, 24}}, "1")
assert (bt:add{2, 4} == tensor.ByteTensor{{4, 12}, {8, 20}, {12, 28}}, "2")
assert (bt:div{2, 4} == tensor.ByteTensor{{2, 3}, {4, 5}, {6, 7}}, "3")
assert (bt:sub{2, 3} == tensor.ByteTensor{{0, 0}, {2, 2}, {4, 4}}, "4")
)";

TEST_F(LuaTensorTest, kScalarOpTable) {
  ASSERT_THAT(lua::PushScript(L, kScalarOpTable, "kScalarOpTable"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kComponentOp = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local bt2 = tensor.ByteTensor{{1, 2, 3}, {4, 5, 6}}
assert (bt:cmul(bt2) == tensor.ByteTensor{{1, 4}, {9, 16}, {25, 36}})
assert (bt:cadd(bt2) == tensor.ByteTensor{{2, 6}, {12, 20}, {30, 42}})
assert (bt:cdiv(bt2) == tensor.ByteTensor{{2, 3}, {4, 5}, {6, 7}})
assert (bt:csub(bt2) == tensor.ByteTensor{{1, 1}, {1, 1}, {1, 1}})
assert (bt:copy(bt2) == tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}})
)";

TEST_F(LuaTensorTest, kComponentOp) {
  ASSERT_THAT(lua::PushScript(L, kComponentOp, "kComponentOp"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kConvertOp = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
assert (bt:byte() == tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}})
assert (bt:char() == tensor.CharTensor{{1, 2}, {3, 4}, {5, 6}})
assert (bt:int16() == tensor.Int16Tensor{{1, 2}, {3, 4}, {5, 6}})
assert (bt:int32() == tensor.Int32Tensor{{1, 2}, {3, 4}, {5, 6}})
assert (bt:int64() == tensor.Int64Tensor{{1, 2}, {3, 4}, {5, 6}})
assert (bt:float() == tensor.FloatTensor{{1, 2}, {3, 4}, {5, 6}})
assert (bt:double() == tensor.DoubleTensor{{1, 2}, {3, 4}, {5, 6}})
)";

TEST_F(LuaTensorTest, kConvertOp) {
  ASSERT_THAT(lua::PushScript(L, kConvertOp, "kConvertOp"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kTestValid = R"(
local tensor = require 'system.tensor'
local api = {}
function api.readTensor(tensor)
  api._tensorWillBeInvalid = tensor
  api._tensorValid = tensor:clone()
  assert(api._tensorValid:ownsStorage() == true)
  assert(api._tensorWillBeInvalid:ownsStorage() == false)
  assert(api._tensorValid(1, 1):val() == 10)
  assert(api._tensorWillBeInvalid(1, 1):val() == 10)
end

function api.testValid()
  return api._tensorValid(1, 1):val() == 10
end

function api.testInvalid(tensor)
  return api._tensorWillBeInvalid(0, 0):val() == 10
end

return api
)";

TEST_F(LuaTensorTest, Invalidate) {
  ASSERT_THAT(lua::PushScript(L, kTestValid, "kTestValid"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef table_ref;
  ASSERT_TRUE(IsFound(lua::Read(L, -1, &table_ref)));
  lua_pop(L, 1);
  table_ref.PushFunction("readTensor");
  {
    std::vector<double> doubles(4, 10.0);
    auto shared = std::make_shared<tensor::StorageValidity>();
    tensor::LuaTensor<double>::CreateObject(
        L, tensor::TensorView<double>(tensor::Layout({2, 2}), doubles.data()),
        shared);
    ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(0));
    shared->Invalidate();
  }
  table_ref.PushFunction("testValid");
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  bool bool_result;
  ASSERT_TRUE(IsFound(lua::Read(L, -1, &bool_result)));
  EXPECT_EQ(true, bool_result);
  lua_pop(L, 1);

  table_ref.PushFunction("testInvalid");
  auto result = lua::Call(L, 0);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.error(), HasSubstr("invalid"));
}

constexpr absl::string_view kTestToString = R"(
local tensor = require 'system.tensor'
local data = '123'
local empty = ''
return tensor.ByteTensor{data:byte(1, -1)}, tensor.ByteTensor{empty:byte(1, -1)}
)";

TEST_F(LuaTensorTest, kTestToString) {
  ASSERT_THAT(lua::PushScript(L, kTestToString, "kTestToString"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  auto* bt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -2);
  EXPECT_THAT(std::make_tuple(bt->tensor_view().storage(),
                              bt->tensor_view().num_elements()),
              ElementsAre('1', '2', '3'));

  auto* bt_alt = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -1);
  EXPECT_EQ(0, bt_alt->tensor_view().num_elements());
}

constexpr absl::string_view kMMulOpLHSNonMatrix = R"(
local tensor = require 'system.tensor'
local at = tensor.FloatTensor(2, 2, 2)
local bt = tensor.FloatTensor(2, 3)
return at:mmul(bt)
)";

TEST_F(LuaTensorTest, kMMulOpLHSNonMatrix) {
  ASSERT_THAT(lua::PushScript(L, kMMulOpLHSNonMatrix, "kMMulOpLHSNonMatrix"),
              IsOkAndHolds(1));
  auto result = lua::Call(L, 0);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.error(), HasSubstr("LHS is not a matrix"));
}

constexpr absl::string_view kMMulOpRHSNonMatrix = R"(
local tensor = require 'system.tensor'
local at = tensor.FloatTensor(2, 2, 2)
local bt = tensor.FloatTensor(2, 3)
return bt:mmul(at)
)";

TEST_F(LuaTensorTest, kMMulOpRHSNonMatrix) {
  ASSERT_THAT(lua::PushScript(L, kMMulOpRHSNonMatrix, "kMMulOpRHSNonMatrix"),
              IsOkAndHolds(1));
  auto result = lua::Call(L, 0);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.error(), HasSubstr("RHS is not a matrix"));
}

constexpr absl::string_view kMMulOpIncompatibleDims = R"(
local tensor = require 'system.tensor'
local at = tensor.FloatTensor(2, 3)
local bt = tensor.FloatTensor(2, 2)
return at:mmul(bt)
)";

TEST_F(LuaTensorTest, kMMulOpIncompatibleDims) {
  ASSERT_THAT(lua::PushScript(L, kMMulOpIncompatibleDims,

                              "kMMulOpIncompatibleDims"),
              IsOkAndHolds(1));
  auto result = lua::Call(L, 0);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(result.error(), HasSubstr("Incorrect matrix dimensions"));
}

constexpr absl::string_view kMMulOpIncompatibleType = R"(
local tensor = require 'system.tensor'
local at = tensor.FloatTensor(2, 2)
local bt = tensor.ByteTensor(2, 2)
return at:mmul(bt)
)";

TEST_F(LuaTensorTest, kMMulOpIncompatibleType) {
  ASSERT_THAT(lua::PushScript(L, kMMulOpIncompatibleType,

                              "kMMulOpIncompatibleType"),
              IsOkAndHolds(1));
  auto result = lua::Call(L, 0);
  EXPECT_FALSE(result.ok());
  EXPECT_THAT(
      result.error(),
      HasSubstr("Must contain 1 RHS tensor of type tensor.FloatTensor"));
}

constexpr absl::string_view kRoundingOps = R"(
local tensor = require 'system.tensor'
local bt = tensor.DoubleTensor{{-2.25, -1.75}, {0.5, 1.0}}
assert (bt:clone():floor() == tensor.DoubleTensor{{-3.0, -2.0}, {0.0, 1.0}})
assert (bt:clone():ceil() == tensor.DoubleTensor{{-2.0, -1.0}, {1.0, 1.0}})
assert (bt:clone():round() == tensor.DoubleTensor{{-2.0, -2.0}, {1.0, 1.0}})
)";

TEST_F(LuaTensorTest, kRoundingOps) {
  ASSERT_THAT(lua::PushScript(L, kRoundingOps, "kRoundingOps"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kShuffle = R"(
local sys_random = require 'system.sys_random'
local tensor = require 'system.tensor'

sys_random:seed(123)
local at = tensor.Int64Tensor{range={5}}:shuffle(sys_random)
local bt = tensor.Tensor{0, 0, 0, 0, 0}
for i = 1,5 do
  local j = at(i):val()
  bt(j):val(1)
end
assert (bt == tensor.Tensor{1, 1, 1, 1, 1})
local ct = tensor.Tensor{0}:narrow(1, 1, 0):shuffle(sys_random)
assert (ct:shape()[1] == 0)
)";

TEST_F(LuaTensorTest, kShuffle) {
  ASSERT_THAT(lua::PushScript(L, kShuffle, "kShuffle"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kReshape = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
assert(bt:reshape{6} == tensor.ByteTensor{1, 2, 3, 4, 5, 6})
assert(bt:reshape{2, 3} == tensor.ByteTensor{{1, 2, 3}, {4, 5, 6}})
assert(bt:reshape{6, 1} == tensor.ByteTensor{{1}, {2}, {3}, {4}, {5}, {6}})
assert(bt:reshape{1, 6} == tensor.ByteTensor{{1, 2, 3, 4, 5, 6}})
)";

TEST_F(LuaTensorTest, Reshape) {
  ASSERT_THAT(lua::PushScript(L, kReshape, "kReshape"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kValTableRead = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
local asTable = bt:val()
for i = 1, 3 do
  for j = 1, 2 do
    assert(asTable[i][j] == (i - 1) * 2 + j)
  end
end
)";

TEST_F(LuaTensorTest, kValTableRead) {
  ASSERT_THAT(lua::PushScript(L, kValTableRead, "kValTableRead"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kValTableWrite = R"(
local tensor = require 'system.tensor'
local bt = tensor.ByteTensor{{1, 2}, {3, 4}, {5, 6}}
bt(1):val{1, 1}
bt(2):val{2, 2}
bt(3):val{3, 3}
for i = 1, 3 do
  for j = 1, 2 do
    assert(bt(i, j):val() == i)
  end
end
)";

TEST_F(LuaTensorTest, kValTableWrite) {
  ASSERT_THAT(lua::PushScript(L, kValTableWrite, "kValTableWrite"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

template <typename T>
void CreateFileWith64IncrementingValues(const std::string& filename) {
  std::ofstream ofs(filename, std::ios::binary);
  for (T val = 0; val < 64; ++val) {
    ofs.write(reinterpret_cast<const char*>(&val), sizeof val);
  }
}

std::string CreateFourRawFiles() {
  std::string temp_dir = testing::TempDir() + "/";
  CreateFileWith64IncrementingValues<unsigned char>(temp_dir + "bytes.bin");
  CreateFileWith64IncrementingValues<double>(temp_dir + "doubles.bin");
  CreateFileWith64IncrementingValues<std::int64_t>(temp_dir + "int64s.bin");
  CreateFileWith64IncrementingValues<float>(temp_dir + "floats.bin");
  return temp_dir;
}

std::string CreateBytesRawFile() {
  std::string temp_dir = testing::TempDir() + "/";
  CreateFileWith64IncrementingValues<unsigned char>(temp_dir + "bytes.bin");
  return temp_dir;
}

constexpr absl::string_view kLoadWholeFile = R"(
local tensor = require 'system.tensor'
local path = ...
assert(tensor.ByteTensor{file = {name = path .. 'bytes.bin'}}
       == tensor.ByteTensor{range = {0, 63}})
assert(tensor.DoubleTensor{file = {name = path .. 'doubles.bin'}}
       == tensor.DoubleTensor{range = {0, 63}})
assert(tensor.Int64Tensor{file = {name = path .. 'int64s.bin'}}
       == tensor.Int64Tensor{range = {0, 63}})
assert(tensor.FloatTensor{file = {name = path .. 'floats.bin'}}
       == tensor.FloatTensor{range = {0, 63}})
)";

TEST_F(LuaTensorTest, kLoadWholeFile) {
  ASSERT_THAT(lua::PushScript(L, kLoadWholeFile, "kLoadWholeFile"),
              IsOkAndHolds(1));
  lua::Push(L, CreateFourRawFiles());
  ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(0));
}

constexpr absl::string_view kLoadStartFile = R"(
local tensor = require 'system.tensor'
local path = ...
assert(tensor.ByteTensor{file =
           {name = path .. 'bytes.bin', numElements = 10}
       } == tensor.ByteTensor{range = {0, 9}})
assert(tensor.DoubleTensor{file =
           {name = path .. 'doubles.bin', numElements = 10}
       } == tensor.DoubleTensor{range = {0, 9}})
assert(tensor.Int64Tensor{file =
           {name = path .. 'int64s.bin', numElements = 10}
       } == tensor.Int64Tensor{range = {0, 9}})
assert(tensor.FloatTensor{file =
           {name = path .. 'floats.bin', numElements = 10}
       } == tensor.FloatTensor{range = {0, 9}})
)";

TEST_F(LuaTensorTest, kLoadStartFile) {
  ASSERT_THAT(lua::PushScript(L, kLoadStartFile, "kLoadStartFile"),
              IsOkAndHolds(1));
  lua::Push(L, CreateFourRawFiles());
  ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(0));
}

constexpr absl::string_view kLoadEndFile = R"(
local tensor = require 'system.tensor'
local path = ...
assert(tensor.ByteTensor{file =
           {name = path .. 'bytes.bin', byteOffset = 40 * 1}
       } == tensor.ByteTensor{range = {40, 63}})
assert(tensor.DoubleTensor{file =
           {name = path .. 'doubles.bin', byteOffset = 40 * 8}
       } == tensor.DoubleTensor{range = {40, 63}})
assert(tensor.Int64Tensor{file =
           {name = path .. 'int64s.bin', byteOffset = 40 * 8}
       } == tensor.Int64Tensor{range = {40, 63}})
assert(tensor.FloatTensor{file =
           {name = path .. 'floats.bin', byteOffset = 40 * 4}
       } == tensor.FloatTensor{range = {40, 63}})
)";

TEST_F(LuaTensorTest, kLoadEndFile) {
  ASSERT_THAT(lua::PushScript(L, kLoadEndFile, "kLoadEndFile"),
              IsOkAndHolds(1));
  lua::Push(L, CreateFourRawFiles());
  ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(0));
}

constexpr absl::string_view kLoadMiddleFile = R"(
local tensor = require 'system.tensor'
local path = ...
assert(tensor.ByteTensor{file =
           {name = path .. 'bytes.bin', byteOffset = 40 * 1, numElements = 6}
       } == tensor.ByteTensor{range = {40, 45}})
assert(tensor.DoubleTensor{file =
           {name = path .. 'doubles.bin', byteOffset = 40 * 8, numElements = 6}
       } == tensor.DoubleTensor{range = {40, 45}})
assert(tensor.Int64Tensor{file =
           {name = path .. 'int64s.bin', byteOffset = 40 * 8, numElements = 6}
       } == tensor.Int64Tensor{range = {40, 45}})
assert(tensor.FloatTensor{file =
           {name = path .. 'floats.bin', byteOffset = 40 * 4, numElements = 6}
       } == tensor.FloatTensor{range = {40, 45}})
)";

TEST_F(LuaTensorTest, kLoadMiddleFile) {
  ASSERT_THAT(lua::PushScript(L, kLoadMiddleFile, "kLoadMiddleFile"),
              IsOkAndHolds(1));
  lua::Push(L, CreateFourRawFiles());
  ASSERT_THAT(lua::Call(L, 1), IsOkAndHolds(0));
}

constexpr absl::string_view kBadFileName = R"(
local tensor = require 'system.tensor'
local path = ...
return tensor.ByteTensor{file = {name = path .. 'bad_file.bin'}}
)";

TEST_F(LuaTensorTest, kBadFileName) {
  std::string temp_dir = testing::TempDir() + "/";
  ASSERT_THAT(lua::PushScript(L, kBadFileName, "kBadFileName"),
              IsOkAndHolds(1));
  lua::Push(L, CreateBytesRawFile());
  ASSERT_THAT(lua::Call(L, 1), StatusIs(HasSubstr("bad_file.bin")));
}

constexpr absl::string_view kBadNumElements = R"(
local tensor = require 'system.tensor'
local path = ...
return tensor.ByteTensor{file = {name = path .. 'bytes.bin', numElements = 65}}
)";

TEST_F(LuaTensorTest, kBadNumElements) {
  ASSERT_THAT(lua::PushScript(L, kBadNumElements, "kBadNumElements"),
              IsOkAndHolds(1));
  lua::Push(L, CreateBytesRawFile());
  ASSERT_THAT(lua::Call(L, 1),
              StatusIs(AllOf(HasSubstr("numElements"), HasSubstr("65"))));
}

constexpr absl::string_view kBadNumElementsOffset = R"(
local tensor = require 'system.tensor'
local path = ...
return tensor.ByteTensor{
    file = {name = path .. 'bytes.bin', byteOffset = 1, numElements = 64}
}
)";

TEST_F(LuaTensorTest, kBadNumElementsOffset) {
  ASSERT_THAT(lua::PushScript(L, kBadNumElementsOffset,

                              "kBadNumElementsOffset"),
              IsOkAndHolds(1));
  lua::Push(L, CreateBytesRawFile());
  ASSERT_THAT(lua::Call(L, 1),
              StatusIs(AllOf(HasSubstr("numElements"), HasSubstr("63"))));
}

constexpr absl::string_view kBadNumElementsNegative = R"(
local tensor = require 'system.tensor'
local path = ...
return tensor.ByteTensor{file = {name = path .. 'bytes.bin', numElements = -1}}
)";

TEST_F(LuaTensorTest, kBadNumElementsNegative) {
  ASSERT_THAT(lua::PushScript(L, kBadNumElementsNegative,

                              "kBadNumElementsNegative"),
              IsOkAndHolds(1));
  lua::Push(L, CreateBytesRawFile());
  ASSERT_THAT(lua::Call(L, 1), StatusIs(HasSubstr("numElements")));
}

constexpr absl::string_view kBadByteOffset = R"(
local tensor = require 'system.tensor'
local path = ...
return tensor.ByteTensor{file = {name = path .. 'bytes.bin', byteOffset = 65}}
)";

TEST_F(LuaTensorTest, kBadByteOffset) {
  ASSERT_THAT(lua::PushScript(L, kBadByteOffset, "kBadByteOffset"),
              IsOkAndHolds(1));
  lua::Push(L, CreateBytesRawFile());
  ASSERT_THAT(lua::Call(L, 1), StatusIs(HasSubstr("byteOffset")));
}

constexpr absl::string_view kBadByteOffsetNegative = R"(
local tensor = require 'system.tensor'
local path = ...
return tensor.ByteTensor{file = {name = path .. 'bytes.bin', byteOffset = -1}}
)";

TEST_F(LuaTensorTest, kBadByteOffsetNegative) {
  ASSERT_THAT(lua::PushScript(L, kBadByteOffsetNegative,

                              "kBadByteOffsetNegative"),
              IsOkAndHolds(1));
  lua::Push(L, CreateBytesRawFile());
  ASSERT_THAT(lua::Call(L, 1), StatusIs(HasSubstr("byteOffset")));
}

constexpr absl::string_view kNumElements = R"(
local tensor = require 'system.tensor'
assert(tensor.ByteTensor(4, 5):size() == 20)
)";

TEST_F(LuaTensorTest, kNumElements) {
  ASSERT_THAT(lua::PushScript(L, kNumElements,

                              "kNumElements"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kClamp = R"(
local tensor = require 'system.tensor'
local doubles = tensor.DoubleTensor(3, 2)
doubles(1):fill(-500)
doubles(2):fill(25)
doubles(3):fill(500)
local clampBoth = doubles:clone():clamp(0, 255)
assert(clampBoth == tensor.DoubleTensor{{0, 0}, {25, 25}, {255, 255}},
       tostring(clampBoth))
local clampLower = doubles:clone():clamp(0, nil)
assert(clampLower == tensor.DoubleTensor{{0, 0}, {25, 25}, {500, 500}},
       tostring(clampLower))
local clampUpper = doubles:clone():clamp(nil, 255)
assert(clampUpper == tensor.DoubleTensor{{-500, -500}, {25, 25}, {255, 255}},
       tostring(clampUpper))
)";

TEST_F(LuaTensorTest, kClamp) {
  ASSERT_THAT(lua::PushScript(L, kClamp, "kClamp"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kClampErrorMaxMin = R"(
local tensor = require 'system.tensor'
tensor.DoubleTensor(3, 2):clamp(255, 0)
)";

TEST_F(LuaTensorTest, kClampErrorMaxMin) {
  ASSERT_THAT(lua::PushScript(L, kClampErrorMaxMin, "kClampMaxMin"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), StatusIs(HasSubstr("must not exceed")));
}

constexpr absl::string_view kClampTypeMismatch = R"(
local tensor = require 'system.tensor'
tensor.ByteTensor(3, 2):clamp(-10, 100)
)";

TEST_F(LuaTensorTest, kClampTypeMismatch) {
  ASSERT_THAT(lua::PushScript(L, kClampTypeMismatch, "kClampTypeMismatch"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), StatusIs(HasSubstr("TypeMismatch")));
}

}  // namespace
}  // namespace deepmind::lab2d
