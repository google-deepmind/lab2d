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

#include "absl/strings/string_view.h"
#include "benchmark/benchmark.h"
#include "dmlab2d/lua/call.h"
#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/n_results_or.h"
#include "dmlab2d/lua/n_results_or_test_util.h"
#include "dmlab2d/lua/push_script.h"
#include "dmlab2d/lua/vm.h"
#include "dmlab2d/system/tensor/lua/tensor.h"
#include "dmlab2d/util/default_read_only_file_system.h"
#include "dmlab2d/util/file_reader_types.h"

namespace deepmind::lab2d {
namespace {

void PerformOperation(benchmark::State& state, absl::string_view ctor,
                      absl::string_view operation) {
  using lua::testing::IsOkAndHolds;
  auto lua_vm = lua::CreateVm();
  auto* L = lua_vm.get();
  LuaRandom::Register(L);
  tensor::LuaTensorRegister(L);
  void* default_fs = const_cast<DeepMindReadOnlyFileSystem*>(
      util::DefaultReadOnlyFileSystem());
  lua_vm.AddCModuleToSearchers("system.tensor", tensor::LuaTensorConstructors,
                               {default_fs});

  if (auto result = lua::PushScript(L, ctor, "Construction"); !result.ok()) {
    LOG(FATAL) << result.error();
  }
  if (auto result = lua::Call(L, 0); !result.ok()) {
    LOG(FATAL) << result.error();
  }
  if (auto result = lua::PushScript(L, operation, "Operation"); !result.ok()) {
    LOG(FATAL) << result.error();
  }

  CHECK_EQ(3, lua_gettop(L));

  // Lua stack: bt1, bt2, function
  for (auto _ : state) {
    lua_pushvalue(L, -1);
    // Lua stack: bt1, bt2, function, function,
    lua_pushvalue(L, -4);
    // Lua stack: bt1, bt2, function, function, bt1
    lua_pushvalue(L, -4);
    // Lua stack: bt1, bt2, function, function, bt1, bt2
    ASSERT_THAT(lua::Call(L, 2), IsOkAndHolds(0));
    // Lua stack: bt1, bt2, function
  }
  lua_pop(L, 3);
}

// Constructors:

constexpr absl::string_view kTwoContigTensors = R"(
local tensor = require 'system.tensor'
return tensor.ByteTensor(1000, 1000, 2), tensor.ByteTensor(1000, 1000, 2)
)";

constexpr absl::string_view kContigNonContigTensors = R"(
local tensor = require 'system.tensor'
local contiguous = tensor.ByteTensor(1000, 1000, 2)
local nonContiguous = tensor.ByteTensor(1000, 1000, 3):narrow(3, 1, 2)
return nonContiguous, contiguous
)";

constexpr absl::string_view kTwoNonContigTensors = R"(
local tensor = require 'system.tensor'
local contiguous = tensor.ByteTensor(1000, 1000, 3):narrow(3, 1, 2)
local nonContiguous = tensor.ByteTensor(1000, 1000, 3):narrow(3, 1, 2)
return nonContiguous, contiguous
)";

// Operations:

constexpr absl::string_view kCopyTensors = R"(
local bt1, bt2 = ...
bt1:copy(bt2)
)";

constexpr absl::string_view kCopyTensorsBySlice = R"(
local bt1, bt2 = ...
bt1:select(3, 1):copy(bt2:select(3, 1))
bt1:select(3, 2):copy(bt2:select(3, 2))
)";

constexpr absl::string_view kDoubleTensors = R"(
local bt1, bt2 = ...
bt1:mul(2)
bt2:mul(2)
)";

constexpr absl::string_view kDoubleBySlice = R"(
local bt1, bt2 = ...
bt1:select(3,1):mul(2)
bt1:select(3,2):mul(2)
bt2:select(3,1):mul(2)
bt2:select(3,2):mul(2)
)";

constexpr absl::string_view kFill = R"(
local bt1, bt2 = ...
bt1:fill(0)
bt2:fill(0)
)";

