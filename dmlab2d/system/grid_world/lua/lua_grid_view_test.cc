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

#include "dmlab2d/system/grid_world/lua/lua_grid_view.h"

#include <vector>

#include "absl/strings/string_view.h"
#include "dmlab2d/lua/call.h"
#include "dmlab2d/lua/n_results_or_test_util.h"
#include "dmlab2d/lua/push_script.h"
#include "dmlab2d/lua/read.h"
#include "dmlab2d/lua/table_ref.h"
#include "dmlab2d/lua/vm.h"
#include "dmlab2d/lua/vm_test_util.h"
#include "dmlab2d/system/grid_world/lua/lua_world.h"
#include "dmlab2d/system/math/lua/math2d.h"
#include "dmlab2d/system/math/math2d.h"
#include "dmlab2d/system/tensor/lua/tensor.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {

using ::deepmind::lab2d::lua::testing::IsOkAndHolds;
using ::deepmind::lab2d::lua::testing::StatusIs;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::HasSubstr;

constexpr absl::string_view kTestWorld = R"(
local grid_world = require 'system.grid_world'
local world = grid_world.World{
    renderOrder = {'layer0', 'layer2'},
    outOfBoundsSprite = '3sprite',
    outOfViewSprite = '4sprite',
    types = {
        type0 = {
            layer = 'layer0',
            sprite = '0sprite'
        },
        type1 = {
            layer = 'layer1',
            sprite = '1sprite'
        },
        type2 = {
            layer = 'layer2',
            sprite = '2sprite'
        },
        typeNoLayer = {},
    }
}

local function makeGrid(layout)
  return world:createGrid{
      layout = layout,
      typeMap = {
          ['a'] = 'type0',
          ['b'] = 'type1',
          ['c'] = 'type2',
      }
  }
end

local function makeView(args)
  return world:createView(args)
end

return {
    makeGrid = makeGrid,
    makeView = makeView,
    spriteNames = world:spriteNames(),
}
)";

class LuaGridViewTest : public lua::testing::TestWithVm {
 protected:
  LuaGridViewTest() {
    tensor::LuaTensorRegister(L);
    vm()->AddCModuleToSearchers("system.tensor", tensor::LuaTensorConstructors);
    vm()->AddCModuleToSearchers("system.grid_world", LuaWorld::Module);
    vm()->AddLuaModuleToSearchers("test_world", kTestWorld.data(),
                                  kTestWorld.size());
  }
};

constexpr absl::string_view kRenderWholeWorld = R"(
local test_world = require 'test_world'
local tensor = require 'system.tensor'
local grid = test_world.makeGrid[[
a a a

b b b

c c c

a b c
]]
local gridView = test_world.makeView{
    left = 0,
    right = 4,
    forward = 0,
    backward = 6,
}

local observation = gridView:observation{
    grid = grid,
    transform = {pos = {0, 0}, orientation = 'N'},
}

local _ = {0, 0} -- Empty
local a = {1, 0} -- 0sprite on layer0
assert(test_world.spriteNames[1 + 1] == '0sprite.N')
local b = {0, 0} -- Invisible.
local c = {0, 9} -- 2sprite on layer2
assert(test_world.spriteNames[9 + 1] == '2sprite.N')
local expectObservation = tensor.Int32Tensor{
    {a, _, a, _, a},
    {_, _, _, _, _},
    {b, _, b, _, b},
    {_, _, _, _, _},
    {c, _, c, _, c},
    {_, _, _, _, _},
    {a, _, b, _, c}
}

assert(observation == expectObservation)
return gridView:observationSpec('whole'), gridView:gridSize()
)";

