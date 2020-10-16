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

#include <cstdint>
#include <random>

#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "dmlab2d/lua/bind.h"
#include "dmlab2d/lua/call.h"
#include "dmlab2d/lua/n_results_or_test_util.h"
#include "dmlab2d/lua/push_script.h"
#include "dmlab2d/lua/read.h"
#include "dmlab2d/lua/vm.h"
#include "dmlab2d/lua/vm_test_util.h"
#include "dmlab2d/system/random/lua/random.h"
#include "dmlab2d/system/tensor/lua/tensor.h"
#include "dmlab2d/system/tensor/tensor_view.h"
#include "dmlab2d/system/tile/lua/tile.h"
#include "dmlab2d/util/default_read_only_file_system.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {

using ::deepmind::lab2d::lua::testing::IsOkAndHolds;
using ::testing::ElementsAre;

class LuaTileSceneTest : public lua::testing::TestWithVm {
 protected:
  LuaTileSceneTest() {
    LuaRandom::Register(L);
    void* default_fs = const_cast<DeepMindReadOnlyFileSystem*>(
        util::DefaultReadOnlyFileSystem());
    vm()->AddCModuleToSearchers(
        "system.sys_random", &lua::Bind<LuaRandom::Require>,
        {&prbg_, reinterpret_cast<void*>(static_cast<std::uintptr_t>(0))});
    tensor::LuaTensorRegister(L);
    vm()->AddCModuleToSearchers("system.tensor", tensor::LuaTensorConstructors,
                                {default_fs});

    vm()->AddCModuleToSearchers("system.tile", LuaTileModule);
  }
  std::mt19937_64 prbg_;
};

constexpr absl::string_view kRenderSprite = R"(
local tensor = require 'system.tensor'
local tile = require 'system.tile'
local set = tile.Set{names = {'red', 'green', 'blue'},
                     shape = {width = 3, height = 3}}

local image = tensor.ByteTensor(3, 3, 4)
set:setSprite{name = 'red', image = image:fill{255, 0, 0, 255}}
set:setSprite{name = 'green', image = image:fill{0, 255, 0, 255}}
set:setSprite{name = 'blue', image = image:fill{0, 0, 255, 255}}

local scene = tile.Scene{shape = {width = 3, height = 1}, set = set}
local grid = tensor.Int32Tensor{range = {0, 2}}:reshape{1, 3}
return scene:render(grid)
)";

TEST_F(LuaTileSceneTest, RenderSprite) {
  ASSERT_THAT(lua::PushScript(L, kRenderSprite, "kRenderSprite"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  tensor::LuaTensor<std::uint8_t>* render = nullptr;
  ASSERT_TRUE(IsFound(Read(L, 1, &render))) << lua::ToString(L, 1);
  tensor::TensorView<std::uint8_t> tensor = render->tensor_view();
  ASSERT_THAT(tensor.shape(), ElementsAre(3, 9, 3));
  for (int row_i = 0; row_i < 3; ++row_i) {
    auto row = absl::MakeConstSpan(tensor.storage() + row_i * 9 * 3, 9 * 3);
    auto red = row.subspan(0 * 9, 9);
    EXPECT_THAT(red, ElementsAre(255, 0, 0, 255, 0, 0, 255, 0, 0));
    auto green = row.subspan(1 * 9, 9);
    EXPECT_THAT(green, ElementsAre(0, 255, 0, 0, 255, 0, 0, 255, 0));
    auto blue = row.subspan(2 * 9, 9);
    EXPECT_THAT(blue, ElementsAre(0, 0, 255, 0, 0, 255, 0, 0, 255));
  }
}

constexpr absl::string_view kBlendSprite = R"(
local tensor = require 'system.tensor'
local tile = require 'system.tile'
local set = tile.Set{
    names = {'black', 'white', 'red', 'green', 'blue'},
    shape = {width = 3, height = 1}
}

set:setSprite{
    name = 'black',
    image = tensor.ByteTensor(1, 3, 4):fill{0, 0, 0, 255}
}

set:setSprite{
    name = 'white',
    image = tensor.ByteTensor(1, 3, 4):fill{255, 255, 255, 255}
}

set:setSprite{name = 'red', image = tensor.ByteTensor{
    {{255, 0, 0, 255}, {255, 0, 0, 255}, {255, 0, 0, 255}},
}}

set:setSprite{name = 'green', image = tensor.ByteTensor{
    {{0, 255, 0, 127}, {0, 255, 0, 127}, {0, 255, 0, 127}},
}}

set:setSprite{name = 'blue', image = tensor.ByteTensor{
    {{0, 0, 255, 0}, {0, 0, 255, 127}, {0, 0, 255, 255}},
}}

local scene = tile.Scene{shape = {width = 3, height = 1}, set = set}
local blackBack = tensor.Int32Tensor{{{0, 2}, {0, 3}, {0, 4}}}
assert(scene:render(blackBack) == tensor.ByteTensor{{
    {255,   0,   0}, {255,   0,   0}, {255,   0,   0},  -- red
    {  0, 127,   0}, {  0, 127,   0}, {  0, 127,   0},  -- green
    {  0,   0,   0}, {  0,   0, 127}, {  0,   0, 255},  -- blue
}})

local whiteBack = tensor.Int32Tensor{{{1, 2}, {1, 3}, {1, 4}}}
assert(scene:render(whiteBack) == tensor.ByteTensor{{
    {255,   0,   0}, {255,   0,   0}, {255,   0,   0},  -- red
    {128, 255, 128}, {128, 255, 128}, {128, 255, 128},  -- green
    {255, 255, 255}, {128, 128, 255}, {  0,   0, 255},  -- blue
}})
)";

TEST_F(LuaTileSceneTest, BlendSprite) {
  ASSERT_THAT(lua::PushScript(L, kBlendSprite, "kBlendSprite"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kRenderShape = R"(
local tensor = require 'system.tensor'
local tile = require 'system.tile'
local shape = {height = 2, width = 3}
local gridShape = {height = 5, width = 7}
local set = tile.Set{names = {}, shape = shape}
local scene = tile.Scene{shape = gridShape, set = set}
local shapeOut = set:shape()
assert(shapeOut.height == shape.height)
assert(shapeOut.width == shape.width)
local shape = scene:shape()
assert(#shape == 3)
assert(shape[1] == shapeOut.height * gridShape.height)
assert(shape[2] == shapeOut.width * gridShape.width)
assert(shape[3] == 3)
)";

TEST_F(LuaTileSceneTest, RenderShape) {
  ASSERT_THAT(lua::PushScript(L, kRenderShape, "kRenderShape"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

}  // namespace
}  // namespace deepmind::lab2d
