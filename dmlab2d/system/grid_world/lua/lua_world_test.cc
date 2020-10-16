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
#include "dmlab2d/system/grid_world/lua/lua_world.h"

#include "dmlab2d/lua/call.h"
#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/n_results_or_test_util.h"
#include "dmlab2d/lua/push_script.h"
#include "dmlab2d/lua/vm.h"
#include "dmlab2d/lua/vm_test_util.h"
#include "dmlab2d/system/grid_world/lua/lua_grid.h"
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

class LuaWorldTest : public lua::testing::TestWithVm {
 protected:
  LuaWorldTest() {
    vm()->AddCModuleToSearchers("system.grid_world", LuaWorld::Module);
  }
};

constexpr char kCreateEmptyWorld[] = R"(
local grid_world = require 'system.grid_world'
return grid_world.World{renderOrder = {}, states = {}}
)";

TEST_F(LuaWorldTest, CreateEmptyWorld) {
  ASSERT_THAT(lua::PushScript(L, kCreateEmptyWorld, "kCreateEmptyWorld"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  LuaWorld* lua_world = nullptr;
  ASSERT_TRUE(IsFound(Read(L, 1, &lua_world)));
  const auto& world = lua_world->world();
  EXPECT_THAT(world.states().Names(), IsEmpty());
  EXPECT_THAT(world.NumRenderLayers(), Eq(0));
  EXPECT_THAT(world.sprites().Names(), IsEmpty());
  EXPECT_THAT(world.layers().Names(), IsEmpty());
}

constexpr char kCreateOneStateWorld[] = R"(
local grid_world = require 'system.grid_world'
return grid_world.World{
    renderOrder = {},
    states = { state0 = {} }
}
)";

TEST_F(LuaWorldTest, CreateOneStateWorld) {
  ASSERT_THAT(lua::PushScript(L, kCreateOneStateWorld, "kCreateOneStateWorld"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  LuaWorld* lua_world = nullptr;
  ASSERT_TRUE(IsFound(Read(L, 1, &lua_world)));
  const auto& world = lua_world->world();
  EXPECT_THAT(world.NumRenderLayers(), Eq(0));
  EXPECT_THAT(world.states().Names(), ElementsAre("state0"));
  EXPECT_THAT(world.sprites().Names(), IsEmpty());
  EXPECT_THAT(world.layers().Names(), IsEmpty());
}

constexpr char kCreateRenderLayerWorld[] = R"(
local grid_world = require 'system.grid_world'
return grid_world.World{
    renderOrder = {'layer2', 'layer0'},
    outOfBoundsSprite = 'sprite3',
    outOfViewSprite = 'sprite4',
    customSprites = {'sprite5', 'sprite6'},
    states = {
        state0 = {
            layer = 'layer0',
            sprite = 'sprite0'
        },
        state1 = {
            layer = 'layer1',
            sprite = 'sprite1'
        },
        state2 = {
            layer = 'layer2',
            sprite = 'sprite2'
        }
    }
}
)";

TEST_F(LuaWorldTest, CreateRenderLayerWorld) {
  ASSERT_THAT(
      lua::PushScript(L, kCreateRenderLayerWorld, "kCreateRenderLayerWorld"),
      IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  LuaWorld* lua_world = nullptr;
  ASSERT_TRUE(IsFound(Read(L, 1, &lua_world)));
  const auto& world = lua_world->world();
  EXPECT_THAT(world.states().Names(),
              ElementsAre("state0", "state1", "state2"));

  EXPECT_THAT(world.sprites().Names(),
              ElementsAre("sprite0", "sprite1", "sprite2", "sprite3", "sprite4",
                          "sprite5", "sprite6"));
  EXPECT_THAT(world.NumRenderLayers(), Eq(2));
  EXPECT_THAT(world.layers().Names(),
              ElementsAre("layer2", "layer0", "layer1"));
  EXPECT_THAT(world.out_of_bounds_sprite(),
              Eq(world.sprites().ToHandle("sprite3")));
  EXPECT_THAT(world.out_of_view_sprite(),
              Eq(world.sprites().ToHandle("sprite4")));
}

constexpr char kCreateGroupedWorld[] = R"(
local grid_world = require 'system.grid_world'
return grid_world.World{
    renderOrder = {'layer2', 'layer0'},
    outOfBoundsSprite = 'sprite3',
    outOfViewSprite = 'sprite4',
    states = {
        state0 = {
            layer = 'layer0',
            sprite = 'sprite0',
            groups = {'all', 'state0', 'state0or1'},
        },
        state1 = {
            layer = 'layer1',
            sprite = 'sprite1',
            groups = {'all', 'state1', 'state0or1'},
        },
        state2 = {
            layer = 'layer2',
            sprite = 'sprite2',
            groups = {'all', 'state2'},
        }
    }
}
)";

TEST_F(LuaWorldTest, CreateGroupedWorld) {
  ASSERT_THAT(lua::PushScript(L, kCreateGroupedWorld, "kCreateGroupedWorld"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  LuaWorld* lua_world = nullptr;
  ASSERT_TRUE(IsFound(Read(L, 1, &lua_world)));
  const auto& world = lua_world->world();
  EXPECT_THAT(world.groups().Names(),
              ElementsAre("all", "state0", "state0or1", "state1", "state2"));
  const auto& state_data0 = world.state_data(world.states().ToHandle("state0"));
  EXPECT_THAT(state_data0.groups,
              ElementsAre(world.groups().ToHandle("all"),
                          world.groups().ToHandle("state0"),
                          world.groups().ToHandle("state0or1")));
  const auto& state_data1 = world.state_data(world.states().ToHandle("state1"));
  EXPECT_THAT(state_data1.groups,
              ElementsAre(world.groups().ToHandle("all"),
                          world.groups().ToHandle("state0or1"),
                          world.groups().ToHandle("state1")));
  const auto& state_data2 = world.state_data(world.states().ToHandle("state2"));
  EXPECT_THAT(state_data2.groups,
              ElementsAre(world.groups().ToHandle("all"),
                          world.groups().ToHandle("state2")));
}

constexpr char kCreateContacts[] = R"(
local grid_world = require 'system.grid_world'
return grid_world.World{
    renderOrder = {'layer2', 'layer0'},
    outOfBoundsSprite = 'sprite3',
    outOfViewSprite = 'sprite4',
    states = {
        state0 = {
            layer = 'layer0',
            sprite = 'sprite0',
            groups = {'all', 'state0', 'state0or1'},
            contact = 'contact0',
        },
        state1 = {
            layer = 'layer1',
            sprite = 'sprite1',
            groups = {'all', 'state1', 'state0or1'},
            contact = 'contact0',
        },
        state2 = {
            layer = 'layer2',
            sprite = 'sprite2',
            groups = {'all', 'state2'},
            contact = 'contact1',
        }
    }
}
)";

TEST_F(LuaWorldTest, CreateContacts) {
  ASSERT_THAT(lua::PushScript(L, kCreateContacts, "kCreateContacts"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  LuaWorld* lua_world = nullptr;
  ASSERT_TRUE(IsFound(Read(L, 1, &lua_world)));
  const auto& world = lua_world->world();
  EXPECT_THAT(world.contacts().Names(), ElementsAre("contact0", "contact1"));
  const auto& state_data = world.state_data(world.states().ToHandle("state0"));
  EXPECT_THAT(state_data.contact_handle,
              Eq(world.contacts().ToHandle("contact0")));
  const auto& state_data1 = world.state_data(world.states().ToHandle("state1"));
  EXPECT_THAT(state_data1.contact_handle,
              Eq(world.contacts().ToHandle("contact0")));
  const auto& state_data2 = world.state_data(world.states().ToHandle("state2"));
  EXPECT_THAT(state_data2.contact_handle,
              Eq(world.contacts().ToHandle("contact1")));
}

constexpr char kCreateHits[] = R"(
local grid_world = require 'system.grid_world'
return grid_world.World{
    renderOrder = {},
    states = {},
    hits = {
        hit0 = {
          layer = 'hitLayer0',
          sprite = 'hitSprite0',
        },
        hit1 = {
          layer = 'hitLayer1',
          sprite = 'hitSprite1',
        },
        hit2NotVis = {},
    },
}
)";

TEST_F(LuaWorldTest, CreateHits) {
  ASSERT_THAT(lua::PushScript(L, kCreateHits, "kCreateHitTypes"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  LuaWorld* lua_world = nullptr;
  ASSERT_TRUE(IsFound(Read(L, 1, &lua_world)));
  const auto& world = lua_world->world();
  EXPECT_THAT(world.hits().Names(), ElementsAre("hit0", "hit1", "hit2NotVis"));
  EXPECT_THAT(world.sprites().Names(), ElementsAre("hitSprite0", "hitSprite1"));
  EXPECT_THAT(world.layers().Names(), ElementsAre("hitLayer0", "hitLayer1"));
  const Hit hit0 = world.hits().ToHandle("hit0");
  const Hit hit1 = world.hits().ToHandle("hit1");
  const Hit hit2_not_vis = world.hits().ToHandle("hit2NotVis");
  EXPECT_THAT(world.hit_data(hit0).layer,
              Eq(world.layers().ToHandle("hitLayer0")));
  EXPECT_THAT(world.hit_data(hit1).layer,
              Eq(world.layers().ToHandle("hitLayer1")));
  EXPECT_THAT(world.hit_data(hit2_not_vis).layer, Layer());
  EXPECT_THAT(world.hit_data(hit0).sprite_handle,
              Eq(world.sprites().ToHandle("hitSprite0")));
  EXPECT_THAT(world.hit_data(hit1).sprite_handle,
              Eq(world.sprites().ToHandle("hitSprite1")));
  EXPECT_THAT(world.hit_data(hit2_not_vis).sprite_handle, Sprite());
}

constexpr char kCreateGrid[] = R"(
local grid_world = require 'system.grid_world'
local world = grid_world.World{
    renderOrder = {'layer0', 'layer1'},
    states = {
        state0 = {
            layer = 'layer0',
            sprite = 'sprite0'
        },
        state1 = {
            layer = 'layer1',
            sprite = 'sprite1'
        },
        state2 = {
            layer = 'layer2',
            sprite = 'sprite2'
        }
    }
}

local grid = world:createGrid{
    layout = '012',
    typeMap = {
      ['0'] = 'state0',
      ['1'] = 'state1',
      ['2'] = 'state2',
    }
}
world = nil
return grid
)";

TEST_F(LuaWorldTest, CreateGrid) {
  ASSERT_THAT(lua::PushScript(L, kCreateGrid, "kCreateGrid"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  LuaGrid* lua_grid = nullptr;
  ASSERT_TRUE(IsFound(Read(L, 1, &lua_grid)));
  ASSERT_THAT(lua_grid->GetGrid().GetWorld().layers().Names(),
              ElementsAre("layer0", "layer1", "layer2"));
}

constexpr char kUpdateOrderWorks[] = R"(
local grid_world = require 'system.grid_world'
return grid_world.World{
    updateOrder = {'ccc', 'aaa', 'bbb'},
    renderOrder = {},
    states = {},
}
)";

TEST_F(LuaWorldTest, UpdateOrderWorks) {
  ASSERT_THAT(lua::PushScript(L, kUpdateOrderWorks, "kUpdateOrderWorks"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  LuaWorld* lua_world = nullptr;
  ASSERT_TRUE(IsFound(Read(L, 1, &lua_world)));
  ASSERT_THAT(lua_world->world().updates().Names(),
              ElementsAre("ccc", "aaa", "bbb"));
}

constexpr char kMissingTable[] = R"(
local grid_world = require 'system.grid_world'
return grid_world.World()
)";

TEST_F(LuaWorldTest, MissingTable) {
  ASSERT_THAT(lua::PushScript(L, kMissingTable, "kMissingTable"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), StatusIs(HasSubstr("table")));
}

constexpr char kMissingRenderOrder[] = R"(
local grid_world = require 'system.grid_world'
return grid_world.World{states = {}}
)";

TEST_F(LuaWorldTest, MissingRenderOrder) {
  ASSERT_THAT(lua::PushScript(L, kMissingRenderOrder, "kMissingRenderOrder"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), StatusIs(HasSubstr("renderOrder")));
}

constexpr char kMissingStates[] = R"(
local grid_world = require 'system.grid_world'
return grid_world.World{renderOrder = {}}
)";

TEST_F(LuaWorldTest, MissingTypes) {
  ASSERT_THAT(lua::PushScript(L, kMissingStates, "kMissingStates"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), StatusIs(HasSubstr("states")));
}

constexpr char kInvalidType[] = R"(
local grid_world = require 'system.grid_world'
return grid_world.World{
    renderOrder = {},
    states = { t0 = 10 }
}
)";

TEST_F(LuaWorldTest, InvalidType) {
  ASSERT_THAT(lua::PushScript(L, kInvalidType, "kInvalidType"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0),
              StatusIs(AllOf(HasSubstr("states"), HasSubstr("t0"))));
}

constexpr char kInvalidLayer[] = R"(
local grid_world = require 'system.grid_world'
return grid_world.World{
    renderOrder = {},
    states = { t0 = {layer = {}} }
}
)";

TEST_F(LuaWorldTest, InvalidLayer) {
  ASSERT_THAT(lua::PushScript(L, kInvalidLayer, "kInvalidLayer"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0),
              StatusIs(AllOf(HasSubstr("layer"), HasSubstr("t0"))));
}

constexpr char kInvalidSprite[] = R"(
local grid_world = require 'system.grid_world'
return grid_world.World{
    renderOrder = {},
    states = { t0 = {sprite = {}} }
}
)";

TEST_F(LuaWorldTest, InvalidSprite) {
  ASSERT_THAT(lua::PushScript(L, kInvalidSprite, "kInvalidSprite"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0),
              StatusIs(AllOf(HasSubstr("sprite"), HasSubstr("t0"))));
}

}  // namespace
}  // namespace deepmind::lab2d