TEST_F(LuaGridViewTest, RenderWholeWorld) {
  ASSERT_THAT(lua::PushScript(L, kRenderWholeWorld, "kRenderWholeWorld"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  lua::TableRef spec_table;
  ASSERT_TRUE(IsFound(lua::Read(L, 1, &spec_table)));

  absl::string_view name;
  ASSERT_TRUE(IsFound(spec_table.LookUp("name", &name)));
  EXPECT_THAT(name, Eq("whole"));

  std::vector<int> shape;
  ASSERT_TRUE(IsFound(spec_table.LookUp("shape", &shape)));
  EXPECT_THAT(shape, ElementsAre(7, 5, 2));

  absl::string_view type;
  ASSERT_TRUE(IsFound(spec_table.LookUp("type", &type)));
  EXPECT_THAT(type, Eq("tensor.Int32Tensor"));

  math::Size2d grid_size2d;
  ASSERT_TRUE(IsFound(Read(L, 2, &grid_size2d)));
  EXPECT_THAT(grid_size2d, Eq(math::Size2d{5, 7}));
}

constexpr absl::string_view kRenderSpriteMap = R"(
local test_world = require 'test_world'
local tensor = require 'system.tensor'
local grid = test_world.makeGrid[[
a a a

b b b

c c c

a b c
]]
local gridView = test_world.makeView{
    left = 0,
    right = 4,
    forward = 0,
    backward = 6,
    spriteMap = {['2sprite'] = '1sprite'}
}

local observation = gridView:observation{
    grid = grid,
    transform = {pos = {0, 0}, orientation = 'N'},
}

local _ = {0, 0} -- Empty
local a = {1, 0} -- 0sprite on layer0
assert(test_world.spriteNames[1 + 1] == '0sprite.N')
local b = {0, 0} -- Invisble.
local c = {0, 5} -- 1sprite on layer2
assert(test_world.spriteNames[5 + 1] == '1sprite.N')
local expectObservation = tensor.Int32Tensor{
    {a, _, a, _, a},
    {_, _, _, _, _},
    {b, _, b, _, b},
    {_, _, _, _, _},
    {c, _, c, _, c},
    {_, _, _, _, _},
    {a, _, b, _, c}
}

assert(observation == expectObservation)
)";

TEST_F(LuaGridViewTest, RenderSpriteMap) {
  ASSERT_THAT(lua::PushScript(L, kRenderSpriteMap, "kRenderSpriteMap"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kInvalidFromSpriteMap = R"(
local test_world = require 'test_world'
test_world.makeView{
    left = 0,
    right = 4,
    forward = 0,
    backward = 6,
    spriteMap = {['FromMissing'] = '1sprite'}
}
)";

constexpr absl::string_view kInvalidToSpriteMap = R"(
local test_world = require 'test_world'
test_world.makeView{
    left = 0,
    right = 4,
    forward = 0,
    backward = 6,
    spriteMap = {['2sprite'] = 'ToMissing'}
}
)";

TEST_F(LuaGridViewTest, InvalidSpriteMap) {
  ASSERT_THAT(
      lua::PushScript(L, kInvalidFromSpriteMap, "kInvalidFromSpriteMap"),
      IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0),
              StatusIs(HasSubstr("Invalid source sprite in "
                                 "`spriteMap`: 'FromMissing'")));
  ASSERT_THAT(lua::PushScript(L, kInvalidToSpriteMap, "kInvalidToSpriteMap"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), StatusIs(HasSubstr("Invalid target sprite in "
                                                  "`spriteMap`: 'ToMissing'")));
}

constexpr absl::string_view kRenderFromViewer = R"(
local test_world = require 'test_world'
local tensor = require 'system.tensor'
local grid = test_world.makeGrid[[
a   a

b b b

c c c

a b c
]]

local piece = grid:createPiece('type0', {pos = {2, 0}, orientation = 'S'})
local gridView = test_world.makeView{
    left = 2,
    right = 2,
    forward = 6,
    backward = 0,
}

local observation = gridView:observation{
    grid = grid,
    piece = piece
}

local _ = {0, 0} -- Empty
local a = {3, 0} -- 0sprite on layer0 South
local p = {1, 0} -- 0sprite on layer0 North
assert(test_world.spriteNames[3 + 1] == '0sprite.S')
local b = {0, 0} -- Invisible.
local c = {0, 11} -- 2sprite on layer2
assert(test_world.spriteNames[11 + 1] == '2sprite.S')
local expectObservation = tensor.Int32Tensor{
    {c, _, b, _, a},
    {_, _, _, _, _},
    {c, _, c, _, c},
    {_, _, _, _, _},
    {b, _, b, _, b},
    {_, _, _, _, _},
    {a, _, p, _, a},
}
assert(observation == expectObservation)
)";

TEST_F(LuaGridViewTest, RenderFromViewer) {
  ASSERT_THAT(lua::PushScript(L, kRenderFromViewer, "kRenderFromViewer"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kRenderFromOffscreenViewer = R"(
local test_world = require 'test_world'
local tensor = require 'system.tensor'
local grid = test_world.makeGrid[[
a   a

b b b

c c c

a b c
]]

local piece = grid:createPiece('typeNoLayer', {pos = {0, 0}, orientation = 'N'})
local gridView = test_world.makeView{
    left = 2,
    right = 2,
    forward = 6,
    backward = 0,
}

local observation = gridView:observation{
    grid = grid,
    piece = piece
}

assert(observation == tensor.Int32Tensor(7, 5, 2):fill(13))

local piece2 = grid:createPiece('type0', {pos = {-1, -1}, orientation = 'N'})

local observation = gridView:observation{
    grid = grid,
    piece = piece2
}

assert(observation == tensor.Int32Tensor(7, 5, 2):fill(13))
)";
TEST_F(LuaGridViewTest, RenderFromOffscreenViewer) {
  ASSERT_THAT(lua::PushScript(L, kRenderFromOffscreenViewer,
                              "kRenderFromOffscreenViewer"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

constexpr absl::string_view kRenderWholeLayout = R"(
local test_world = require 'test_world'
local tensor = require 'system.tensor'
local layout = [[
a a a

b b b

c c c

a b c
]]

local gridView = test_world.makeView{layout = layout}
local grid = test_world.makeGrid(layout)
local _ = {0, 0} -- Empty
local a = {1, 0} -- 0sprite on layer0
local b = {0, 0} -- Invisble.
local c = {0, 9} -- 2sprite on layer2
local expectObservation = tensor.Int32Tensor{
    {a, _, a, _, a},
    {_, _, _, _, _},
    {b, _, b, _, b},
    {_, _, _, _, _},
    {c, _, c, _, c},
    {_, _, _, _, _},
    {a, _, b, _, c},
}

local observation = gridView:observation{grid = grid}

assert(observation == expectObservation,
       tostring(observation) .. ' ~= ' .. tostring(expectObservation))
)";
TEST_F(LuaGridViewTest, RenderWholeLayout) {
  ASSERT_THAT(lua::PushScript(L, kRenderWholeLayout, "kRenderWholeLayout"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
}

}  // namespace
}  // namespace deepmind::lab2d
