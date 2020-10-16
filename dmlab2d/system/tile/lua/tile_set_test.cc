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

#include "dmlab2d/system/tile/lua/tile_set.h"

#include <vector>

#include "absl/strings/string_view.h"
#include "dmlab2d/lua/bind.h"
#include "dmlab2d/lua/n_results_or_test_util.h"
#include "dmlab2d/lua/push_script.h"
#include "dmlab2d/lua/vm_test_util.h"
#include "dmlab2d/system/math/lua/math2d.h"
#include "dmlab2d/system/math/math2d.h"
#include "dmlab2d/system/tensor/lua/tensor.h"
#include "dmlab2d/system/tile/pixel.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {

using ::deepmind::lab2d::lua::testing::IsOkAndHolds;
using ::testing::ElementsAre;
using ::testing::Eq;

constexpr Pixel MakePixel(unsigned char r, unsigned char g, unsigned char b) {
  return Pixel{PixelByte(r), PixelByte(g), PixelByte(b)};
}

int LuaTileModule(lua_State* L) {
  lua::TableRef table = lua::TableRef::Create(L);
  table.Insert("set", &lua::Bind<LuaTileSet::Create>);
  lua::Push(L, table);
  return 1;
}

class LuaTileSetTest : public lua::testing::TestWithVm {
 protected:
  LuaTileSetTest() {
    tensor::LuaTensorRegister(L);
    vm()->AddCModuleToSearchers("system.tensor", tensor::LuaTensorConstructors);
    LuaTileSet::Register(L);
    vm()->AddCModuleToSearchers("system.tile", &LuaTileModule);
  }
};

constexpr absl::string_view kCreate = R"(
local tile = require 'system.tile'
local set = tile.set{names = {'a', 'b', 'c', 'd'},
                     shape = {width = 3, height = 7}}
return set:names(), set:shape()
)";

TEST_F(LuaTileSetTest, Create) {
  ASSERT_THAT(lua::PushScript(L, kCreate, "kCreate"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(2));
  std::vector<absl::string_view> sprite_names;
  ASSERT_TRUE(IsFound(lua::Read(L, 1, &sprite_names)));
  EXPECT_THAT(sprite_names, ElementsAre("a", "b", "c", "d"));

  math::Size2d sprite_size;
  ASSERT_TRUE(IsFound(Read(L, 2, &sprite_size)));
  EXPECT_THAT(sprite_size.height, Eq(7));
  EXPECT_THAT(sprite_size.width, Eq(3));
}

constexpr absl::string_view kSetSprite = R"(
local tensor = require 'system.tensor'
local tile = require 'system.tile'
local set = tile.set{names = {'red', 'green', 'blue'},
                     shape = {width = 1, height = 1}}

local image = tensor.ByteTensor(1, 1, 4)
set:setSprite{name = 'red', image = image:fill{255, 0, 0, 255}}
set:setSprite{name = 'green', image = image:fill{0, 255, 0, 127}}
set:setSprite{name = 'blue', image = image:fill{0, 0, 255, 63}}
return set
)";

TEST_F(LuaTileSetTest, SetSprite) {
  ASSERT_THAT(lua::PushScript(L, kSetSprite, "kSetSprite"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  LuaTileSet* lua_tile_set = nullptr;
  ASSERT_TRUE(IsFound(Read(L, 1, &lua_tile_set))) << lua::ToString(L, 1);
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteRgbData(0),
              ElementsAre(MakePixel(255, 0, 0)));
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteAlphaData(0),
              ElementsAre(PixelByte(255)));
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteRgbData(1),
              ElementsAre(MakePixel(0, 255, 0)));
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteAlphaData(1),
              ElementsAre(PixelByte(127)));
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteRgbData(2),
              ElementsAre(MakePixel(0, 0, 255)));
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteAlphaData(2),
              ElementsAre(PixelByte(63)));
}

constexpr absl::string_view kSetMultBroadcast = R"(
local tensor = require 'system.tensor'
local tile = require 'system.tile'
local set = tile.set{
    names = {
        'startSentinal',
        'prefix.N',
        'prefix.E',
        'prefix.S',
        'prefix.W',
        'endSentinal',
    },
    shape = {width = 1, height = 1}
}

set:setSprite{
    name = 'prefix',
    image = tensor.ByteTensor{{{20, 30, 40}}}
}

return set
)";

TEST_F(LuaTileSetTest, SetMultBroadcast) {
  ASSERT_THAT(lua::PushScript(L, kSetMultBroadcast, "kSetMultBroadcast"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  LuaTileSet* lua_tile_set = nullptr;
  ASSERT_TRUE(IsFound(Read(L, 1, &lua_tile_set))) << lua::ToString(L, 1);
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteRgbData(0),
              ElementsAre(MakePixel(0, 0, 0)));
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteRgbData(5),
              ElementsAre(MakePixel(0, 0, 0)));
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteRgbData(1),
              ElementsAre(MakePixel(20, 30, 40)));
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteRgbData(2),
              ElementsAre(MakePixel(20, 30, 40)));
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteRgbData(3),
              ElementsAre(MakePixel(20, 30, 40)));
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteRgbData(4),
              ElementsAre(MakePixel(20, 30, 40)));
}

constexpr absl::string_view kSetMultSim = R"(
local tensor = require 'system.tensor'
local tile = require 'system.tile'
local set = tile.set{
    names = {
        'startSentinal',
        'prefix.0',
        'prefix.1',
        'prefix.2',
        'endSentinal',
    },
    shape = {width = 1, height = 1}
}
set:setSprite{
    name = 'prefix',
    image = tensor.ByteTensor{
        {{{1, 2, 3}}},
        {{{4, 5, 6}}},
        {{{7, 8, 9}}},
    }
}
return set
)";

TEST_F(LuaTileSetTest, SetMultSim) {
  ASSERT_THAT(lua::PushScript(L, kSetMultSim, "kSetMultSim"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  LuaTileSet* lua_tile_set = nullptr;
  ASSERT_TRUE(IsFound(Read(L, 1, &lua_tile_set))) << lua::ToString(L, 1);
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteRgbData(0),
              ElementsAre(MakePixel(0, 0, 0)));
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteRgbData(4),
              ElementsAre(MakePixel(0, 0, 0)));
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteRgbData(1),
              ElementsAre(MakePixel(1, 2, 3)));
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteRgbData(2),
              ElementsAre(MakePixel(4, 5, 6)));
  EXPECT_THAT(lua_tile_set->tile_set().GetSpriteRgbData(3),
              ElementsAre(MakePixel(7, 8, 9)));
}

}  // namespace
}  // namespace deepmind::lab2d