constexpr absl::string_view kFillByTable = R"(
local bt1, bt2 = ...
bt1:fill{0, 0}
bt2:fill{0, 0}
)";

constexpr absl::string_view kFillBySlice = R"(
local bt1, bt2 = ...
local fill = {0, 0}
for i = 1, 2 do
  bt1:select(3, i):fill(fill[i])
end
for i = 1, 2 do
  bt2:select(3, i):fill(fill[i])
end
)";

void BM_ContiguousTensorCopy(benchmark::State& state) {
  PerformOperation(state, kTwoContigTensors, kCopyTensors);
}

BENCHMARK(BM_ContiguousTensorCopy);

void BM_ContiguousToNonContiguousCopy(benchmark::State& state) {
  PerformOperation(state, kContigNonContigTensors, kCopyTensors);
}

BENCHMARK(BM_ContiguousToNonContiguousCopy);

void BM_TwoNonContiguousCopy(benchmark::State& state) {
  PerformOperation(state, kTwoNonContigTensors, kCopyTensors);
}

BENCHMARK(BM_TwoNonContiguousCopy);

void BM_TwoNonContiguousCopyBySlice(benchmark::State& state) {
  PerformOperation(state, kTwoNonContigTensors, kCopyTensorsBySlice);
}

BENCHMARK(BM_TwoNonContiguousCopyBySlice);

void BM_ContiguousTensorDouble(benchmark::State& state) {
  PerformOperation(state, kTwoContigTensors, kDoubleTensors);
}

BENCHMARK(BM_ContiguousTensorDouble);

void BM_NonContiguousTensorDouble(benchmark::State& state) {
  PerformOperation(state, kTwoNonContigTensors, kDoubleTensors);
}

BENCHMARK(BM_NonContiguousTensorDouble);

void BM_NonContiguousTensorDoubleBySlice(benchmark::State& state) {
  PerformOperation(state, kTwoNonContigTensors, kDoubleBySlice);
}

BENCHMARK(BM_NonContiguousTensorDoubleBySlice);

void BM_ContiguousFill(benchmark::State& state) {
  PerformOperation(state, kTwoContigTensors, kFill);
}

BENCHMARK(BM_ContiguousFill);

void BM_NonContiguousFill(benchmark::State& state) {
  PerformOperation(state, kTwoNonContigTensors, kFill);
}

BENCHMARK(BM_NonContiguousFill);

void BM_ContiguousFillByTable(benchmark::State& state) {
  PerformOperation(state, kTwoContigTensors, kFillByTable);
}

BENCHMARK(BM_ContiguousFillByTable);

void BM_ContiguousFillBySelect(benchmark::State& state) {
  PerformOperation(state, kTwoContigTensors, kFillBySlice);
}

BENCHMARK(BM_ContiguousFillBySelect);

void BM_NonContiguousFillByTable(benchmark::State& state) {
  PerformOperation(state, kTwoNonContigTensors, kFillByTable);
}

BENCHMARK(BM_NonContiguousFillByTable);

void BM_NonContiguousFillBySelect(benchmark::State& state) {
  PerformOperation(state, kTwoNonContigTensors, kFillBySlice);
}

BENCHMARK(BM_NonContiguousFillBySelect);

constexpr absl::string_view kTwoSmallContigTensors = R"(
local tensor = require 'system.tensor'
return tensor.ByteTensor(100, 100, 2), tensor.ByteTensor(100, 100, 2)
)";

constexpr absl::string_view kFillByValue = R"(
local unpack = _G.unpack or table.unpack  -- Handle different versions of Lua.
local bt1, bt2 = ...
local ci, cj, ck = unpack(bt1:shape())
for i = 1, ci do
  for j = 1, cj do
    for k = 1, ck do
      bt1(i, j, k):val(1)
      bt2(i, j, k):val(1)
    end
  end
end
)";

void BM_MemberCall(benchmark::State& state) {
  PerformOperation(state, kTwoSmallContigTensors, kFillByValue);
}

BENCHMARK(BM_MemberCall);

}  // namespace
}  // namespace deepmind::lab2d
