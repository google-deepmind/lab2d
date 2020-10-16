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

#include "dmlab2d/system/grid_world/world.h"

#include "dmlab2d/system/grid_world/handles.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::IsEmpty;

TEST(WorldTest, StatesWorks) {
  World::Args args;
  args.states["state0"];
  args.states["state1"];
  const World world(args);
  EXPECT_THAT(world.states().Names(), ElementsAre("state0", "state1"));
}

TEST(WorldTest, NumRenderLayersWorks) {
  World::Args args;
  args.render_order = {"layer2", "layer1"};
  args.states["state0"].layer = "layer0";
  args.states["state1"].layer = "layer1";
  args.states["state2"].layer = "layer2";
  args.states["state3"].layer = "layer3";
  const World world(args);
  EXPECT_THAT(world.NumRenderLayers(), Eq(2));
}

TEST(WorldTest, LayersWorks) {
  World::Args args;
  args.render_order = {"layer2", "layer1", "custom_layer"};
  args.states["state0"].layer = "layer0";
  args.states["state1"].layer = "layer1";
  args.states["state2"].layer = "layer2";
  args.states["state3"].layer = "layer3";
  const World world(args);
  EXPECT_THAT(
      world.layers().Names(),
      ElementsAre("layer2", "layer1", "custom_layer", "layer0", "layer3"));
}

TEST(WorldTest, ContactsWorks) {
  World::Args args;
  args.states["state0"].contact = "contacts0";
  args.states["state1"].contact = "contacts0";
  args.states["state2"].contact = "contacts1";
  args.states["state3"].contact = "contacts1";
  const World world(args);
  EXPECT_THAT(world.contacts().Names(), ElementsAre("contacts0", "contacts1"));
  EXPECT_THAT(
      world.state_data(world.states().ToHandle("state0")).contact_handle,
      Eq(world.contacts().ToHandle("contacts0")));
  EXPECT_THAT(
      world.state_data(world.states().ToHandle("state1")).contact_handle,
      Eq(world.contacts().ToHandle("contacts0")));
  EXPECT_THAT(
      world.state_data(world.states().ToHandle("state2")).contact_handle,
      Eq(world.contacts().ToHandle("contacts1")));
  EXPECT_THAT(
      world.state_data(world.states().ToHandle("state3")).contact_handle,
      Eq(world.contacts().ToHandle("contacts1")));
}

TEST(WorldTest, UpdatesWorks) {
  World::Args args;
  args.update_order = {{"one"}, {"two"}, {"three"}};
  args.states["state0"].layer = "layer0";
  const World world(args);
  EXPECT_THAT(world.updates().Names(), ElementsAre("one", "two", "three"));
}

TEST(WorldTest, SpritesWorks) {
  World::Args args;
  args.states["state0"].sprite = "sprite0";
  args.custom_sprites = {"sprite1", "sprite2", "sprite3"};
  args.out_of_bounds_sprite = "sprite4";
  args.out_of_view_sprite = "sprite5";
  const World world(args);
  EXPECT_THAT(world.sprites().Names(),
              ElementsAre("sprite0", "sprite1", "sprite2", "sprite3", "sprite4",
                          "sprite5"));
  EXPECT_THAT(world.out_of_bounds_sprite(),
              Eq(world.sprites().ToHandle("sprite4")));
  EXPECT_THAT(world.out_of_view_sprite(),
              Eq(world.sprites().ToHandle("sprite5")));
}

TEST(WorldTest, SpriteNamesWorks) {
  World::Args args;
  args.states["state0"].sprite = "sprite0";
  args.states["state1"].sprite = "sprite1";
  args.states["state2"].sprite = "sprite1";
  const World world(args);

  EXPECT_THAT(world.state_data(State(0)).sprite_handle, Eq(Sprite(0)));
  EXPECT_THAT(world.state_data(State(1)).sprite_handle, Eq(Sprite(1)));
  EXPECT_THAT(world.state_data(State(2)).sprite_handle, Eq(Sprite(1)));
}

TEST(WorldTest, GroupsWorks) {
  World::Args args;
  args.states["state0"].group_names = {"group0", "group1"};
  args.states["state1"].group_names = {"group0"};
  args.states["state2"].group_names = {"group1"};
  args.states["state3"].group_names = {};
  const World world(args);
  EXPECT_THAT(world.groups().Names(), ElementsAre("group0", "group1"));
  EXPECT_THAT(world.state_data(world.states().ToHandle("state0")).groups,
              ElementsAre(world.groups().ToHandle("group0"),
                          world.groups().ToHandle("group1")));
  EXPECT_THAT(world.state_data(world.states().ToHandle("state1")).groups,
              ElementsAre(world.groups().ToHandle("group0")));
  EXPECT_THAT(world.state_data(world.states().ToHandle("state2")).groups,
              ElementsAre(world.groups().ToHandle("group1")));
  EXPECT_THAT(world.state_data(world.states().ToHandle("state3")).groups,
              IsEmpty());
}

TEST(WorldTest, HitsWorks) {
  World::Args args;
  args.hits["hit0"] = World::HitArg{"hitLayer0", "hitSprite0"};
  args.hits["hit1"] = World::HitArg{"hitLayer1", "hitSprite1"};
  args.hits["hit2NotVis"] = World::HitArg{};
  const World world(args);
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

}  // namespace
}  // namespace deepmind::lab2d
