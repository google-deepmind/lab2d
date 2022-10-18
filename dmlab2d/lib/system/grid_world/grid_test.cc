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

#include "dmlab2d/lib/system/grid_world/grid.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <limits>
#include <memory>
#include <numeric>
#include <random>
#include <type_traits>
#include <vector>

#include "absl/strings/ascii.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "dmlab2d/lib/system/grid_world/collections/fixed_handle_map.h"
#include "dmlab2d/lib/system/grid_world/grid_shape.h"
#include "dmlab2d/lib/system/grid_world/grid_view.h"
#include "dmlab2d/lib/system/grid_world/grid_window.h"
#include "dmlab2d/lib/system/grid_world/handles.h"
#include "dmlab2d/lib/system/grid_world/sprite_instance.h"
#include "dmlab2d/lib/system/grid_world/text_tools.h"
#include "dmlab2d/lib/system/grid_world/world.h"
#include "dmlab2d/lib/system/math/math2d.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {

using ::testing::_;
using ::testing::AnyOf;
using ::testing::Each;
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::IsTrue;
using ::testing::Ne;
using ::testing::Return;
using ::testing::SizeIs;
using ::testing::UnorderedElementsAre;

void PlaceGrid(const CharMap& char_to_state, absl::string_view layout,
               math::Orientation2d orientation, Grid* grid) {
  layout = RemoveLeadingAndTrailingNewLines(layout);
  std::vector<absl::string_view> lines = absl::StrSplit(layout, '\n');
  int row = 0;
  for (auto line : lines) {
    int col = 0;
    for (auto character : line) {
      auto state = char_to_state[character];
      if (!state.IsEmpty()) {
        grid->CreateInstance(state, math::Transform2d{{col, row}, orientation});
      }
      ++col;
    }
    ++row;
  }
}

World::Args CreateWorldArgs() {
  World::Args args = {};
  args.render_order = {"fruit", "pieces"};
  args.states["Player"] = World::StateArg{"pieces", "Player"};
  args.states["Wall"] = World::StateArg{"pieces", "*Wall"};
  args.states["Apple"] = World::StateArg{"fruit", "Apple"};
  args.states["Spawn"] = World::StateArg{"invisible", "Spawn"};
  args.states["OffGrid"] = World::StateArg{};
  args.out_of_bounds_sprite = "OutOfBounds";
  args.out_of_view_sprite = "OutOfView";
  return args;
}

class MockStateCallback : public Grid::StateCallback {
 public:
  MOCK_METHOD(void, OnAdd, (Piece piece), (override));

  MOCK_METHOD(void, OnRemove, (Piece piece), (override));

  MOCK_METHOD(void, OnUpdate,
              (Update update, Piece piece, int num_frames_in_state),
              (override));

  MOCK_METHOD(void, OnBlocked, (Piece mover, Piece blocker), (override));

  MOCK_METHOD(void, OnEnter, (Contact contact, Piece piece, Piece instigator),
              (override));

  MOCK_METHOD(void, OnLeave, (Contact contact, Piece piece, Piece instigator),
              (override));

  MOCK_METHOD(Grid::HitResponse, OnHit,
              (Hit hit, Piece piece, Piece instigator), (override));
};

GridView CreateGridView(const World& world, int left, int right, int forward,
                        int backward) {
  FixedHandleMap<Sprite, Sprite> sprite_map(world.sprites().NumElements());
  for (std::size_t i = 0; i < sprite_map.size(); ++i) {
    Sprite handle(i);
    sprite_map[handle] = handle;
  }
  // +-----+
  // |     |
  // |     |
  // |     |
  // |     |
  // |     |
  // |  p  |
  // +-----+
  const GridWindow kWindow(/*centered=*/false,
                           /*left=*/left, /*right=*/right,
                           /*forward=*/forward, /*backward=*/backward);
  return GridView(/*window=*/kWindow,
                  /*num_render_layers=*/world.NumRenderLayers(),
                  /*sprite_map=*/std::move(sprite_map),
                  /*out_of_bounds_sprite=*/world.out_of_bounds_sprite(),
                  /*out_of_view_sprite=*/world.out_of_view_sprite());
}

TEST(GridTest, GetShapeWorks) {
  const World world(CreateWorldArgs());
  const math::Size2d grid_size_2d{3, 5};
  Grid grid(world, grid_size_2d, GridShape::Topology::kBounded);
  const auto& shape = grid.GetShape();
  EXPECT_THAT(shape.GridSize2d(), Eq(grid_size_2d));
  EXPECT_THAT(shape.layer_count(), Eq(world.layers().NumElements()));
}

TEST(GridTest, GetShapeTorusWorks) {
  const World world(CreateWorldArgs());
  Grid grid(world, math::Size2d{3, 5}, GridShape::Topology::kTorus);
  const auto& shape = grid.GetShape();
  EXPECT_THAT(shape.topology(), Eq(GridShape::Topology::kTorus));
}

constexpr const absl::string_view kGrid = R"(
*********
* P A P *
*********
)";

TEST(GridTest, ToStringWorks) {
  const World world(CreateWorldArgs());
  Grid grid(world, GetSize2dOfText(kGrid), GridShape::Topology::kBounded);
  CharMap char_to_state = {};
  char_to_state['P'] = world.states().ToHandle("Player");
  char_to_state['*'] = world.states().ToHandle("Wall");
  char_to_state['A'] = world.states().ToHandle("Apple");
  PlaceGrid(char_to_state, kGrid, math::Orientation2d::kNorth, &grid);
  std::string text_render = grid.ToString();
  EXPECT_THAT(absl::StripAsciiWhitespace(text_render),
              Eq(absl::StripAsciiWhitespace(kGrid)));
}

TEST(GridTest, ReleaseInstanceWorks) {
  const World world(CreateWorldArgs());
  Grid grid(world, math::Size2d{1, 1}, GridShape::Topology::kBounded);
  math::Transform2d trans = {{0, 0}, math::Orientation2d::kNorth};

  auto mock_state_callback = std::make_unique<MockStateCallback>();
  EXPECT_CALL(*mock_state_callback.get(), OnAdd(Piece(0)));
  EXPECT_CALL(*mock_state_callback.get(), OnRemove(Piece(0)));

  grid.SetCallback(world.states().ToHandle("Player"),
                   std::move(mock_state_callback));

  Piece player = grid.CreateInstance(world.states().ToHandle("Player"), trans);
  grid.ReleaseInstance(player);
  EXPECT_THAT(grid.AllPieceHandles(trans.position),
              ElementsAre(Eq(Piece()), Eq(Piece()), Eq(Piece())));
}

constexpr const absl::string_view kFruitGrid = R"(
***********
* AAAA LL *
***********
)";

TEST(GridTest, GroupsWorks) {
  World::Args args = CreateWorldArgs();
  std::mt19937_64 random;
  args.states["Apple"].group_names = {"fruits", "apples"};
  auto& lemon = args.states["Lemon"];
  lemon.group_names = {"fruits", "lemons"};
  lemon.layer = "fruit";
  lemon.sprite = "Lemon";
  const World world(args);
  CharMap char_to_state = {};
  char_to_state['*'] = world.states().ToHandle("Wall");
  char_to_state['A'] = world.states().ToHandle("Apple");
  char_to_state['L'] = world.states().ToHandle("Lemon");
  Grid grid(world, GetSize2dOfText(kFruitGrid), GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, kFruitGrid, math::Orientation2d::kNorth, &grid);
  EXPECT_THAT(grid.PieceCountByGroup(world.groups().ToHandle("fruits")), Eq(6));
  EXPECT_THAT(grid.PieceCountByGroup(world.groups().ToHandle("apples")), Eq(4));
  EXPECT_THAT(grid.PieceCountByGroup(world.groups().ToHandle("lemons")), Eq(2));

  auto to_states = [&grid](absl::Span<const Piece> pieces) {
    std::vector<State> states;
    states.reserve(pieces.size());
    std::transform(pieces.begin(), pieces.end(), std::back_inserter(states),
                   [&grid](Piece piece) { return grid.GetState(piece); });
    return states;
  };

  auto fruit_states =
      grid.PiecesByGroupShuffled(world.groups().ToHandle("fruits"), &random);
  EXPECT_THAT(to_states(fruit_states),
              testing::UnorderedElementsAre(
                  char_to_state['A'], char_to_state['A'], char_to_state['A'],
                  char_to_state['A'], char_to_state['L'], char_to_state['L']));

  auto apple_states =
      grid.PiecesByGroupShuffled(world.groups().ToHandle("apples"), &random);
  EXPECT_THAT(
      to_states(apple_states),
      testing::UnorderedElementsAre(char_to_state['A'], char_to_state['A'],
                                    char_to_state['A'], char_to_state['A']));
  auto lemon_states =
      grid.PiecesByGroupShuffled(world.groups().ToHandle("lemons"), &random);
  EXPECT_THAT(
      to_states(lemon_states),
      testing::UnorderedElementsAre(char_to_state['L'], char_to_state['L']));

  auto some_fruit = grid.PiecesByGroupShuffledWithProbabilty(
      world.groups().ToHandle("fruits"), 0.5, &random);
  EXPECT_THAT(to_states(some_fruit),
              Each(AnyOf(char_to_state['L'], char_to_state['A'])));

  auto some_fruit2 = grid.PiecesByGroupShuffledWithMaxCount(
      world.groups().ToHandle("fruits"), 4, &random);
  EXPECT_THAT(to_states(some_fruit2),
              Each(AnyOf(char_to_state['L'], char_to_state['A'])));
  EXPECT_THAT(some_fruit2, SizeIs(4));
}

TEST(GridTest, GroupsRemoveRandomWorks) {
  World::Args args = CreateWorldArgs();
  std::mt19937_64 random;
  args.states["Apple"].group_names = {"fruits", "apples"};
  auto& lemon = args.states["Lemon"];
  lemon.group_names = {"fruits", "lemons"};
  lemon.layer = "fruit";
  lemon.sprite = "Lemon";
  const World world(args);
  CharMap char_to_state = {};
  char_to_state['*'] = world.states().ToHandle("Wall");
  char_to_state['A'] = world.states().ToHandle("Apple");
  char_to_state['L'] = world.states().ToHandle("Lemon");
  Grid grid(world, GetSize2dOfText(kFruitGrid), GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, kFruitGrid, math::Orientation2d::kNorth, &grid);
  EXPECT_THAT(grid.PieceCountByGroup(world.groups().ToHandle("fruits")), Eq(6));
  EXPECT_THAT(grid.PieceCountByGroup(world.groups().ToHandle("apples")), Eq(4));
  EXPECT_THAT(grid.PieceCountByGroup(world.groups().ToHandle("lemons")), Eq(2));
  Piece random_apple =
      grid.RandomPieceByGroup(world.groups().ToHandle("apples"), &random);
  grid.ReleaseInstance(random_apple);
  EXPECT_THAT(grid.PieceCountByGroup(world.groups().ToHandle("fruits")), Eq(5));
  EXPECT_THAT(grid.PieceCountByGroup(world.groups().ToHandle("apples")), Eq(3));
  EXPECT_THAT(grid.PieceCountByGroup(world.groups().ToHandle("lemons")), Eq(2));
  Piece random_lemon =
      grid.RandomPieceByGroup(world.groups().ToHandle("lemons"), &random);
  EXPECT_FALSE(random_lemon.IsEmpty());
  grid.ReleaseInstance(random_lemon);
  random_lemon =
      grid.RandomPieceByGroup(world.groups().ToHandle("lemons"), &random);
  EXPECT_FALSE(random_lemon.IsEmpty());
  grid.ReleaseInstance(random_lemon);
  random_lemon =
      grid.RandomPieceByGroup(world.groups().ToHandle("lemons"), &random);
  EXPECT_TRUE(random_lemon.IsEmpty());
}

TEST(GridTest, AllPieceHandlesWorks) {
  const World world(CreateWorldArgs());
  Grid grid(world, math::Size2d{1, 1}, GridShape::Topology::kBounded);
  math::Transform2d trans = {{0, 0}, math::Orientation2d::kNorth};
  Piece player = grid.CreateInstance(world.states().ToHandle("Player"), trans);
  Piece apple_handle =
      grid.CreateInstance(world.states().ToHandle("Apple"), trans);
  Piece spawn_handle =
      grid.CreateInstance(world.states().ToHandle("Spawn"), trans);
  EXPECT_THAT(grid.AllPieceHandles(trans.position),
              ElementsAre(Eq(apple_handle), Eq(player), Eq(spawn_handle)));
}

TEST(GridTest, AllSpriteInstances) {
  const World world(CreateWorldArgs());
  Grid grid(world, math::Size2d{1, 1}, GridShape::Topology::kBounded);
  math::Transform2d trans = {{0, 0}, math::Orientation2d::kEast};
  grid.CreateInstance(world.states().ToHandle("Player"), trans);
  grid.CreateInstance(world.states().ToHandle("Apple"), trans);
  grid.CreateInstance(world.states().ToHandle("Spawn"), trans);
  absl::Span<const SpriteInstance> sprites =
      grid.AllSpriteInstances(trans.position);
  ASSERT_THAT(sprites.size(), Eq(2));
  ASSERT_THAT(sprites[0].handle, Eq(world.sprites().ToHandle("Apple")));
  ASSERT_THAT(sprites[0].orientation, Eq(trans.orientation));
  ASSERT_THAT(sprites[1].handle, Eq(world.sprites().ToHandle("Player")));
  ASSERT_THAT(sprites[1].orientation, Eq(trans.orientation));
}

TEST(GridTest, SpawnSameLayerPreventsSpawn) {
  const World world(CreateWorldArgs());
  Grid grid(world, math::Size2d{1, 1}, GridShape::Topology::kBounded);
  math::Transform2d trans = {{0, 0}, math::Orientation2d::kNorth};
  Piece wall_handle =
      grid.CreateInstance(world.states().ToHandle("Wall"), trans);
  Piece player = grid.CreateInstance(world.states().ToHandle("Player"), trans);
  EXPECT_THAT(player, Eq(Piece()));
  EXPECT_THAT(grid.AllPieceHandles(trans.position),
              ElementsAre(Eq(Piece()), Eq(wall_handle), Eq(Piece())));
}

TEST(GridTest, SpawnSameLayerPreventsSpawnTorus) {
  const World world(CreateWorldArgs());
  Grid grid(world, math::Size2d{2, 2}, GridShape::Topology::kTorus);
  math::Transform2d trans = {{0, 0}, math::Orientation2d::kNorth};
  Piece wall_handle =
      grid.CreateInstance(world.states().ToHandle("Wall"), trans);
  trans.position = math::Position2d{4, 4};  // Same as {0, 0} on torus.
  Piece player = grid.CreateInstance(world.states().ToHandle("Player"), trans);
  EXPECT_THAT(player, Eq(Piece()));
  EXPECT_THAT(grid.AllPieceHandles(trans.position),
              ElementsAre(Eq(Piece()), Eq(wall_handle), Eq(Piece())));
}

TEST(GridTest, GetPieceTransformWorks) {
  const World world(CreateWorldArgs());
  Grid grid(world, math::Size2d{5, 5}, GridShape::Topology::kBounded);
  math::Transform2d trans = {{3, 3}, math::Orientation2d::kWest};
  Piece player = grid.CreateInstance(world.states().ToHandle("Player"), trans);
  EXPECT_THAT(grid.AllPieceHandles(trans.position),
              ElementsAre(Eq(Piece()), Eq(player), Eq(Piece())));
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(trans));
  // Empty handles produce out of bounds.
  EXPECT_FALSE(
      grid.GetShape().InBounds((grid.GetPieceTransform(Piece()).position)));
}

TEST(GridTest, GetPieceTransformTorusWorks) {
  const World world(CreateWorldArgs());
  Grid grid(world, math::Size2d{5, 5}, GridShape::Topology::kTorus);
  math::Transform2d trans_looped = {{8, 8}, math::Orientation2d::kWest};
  math::Transform2d trans_canonical = {{3, 3}, math::Orientation2d::kWest};
  Piece player =
      grid.CreateInstance(world.states().ToHandle("Player"), trans_looped);
  EXPECT_THAT(grid.AllPieceHandles(trans_looped.position),
              ElementsAre(Eq(Piece()), Eq(player), Eq(Piece())));

  EXPECT_THAT(grid.AllPieceHandles(trans_canonical.position),
              ElementsAre(Eq(Piece()), Eq(player), Eq(Piece())));

  EXPECT_THAT(grid.GetPieceTransform(player), Eq(trans_canonical));
}

TEST(GridTest, UpdateWorks) {
  std::mt19937_64 random;
  World::Args args = CreateWorldArgs();
  args.update_order = {{"one"}, {"two"}};
  args.states["Player"].group_names = {"players"};
  const World world(args);
  Grid grid(world, math::Size2d{1, 4}, GridShape::Topology::kBounded);
  math::Transform2d trans = {{0, 0}, math::Orientation2d::kWest};

  auto mock_state_callback = std::make_unique<MockStateCallback>();
  EXPECT_CALL(*mock_state_callback.get(),
              OnUpdate(world.updates().ToHandle("one"), _, _))
      .Times(1 + 2 + 3);

  EXPECT_CALL(*mock_state_callback.get(), OnAdd(_)).Times(4);

  grid.SetCallback(world.states().ToHandle("Player"),
                   std::move(mock_state_callback));

  grid.SetUpdateInfo(world.updates().ToHandle("one"),
                     world.groups().ToHandle("players"), /*probability=*/1.0,
                     /*start_frame=*/0);

  Piece player0 = grid.CreateInstance(world.states().ToHandle("Player"), trans);
  grid.DoUpdate(&random);
  trans.position.x += 1;
  Piece player1 = grid.CreateInstance(world.states().ToHandle("Player"), trans);
  grid.DoUpdate(&random);
  trans.position.x += 1;
  Piece player2 = grid.CreateInstance(world.states().ToHandle("Player"), trans);
  grid.DoUpdate(&random);
  trans.position.x += 1;
  Piece player3 = grid.CreateInstance(world.states().ToHandle("Player"), trans);
  EXPECT_THAT(grid.GetPieceFrames(player0), Eq(3));
  EXPECT_THAT(grid.GetPieceFrames(player1), Eq(2));
  EXPECT_THAT(grid.GetPieceFrames(player2), Eq(1));
  EXPECT_THAT(grid.GetPieceFrames(player3), Eq(0));
}

TEST(GridTest, SetStateSameLayerWorks) {
  std::mt19937_64 random;
  const World world(CreateWorldArgs());
  Grid grid(world, math::Size2d{1, 4}, GridShape::Topology::kBounded);

  auto mock_player_state_callback = std::make_unique<MockStateCallback>();
  EXPECT_CALL(*mock_player_state_callback.get(), OnAdd(_));
  EXPECT_CALL(*mock_player_state_callback.get(), OnRemove(_));
  grid.SetCallback(world.states().ToHandle("Player"),
                   std::move(mock_player_state_callback));

  auto mock_wall_state_callback = std::make_unique<MockStateCallback>();
  EXPECT_CALL(*mock_wall_state_callback.get(), OnAdd(_));
  EXPECT_CALL(*mock_wall_state_callback.get(), OnRemove(_)).Times(0);
  grid.SetCallback(world.states().ToHandle("Wall"),
                   std::move(mock_wall_state_callback));

  State player_state = world.states().ToHandle("Player");
  State wall_state = world.states().ToHandle("Wall");
  math::Transform2d trans = {{0, 0}, math::Orientation2d::kWest};
  Piece player0 = grid.CreateInstance(player_state, trans);
  grid.DoUpdate(&random);
  grid.DoUpdate(&random);
  ASSERT_THAT(grid.GetState(player0), Eq(player_state));
  EXPECT_THAT(grid.GetPieceFrames(player0), Eq(2));

  absl::Span<const SpriteInstance> sprites_before =
      grid.AllSpriteInstances(trans.position);
  const SpriteInstance& sprite_before =
      sprites_before[world.layers().ToHandle("pieces").Value()];
  EXPECT_THAT(sprite_before.handle, Eq(world.sprites().ToHandle("Player")));

  grid.SetState(player0, wall_state);
  grid.DoUpdate(&random);
  EXPECT_THAT(grid.GetPieceFrames(player0), Eq(0));
  ASSERT_THAT(grid.GetState(player0), Eq(wall_state));

  absl::Span<const SpriteInstance> sprites_after =
      grid.AllSpriteInstances(trans.position);
  const SpriteInstance& sprite_after =
      sprites_after[world.layers().ToHandle("pieces").Value()];
  EXPECT_THAT(sprite_after.handle, Eq(world.sprites().ToHandle("*Wall")));
}

TEST(GridTest, SetStateDifferentLayerWorks) {
  std::mt19937_64 random;
  const World world(CreateWorldArgs());

  Grid grid(world, math::Size2d{1, 4}, GridShape::Topology::kBounded);

  State player_state = world.states().ToHandle("Player");
  State apple_state = world.states().ToHandle("Apple");

  auto mock_player_state_callback = std::make_unique<MockStateCallback>();
  EXPECT_CALL(*mock_player_state_callback.get(), OnAdd(_));
  EXPECT_CALL(*mock_player_state_callback.get(), OnRemove(_));
  grid.SetCallback(player_state, std::move(mock_player_state_callback));

  auto mock_apple_state_callback = std::make_unique<MockStateCallback>();
  EXPECT_CALL(*mock_apple_state_callback.get(), OnAdd(_));
  EXPECT_CALL(*mock_apple_state_callback.get(), OnRemove(_)).Times(0);
  grid.SetCallback(apple_state, std::move(mock_apple_state_callback));

  math::Transform2d trans = {{0, 0}, math::Orientation2d::kWest};
  Piece player0 = grid.CreateInstance(player_state, trans);
  ASSERT_THAT(grid.GetState(player0), Eq(player_state));

  absl::Span<const SpriteInstance> sprites_before =
      grid.AllSpriteInstances(trans.position);
  const SpriteInstance& sprite_before =
      sprites_before[world.layers().ToHandle("pieces").Value()];
  EXPECT_THAT(sprite_before.handle, Eq(world.sprites().ToHandle("Player")));

  grid.SetState(player0, apple_state);
  grid.DoUpdate(&random);
  ASSERT_THAT(grid.GetState(player0), Eq(apple_state));

  absl::Span<const SpriteInstance> sprites_after =
      grid.AllSpriteInstances(trans.position);
  const SpriteInstance& sprite_after =
      sprites_after[world.layers().ToHandle("fruit").Value()];
  EXPECT_THAT(sprite_after.handle, Eq(world.sprites().ToHandle("Apple")));
}

TEST(GridTest, SetStateDifferentLayerWithDelayWorks) {
  std::mt19937_64 random;
  const World world(CreateWorldArgs());
  Grid grid(world, math::Size2d{1, 4}, GridShape::Topology::kBounded);
  State player_state = world.states().ToHandle("Player");
  State apple_state = world.states().ToHandle("Apple");
  math::Transform2d trans = {{0, 0}, math::Orientation2d::kWest};
  Piece player0 = grid.CreateInstance(player_state, trans);
  Piece apple_handle0 = grid.CreateInstance(apple_state, trans);
  ASSERT_THAT(grid.GetState(player0), Eq(player_state));

  absl::Span<const SpriteInstance> sprites_before =
      grid.AllSpriteInstances(trans.position);
  const SpriteInstance& sprite_before =
      sprites_before[world.layers().ToHandle("pieces").Value()];
  EXPECT_THAT(sprite_before.handle, Eq(world.sprites().ToHandle("Player")));

  grid.SetState(player0, apple_state);
  grid.DoUpdate(&random);

  // Can't change as apple_handle0 is in the way.
  ASSERT_THAT(grid.GetState(player0), Eq(player_state));

  grid.ReleaseInstance(apple_handle0);
  grid.DoUpdate(&random);
  ASSERT_THAT(grid.GetState(player0), Eq(apple_state));

  absl::Span<const SpriteInstance> sprites_after =
      grid.AllSpriteInstances(trans.position);
  const SpriteInstance& sprite_after =
      sprites_after[world.layers().ToHandle("fruit").Value()];
  EXPECT_THAT(sprite_after.handle, Eq(world.sprites().ToHandle("Apple")));
}

TEST(GridTest, SetStateOffGridWorks) {
  std::mt19937_64 random;
  const World world(CreateWorldArgs());
  Grid grid(world, math::Size2d{1, 4}, GridShape::Topology::kBounded);
  State player_state = world.states().ToHandle("Player");
  State off_grid_state = world.states().ToHandle("OffGrid");
  math::Transform2d trans = {{0, 0}, math::Orientation2d::kWest};
  Piece player0 = grid.CreateInstance(player_state, trans);

  absl::Span<const SpriteInstance> sprites_before =
      grid.AllSpriteInstances(trans.position);
  const SpriteInstance& sprite_before =
      sprites_before[world.layers().ToHandle("pieces").Value()];
  EXPECT_THAT(sprite_before.handle, Eq(world.sprites().ToHandle("Player")));

  grid.SetState(player0, off_grid_state);
  grid.DoUpdate(&random);
  ASSERT_THAT(grid.GetState(player0), Eq(off_grid_state));

  for (const auto& sprite : grid.AllSpriteInstances(trans.position)) {
    EXPECT_THAT(sprite.handle, Eq(Sprite()));
  }
  grid.SetState(player0, player_state);
  grid.DoUpdate(&random);
  ASSERT_THAT(grid.GetState(player0), Eq(player_state));
  absl::Span<const SpriteInstance> sprites_after =
      grid.AllSpriteInstances(trans.position);
  const SpriteInstance& sprite_after =
      sprites_after[world.layers().ToHandle("pieces").Value()];
  EXPECT_THAT(sprite_after.handle, Eq(world.sprites().ToHandle("Player")));
}

TEST(GridTest, SetStateOffGridWorksAfterSetStateSelf) {
  std::mt19937_64 random;
  const World world(CreateWorldArgs());
  Grid grid(world, math::Size2d{1, 4}, GridShape::Topology::kBounded);
  State player_state = world.states().ToHandle("Player");
  State off_grid_state = world.states().ToHandle("OffGrid");
  math::Transform2d trans = {{0, 0}, math::Orientation2d::kWest};
  Piece player0 = grid.CreateInstance(player_state, trans);
  grid.SetState(player0, player_state);
  grid.DoUpdate(&random);
  grid.SetState(player0, off_grid_state);
  grid.DoUpdate(&random);
  ASSERT_THAT(grid.GetState(player0), Eq(off_grid_state));
}

TEST(GridTest, EnterLeaveCallbacksCalledCreateOn) {
  std::mt19937_64 random;
  World::Args args = CreateWorldArgs();
  args.states["Player"].contact = "playerContact";

  const World world(args);

  State player_state = world.states().ToHandle("Player");
  State apple_state = world.states().ToHandle("Apple");

  Contact player_contact = world.contacts().ToHandle("playerContact");

  Grid grid(world, math::Size2d{4, 1}, GridShape::Topology::kBounded);

  auto mock_player_state_callback = std::make_unique<MockStateCallback>();
  EXPECT_CALL(*mock_player_state_callback.get(), OnAdd(_));
  EXPECT_CALL(*mock_player_state_callback.get(), OnRemove(_)).Times(0);
  grid.SetCallback(player_state, std::move(mock_player_state_callback));

  auto mock_apple_state_callback = std::make_unique<MockStateCallback>();
  EXPECT_CALL(*mock_apple_state_callback.get(), OnAdd(_));
  EXPECT_CALL(*mock_apple_state_callback.get(), OnEnter(player_contact, _, _));
  EXPECT_CALL(*mock_apple_state_callback.get(), OnLeave(player_contact, _, _));
  EXPECT_CALL(*mock_apple_state_callback.get(), OnRemove(_));
  grid.SetCallback(apple_state, std::move(mock_apple_state_callback));

  math::Transform2d player_trans = {{0, 0}, math::Orientation2d::kNorth};
  grid.CreateInstance(player_state, player_trans);

  math::Transform2d apple_trans = {{0, 0}, math::Orientation2d::kNorth};
  Piece apple_handle = grid.CreateInstance(apple_state, apple_trans);
  grid.ReleaseInstance(apple_handle);
}

TEST(GridTest, EnterLeaveCallbacksCalledStepOver) {
  std::mt19937_64 random;
  World::Args args = CreateWorldArgs();
  args.states["Player"].contact = "playerContact";
  args.states["Apple"].contact = "appleContact";

  const World world(args);

  State player_state = world.states().ToHandle("Player");
  State apple_state = world.states().ToHandle("Apple");

  Contact player_contact = world.contacts().ToHandle("playerContact");
  Contact apple_contact = world.contacts().ToHandle("appleContact");

  Grid grid(world, math::Size2d{4, 1}, GridShape::Topology::kBounded);

  auto mock_player_state_callback = std::make_unique<MockStateCallback>();
  EXPECT_CALL(*mock_player_state_callback.get(), OnAdd(_));
  EXPECT_CALL(*mock_player_state_callback.get(), OnEnter(apple_contact, _, _));
  EXPECT_CALL(*mock_player_state_callback.get(), OnLeave(apple_contact, _, _));
  EXPECT_CALL(*mock_player_state_callback.get(), OnRemove(_)).Times(0);
  grid.SetCallback(player_state, std::move(mock_player_state_callback));

  auto mock_apple_state_callback = std::make_unique<MockStateCallback>();
  EXPECT_CALL(*mock_apple_state_callback.get(), OnAdd(_));
  EXPECT_CALL(*mock_apple_state_callback.get(), OnEnter(player_contact, _, _));
  EXPECT_CALL(*mock_apple_state_callback.get(), OnLeave(player_contact, _, _));
  EXPECT_CALL(*mock_apple_state_callback.get(), OnRemove(_)).Times(0);
  grid.SetCallback(apple_state, std::move(mock_apple_state_callback));

  math::Transform2d player_trans = {{0, 0}, math::Orientation2d::kNorth};
  Piece player = grid.CreateInstance(player_state, player_trans);

  math::Transform2d apple_trans = {{1, 0}, math::Orientation2d::kNorth};
  grid.CreateInstance(apple_state, apple_trans);

  grid.PushPiece(player, math::Orientation2d::kEast, Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  grid.PushPiece(player, math::Orientation2d::kEast, Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
}

TEST(GridTest, AddRemoveCallbacksChangeState) {
  std::mt19937_64 random;

  const World world(CreateWorldArgs());

  State player_state = world.states().ToHandle("Player");
  State apple_state = world.states().ToHandle("Apple");

  Grid grid(world, math::Size2d{2, 1}, GridShape::Topology::kBounded);

  auto mock_player_state_callback = std::make_unique<MockStateCallback>();
  EXPECT_CALL(*mock_player_state_callback.get(), OnAdd(_));
  EXPECT_CALL(*mock_player_state_callback.get(), OnRemove(_));
  grid.SetCallback(player_state, std::move(mock_player_state_callback));

  auto mock_apple_state_callback = std::make_unique<MockStateCallback>();
  EXPECT_CALL(*mock_apple_state_callback.get(), OnAdd(_));
  EXPECT_CALL(*mock_apple_state_callback.get(), OnRemove(_)).Times(0);
  grid.SetCallback(apple_state, std::move(mock_apple_state_callback));

  math::Transform2d player_trans = {{0, 0}, math::Orientation2d::kNorth};
  Piece player = grid.CreateInstance(player_state, player_trans);
  grid.DoUpdate(&random);
  grid.SetState(player, apple_state);
  grid.DoUpdate(&random);
}

TEST(GridTest, CallBackOnTeleportToGroup) {
  std::mt19937_64 random;
  World::Args args = CreateWorldArgs();
  args.states["Player"].contact = "playerContact";
  args.states["Spawn"].group_names = {"spawns"};
  const World world(args);

  State player_state = world.states().ToHandle("Player");
  State offgrid_state = world.states().ToHandle("OffGrid");
  State apple_state = world.states().ToHandle("Apple");
  State spawn_state = world.states().ToHandle("Spawn");
  Group spawn_group = world.groups().ToHandle("spawns");
  Contact player_contact = world.contacts().ToHandle("playerContact");

  Grid grid(world, math::Size2d{2, 1}, GridShape::Topology::kBounded);

  auto mock_player_state_callback = std::make_unique<MockStateCallback>();
  EXPECT_CALL(*mock_player_state_callback.get(), OnAdd(_));
  EXPECT_CALL(*mock_player_state_callback.get(), OnRemove(_)).Times(0);
  grid.SetCallback(player_state, std::move(mock_player_state_callback));

  auto mock_apple_state_callback = std::make_unique<MockStateCallback>();
  EXPECT_CALL(*mock_apple_state_callback.get(), OnAdd(_));
  EXPECT_CALL(*mock_apple_state_callback.get(), OnEnter(player_contact, _, _));
  EXPECT_CALL(*mock_apple_state_callback.get(), OnLeave(player_contact, _, _));
  EXPECT_CALL(*mock_apple_state_callback.get(), OnRemove(_)).Times(0);
  grid.SetCallback(apple_state, std::move(mock_apple_state_callback));

  math::Transform2d off_screen = {{-1, -1}, math::Orientation2d::kNorth};
  Piece player = grid.CreateInstance(offgrid_state, off_screen);
  math::Transform2d trans = {{0, 0}, math::Orientation2d::kNorth};
  grid.CreateInstance(apple_state, trans);
  Piece spawn_handle = grid.CreateInstance(spawn_state, trans);

  grid.DoUpdate(&random);
  grid.TeleportToGroup(player, spawn_group, player_state,
                       Grid::TeleportOrientation::kMatchTarget);
  grid.DoUpdate(&random);
  grid.PushPiece(spawn_handle, math::Orientation2d::kEast,
                 Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  grid.TeleportToGroup(player, spawn_group, player_state,
                       Grid::TeleportOrientation::kMatchTarget);
  grid.DoUpdate(&random);
}

std::string RenderToString(const GridView& view, math::Transform2d trans,
                           Grid* grid) {
  std::string result;
  const auto& size2d = view.GetWindow().size2d();
  std::vector<int> sprite_ids(view.NumCells());
  result.reserve(size2d.height * (size2d.width + 1) + 1);
  grid->Render(trans, view, absl::MakeSpan(sprite_ids));
  result.push_back('\n');
  for (int i = 0; i < size2d.height; ++i) {
    for (int j = 0; j < size2d.width; ++j) {
      int cell = i * size2d.width + j;
      char c = ' ';
      for (int k = 0; k < view.NumRenderLayers(); ++k) {
        int sub_cell = cell * view.NumRenderLayers() + k;
        int sprite_id = sprite_ids[sub_cell];
        if (sprite_id > 0) {
          auto sprite_handle = Sprite((sprite_id - 1) / 4);
          c = grid->GetWorld().sprites().ToName(sprite_handle).front();
        }
      }
      result.push_back(c);
    }
    result.push_back('\n');
  }
  return result;
}

constexpr const absl::string_view kGridRenderNorth = R"(
*************
****     ****
****     ****
****A    ****
****A    ****
****A    ****
****AAP  ****
*************
*************
*************
*************
*************
*************
)";

constexpr const absl::string_view kGridRenderEast = R"(
*************
*************
*************
*************
******AAAA  *
******A     *
******P     *
******      *
******      *
*************
*************
*************
*************
)";

constexpr const absl::string_view kGridRenderSouth = R"(
*************
*************
*************
*************
*************
*************
****  PAA****
****    A****
****    A****
****    A****
****     ****
****     ****
*************
)";

constexpr const absl::string_view kGridRenderWest = R"(
*************
*************
*************
*************
*      ******
*      ******
*     P******
*     A******
*  AAAA******
*************
*************
*************
*************
)";

constexpr const absl::string_view kGridRenderExpected = R"(
*******
*     *
*     *
*A    *
*A    *
*A    *
*AAP  *
*******
)";

TEST(GridTest, RenderEachDirectionWorks) {
  const World world(CreateWorldArgs());
  GridView view = CreateGridView(world, /*left=*/3, /*right=*/3,
                                 /*forward=*/6, /*backward=*/1);
  CharMap char_to_state = {};
  char_to_state['P'] = world.states().ToHandle("Player");
  char_to_state['*'] = world.states().ToHandle("Wall");
  char_to_state['A'] = world.states().ToHandle("Apple");

  Grid grid_north(world, GetSize2dOfText(kGridRenderNorth),
                  GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, kGridRenderNorth, math::Orientation2d::kNorth,
            &grid_north);

  Grid grid_east(world, GetSize2dOfText(kGridRenderEast),
                 GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, kGridRenderEast, math::Orientation2d::kEast,
            &grid_east);

  Grid grid_south(world, GetSize2dOfText(kGridRenderSouth),
                  GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, kGridRenderSouth, math::Orientation2d::kSouth,
            &grid_south);

  Grid grid_west(world, GetSize2dOfText(kGridRenderWest),
                 GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, kGridRenderWest, math::Orientation2d::kWest,
            &grid_west);

  EXPECT_THAT(RenderToString(
                  view, math::Transform2d{{6, 6}, math::Orientation2d::kNorth},
                  &grid_north),
              Eq(kGridRenderExpected));

  EXPECT_THAT(RenderToString(
                  view, math::Transform2d{{6, 6}, math::Orientation2d::kEast},
                  &grid_east),
              Eq(kGridRenderExpected));
  EXPECT_THAT(RenderToString(
                  view, math::Transform2d{{6, 6}, math::Orientation2d::kWest},
                  &grid_west),
              Eq(kGridRenderExpected));

  EXPECT_THAT(RenderToString(
                  view, math::Transform2d{{6, 6}, math::Orientation2d::kSouth},
                  &grid_south),
              Eq(kGridRenderExpected));
}

constexpr const absl::string_view kGridRenderTorus = R"(
**A**
    *
   AA
   A*
  PA*
)";

constexpr const absl::string_view kGridRenderTorusNorthResult = R"(
***A****A****A**
*    *    *    *
A   AA   AA   AA
*   A*   A*   A*
*  PA*  PA*  PA*
***A****A****A**
*    *    *    *
A   AA   AA   AA
*   A*   A*   A*
*  PA*  PA*  PA*
***A****A****A**
)";

constexpr const absl::string_view kGridRenderTorusEastResult =
    "\n"
    "    *    *    * \n"
    "    *    *    * \n"
    "*A****A****A****\n"
    " AAA* AAA* AAA* \n"
    "   PA   PA   PA \n"
    "    *    *    * \n"
    "    *    *    * \n"
    "*A****A****A****\n"
    " AAA* AAA* AAA* \n"
    "   PA   PA   PA \n"
    "    *    *    * \n";

constexpr const absl::string_view kGridRenderTorusSouthResult =
    "\n"
    " *A   *A   *A   \n"
    " AA   AA   AA   \n"
    " *    *    *    \n"
    "***A****A****A**\n"
    " *AP  *AP  *AP  \n"
    " *A   *A   *A   \n"
    " AA   AA   AA   \n"
    " *    *    *    \n"
    "***A****A****A**\n"
    " *AP  *AP  *AP  \n"
    " *A   *A   *A   \n";

TEST(GridTest, RenderTorusWorks) {
  const World world(CreateWorldArgs());
  GridView view = CreateGridView(world, /*left=*/8, /*right=*/7,
                                 /*forward=*/9, /*backward=*/1);
  CharMap char_to_state = {};
  char_to_state['P'] = world.states().ToHandle("Player");
  char_to_state['*'] = world.states().ToHandle("Wall");
  char_to_state['A'] = world.states().ToHandle("Apple");
  Grid grid(world, GetSize2dOfText(kGridRenderTorus),
            GridShape::Topology::kTorus);
  PlaceGrid(char_to_state, kGridRenderTorus, math::Orientation2d::kNorth,
            &grid);

  EXPECT_THAT(
      RenderToString(
          view, math::Transform2d{{2, 4}, math::Orientation2d::kNorth}, &grid),
      Eq(kGridRenderTorusNorthResult));

  EXPECT_THAT(
      RenderToString(
          view, math::Transform2d{{2, 4}, math::Orientation2d::kEast}, &grid),
      Eq(kGridRenderTorusEastResult));

  EXPECT_THAT(
      RenderToString(
          view, math::Transform2d{{2, 4}, math::Orientation2d::kSouth}, &grid),
      Eq(kGridRenderTorusSouthResult));
}

int CenterOffset(const GridView& grid_view, int offset_x, int offset_y,
                 Layer layer) {
  int y = grid_view.GetWindow().forward() + offset_y;
  int x = grid_view.GetWindow().right() + offset_x;
  return (y * grid_view.GetWindow().width() + x) * grid_view.NumRenderLayers() +
         layer.Value();
}

constexpr const absl::string_view kGridRenderOutOfBounds = R"(
*****
*A  *
*AP *
*****
)";

TEST(GridTest, RenderOutOfBounds) {
  const World world(CreateWorldArgs());
  GridView view = CreateGridView(world, /*left=*/3, /*right=*/3,
                                 /*forward=*/6, /*backward=*/1);
  CharMap char_to_state = {};
  char_to_state['P'] = world.states().ToHandle("Player");
  char_to_state['*'] = world.states().ToHandle("Wall");
  char_to_state['A'] = world.states().ToHandle("Apple");

  Grid grid(world, GetSize2dOfText(kGridRenderOutOfBounds),
            GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, kGridRenderOutOfBounds, math::Orientation2d::kNorth,
            &grid);

  std::vector<int> sprite_ids(view.NumCells());
  // Render centered on player.
  grid.Render(math::Transform2d{{2, 2}, math::Orientation2d::kNorth}, view,
              absl::MakeSpan(sprite_ids));

  EXPECT_THAT(
      sprite_ids[CenterOffset(view, 0, 0, world.layers().ToHandle("pieces"))],
      Eq(view.ToSpriteId(SpriteInstance{world.sprites().ToHandle("Player"),
                                        math::Orientation2d::kNorth})));
  EXPECT_THAT(
      sprite_ids[CenterOffset(view, -1, 0, world.layers().ToHandle("fruit"))],
      Eq(view.ToSpriteId(SpriteInstance{world.sprites().ToHandle("Apple"),
                                        math::Orientation2d::kNorth})));
  EXPECT_THAT(
      sprite_ids[CenterOffset(view, -2, 0, world.layers().ToHandle("pieces"))],
      Eq(view.ToSpriteId(SpriteInstance{world.sprites().ToHandle("*Wall"),
                                        math::Orientation2d::kNorth})));
  EXPECT_THAT(
      sprite_ids[CenterOffset(view, -3, 0, world.layers().ToHandle("pieces"))],
      Eq(view.ToSpriteId(SpriteInstance{world.sprites().ToHandle("OutOfBounds"),
                                        math::Orientation2d::kNorth})));
  EXPECT_THAT(
      sprite_ids[CenterOffset(view, 0, 0, world.layers().ToHandle("pieces"))],
      Eq(view.ToSpriteId(SpriteInstance{world.sprites().ToHandle("Player"),
                                        math::Orientation2d::kNorth})));

  EXPECT_THAT(
      sprite_ids[CenterOffset(view, 0, -1, world.layers().ToHandle("fruit"))],
      Eq(0));
  EXPECT_THAT(
      sprite_ids[CenterOffset(view, 0, -2, world.layers().ToHandle("pieces"))],
      Eq(view.ToSpriteId(SpriteInstance{world.sprites().ToHandle("*Wall"),
                                        math::Orientation2d::kNorth})));
  EXPECT_THAT(
      sprite_ids[CenterOffset(view, 0, -3, world.layers().ToHandle("pieces"))],
      Eq(view.ToSpriteId(SpriteInstance{world.sprites().ToHandle("OutOfBounds"),
                                        math::Orientation2d::kNorth})));
}

constexpr const absl::string_view kPlayerMoveGridRelative = R"(
 A  *
 A  *
 A  *
*****
)";

TEST(GridTest, PlayerMoveGridRelativeWorks) {
  std::mt19937_64 random;
  const World world(CreateWorldArgs());
  GridView view = CreateGridView(world, /*left=*/3, /*right=*/3,
                                 /*forward=*/6, /*backward=*/1);
  CharMap char_to_state = {};
  char_to_state['P'] = world.states().ToHandle("Player");
  char_to_state['*'] = world.states().ToHandle("Wall");
  char_to_state['A'] = world.states().ToHandle("Apple");

  Grid grid(world, GetSize2dOfText(kPlayerMoveGridRelative),
            GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, kPlayerMoveGridRelative, math::Orientation2d::kNorth,
            &grid);
  math::Transform2d player_transform = {{0, 0}, math::Orientation2d::kEast};
  Piece player = grid.CreateInstance(char_to_state['P'], player_transform);

  // PA  *
  //  A  *
  //  A  *
  // *****
  // Can't move off grid.
  grid.PushPiece(player, math::Orientation2d::kWest, Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform));
  grid.PushPiece(player, math::Orientation2d::kNorth, Grid::Perspective::kGrid);
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform));

  // Can move South twice.
  // vA  *
  // PA  *
  //  A  *
  // *****
  grid.PushPiece(player, math::Orientation2d::kSouth, Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  player_transform.position += math::Vector2d::South();
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform));

  // Ensure sprite in correct location.
  EXPECT_THAT(grid.AllSpriteInstances(
                      player_transform
                          .position)[world.layers().ToHandle("pieces").Value()]
                  .orientation,
              Eq(player_transform.orientation));

  //  A  *
  // vA  *
  // PA  *
  // *****
  grid.PushPiece(player, math::Orientation2d::kSouth, Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  player_transform.position += math::Vector2d::South();
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform));

  // Stops at wall.
  grid.PushPiece(player, math::Orientation2d::kSouth, Grid::Perspective::kGrid);
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform));

  // Step over apple.
  //  A  *
  //  A  *
  // >P  *
  // *****
  grid.PushPiece(player, math::Orientation2d::kEast, Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  player_transform.position += math::Vector2d::East();
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform))
      << grid.ToString();
  //  A  *
  //  A  *
  //  >P *
  // *****
  grid.PushPiece(player, math::Orientation2d::kEast, Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  player_transform.position += math::Vector2d::East();
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform))
      << grid.ToString();
  //  A  *
  //  A  *
  //  A>P*
  // *****
  grid.PushPiece(player, math::Orientation2d::kEast, Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  player_transform.position += math::Vector2d::East();
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform))
      << grid.ToString();

  // Return to origin.
  //  A  *
  //  A P*
  //  A ^*
  // *****
  grid.PushPiece(player, math::Orientation2d::kNorth, Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  //  A P*
  //  A ^*
  //  A  *
  // *****
  grid.PushPiece(player, math::Orientation2d::kNorth, Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  //  AP<*
  //  A  *
  //  A  *
  // *****
  grid.PushPiece(player, math::Orientation2d::kWest, Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  //  P< *
  //  A  *
  //  A  *
  // *****
  grid.PushPiece(player, math::Orientation2d::kWest, Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  // P<  *
  //  A  *
  //  A  *
  // *****
  grid.PushPiece(player, math::Orientation2d::kWest, Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  player_transform.position = math::Position2d{0, 0};
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform))
      << grid.ToString();
}

constexpr const absl::string_view kPlayerCanTeleport = R"(
   *
 * *
   *
****
)";

TEST(GridTest, PlayerCanTeleport) {
  std::mt19937_64 random;
  const World world(CreateWorldArgs());
  GridView view = CreateGridView(world, /*left=*/3, /*right=*/3,
                                 /*forward=*/6, /*backward=*/1);
  CharMap char_to_state = {};
  char_to_state['P'] = world.states().ToHandle("Player");
  char_to_state['*'] = world.states().ToHandle("Wall");

  Grid grid(world, GetSize2dOfText(kPlayerCanTeleport),
            GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, kPlayerCanTeleport, math::Orientation2d::kNorth,
            &grid);
  math::Transform2d player_transform = {{0, 0}, math::Orientation2d::kEast};
  // Create two off grid.
  Piece player0 = grid.CreateInstance(char_to_state['P'], player_transform);
  player_transform.position.x += 1;
  Piece player1 = grid.CreateInstance(char_to_state['P'], player_transform);
  EXPECT_THAT(grid.GetPieceTransform(player0).position,
              Eq(math::Position2d{0, 0}));
  EXPECT_THAT(grid.GetPieceTransform(player1).position,
              Eq(math::Position2d{1, 0}));
  // Place player0 on grid.
  grid.TeleportPiece(player0, {2, 2}, Grid::TeleportOrientation::kKeepOriginal);
  grid.DoUpdate(&random);
  EXPECT_THAT(grid.GetPieceTransform(player0).position,
              Eq(math::Position2d{2, 2}));

  // Fail to place player1 on same location.
  grid.TeleportPiece(player1, {2, 2}, Grid::TeleportOrientation::kKeepOriginal);
  grid.DoUpdate(&random);
  EXPECT_THAT(grid.GetPieceTransform(player1).position,
              Eq(math::Position2d{1, 0}));

  // Move player0 away.
  grid.TeleportPiece(player0, {0, 0}, Grid::TeleportOrientation::kKeepOriginal);
  grid.DoUpdate(&random);
  EXPECT_THAT(grid.GetPieceTransform(player0).position,
              Eq(math::Position2d{0, 0}));

  // Now place player1.
  grid.TeleportPiece(player1, {2, 2}, Grid::TeleportOrientation::kKeepOriginal);
  grid.DoUpdate(&random);
  EXPECT_THAT(grid.GetPieceTransform(player1).position,
              Eq(math::Position2d{2, 2}));

  // Same location.
  grid.TeleportPiece(player1, {2, 2}, Grid::TeleportOrientation::kKeepOriginal);
  grid.DoUpdate(&random);
  EXPECT_THAT(grid.GetPieceTransform(player1).position,
              Eq(math::Position2d{2, 2}));

  // Move off screen.
  grid.TeleportPiece(player0, {-1, -1},
                     Grid::TeleportOrientation::kKeepOriginal);
  grid.DoUpdate(&random);
  EXPECT_THAT(grid.GetPieceTransform(player0).position,
              Eq(math::Position2d{0, 0}));

  grid.TeleportPiece(player1, {2, 0}, Grid::TeleportOrientation::kKeepOriginal);
  grid.DoUpdate(&random);
  EXPECT_THAT(grid.GetPieceTransform(player1).position,
              Eq(math::Position2d{2, 0}));
}

constexpr const absl::string_view kPlayerCanTeleportToGroup = R"(
SSS*
   *
****
)";

TEST(GridTest, PlayerCanTeleportToGroupAndFlush) {
  std::mt19937_64 random;
  World::Args args = CreateWorldArgs();
  args.states["Spawn"].group_names = {"spawns"};
  const World world(args);
  GridView view = CreateGridView(world, /*left=*/3, /*right=*/3,
                                 /*forward=*/6, /*backward=*/1);
  CharMap char_to_state = {};
  char_to_state['S'] = world.states().ToHandle("Spawn");
  char_to_state['P'] = world.states().ToHandle("Player");
  char_to_state['*'] = world.states().ToHandle("Wall");
  Group spawn_group = world.groups().ToHandle("spawns");
  ASSERT_FALSE(spawn_group.IsEmpty());

  Grid grid(world, GetSize2dOfText(kPlayerCanTeleportToGroup),
            GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, kPlayerCanTeleportToGroup,
            math::Orientation2d::kNorth, &grid);
  math::Transform2d player_transform = {{-1, -1}, math::Orientation2d::kEast};
  Piece piece0 = grid.CreateInstance(char_to_state['P'], player_transform);
  Piece piece1 = grid.CreateInstance(char_to_state['P'], player_transform);
  Piece piece2 = grid.CreateInstance(char_to_state['P'], player_transform);
  Piece piece3 = grid.CreateInstance(char_to_state['P'], player_transform);
  grid.TeleportToGroup(piece0, spawn_group, State(),
                       Grid::TeleportOrientation::kMatchTarget);
  grid.DoUpdate(&random);
  EXPECT_THAT(grid.GetPieceTransform(piece0).position.y, Eq(0));
  grid.TeleportPiece(piece0, {0, 0}, Grid::TeleportOrientation::kKeepOriginal);
  grid.TeleportToGroup(piece1, spawn_group, State(),
                       Grid::TeleportOrientation::kMatchTarget);
  grid.TeleportToGroup(piece2, spawn_group, State(),
                       Grid::TeleportOrientation::kMatchTarget);
  grid.TeleportToGroup(piece3, spawn_group, State(),
                       Grid::TeleportOrientation::kMatchTarget);
  grid.DoUpdate(&random);
  EXPECT_THAT(grid.GetPieceTransform(piece1).position.y, Eq(0));
  EXPECT_THAT(grid.GetPieceTransform(piece2).position.y, Eq(0));
  EXPECT_THAT(grid.GetPieceTransform(piece3).position.y, Eq(-1));
  std::array<int, 3> xs = {grid.GetPieceTransform(piece0).position.x,
                           grid.GetPieceTransform(piece1).position.x,
                           grid.GetPieceTransform(piece2).position.x};
  EXPECT_THAT(xs, UnorderedElementsAre(0, 1, 2));
  grid.TeleportPiece(piece0, {0, 1}, Grid::TeleportOrientation::kKeepOriginal);
  grid.DoUpdate(&random, 1);
  EXPECT_THAT(grid.GetPieceTransform(piece3).position.y, Eq(0));
}

constexpr const absl::string_view kPlayerMoveRelative = R"(
   *
 * *
   *
****
)";

TEST(GridTest, PlayerMoveRelativeEastWorks) {
  std::mt19937_64 random;
  const World world(CreateWorldArgs());
  GridView view = CreateGridView(world, /*left=*/3, /*right=*/3,
                                 /*forward=*/6, /*backward=*/1);
  CharMap char_to_state = {};
  char_to_state['P'] = world.states().ToHandle("Player");
  char_to_state['*'] = world.states().ToHandle("Wall");

  Grid grid(world, GetSize2dOfText(kPlayerMoveRelative),
            GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, kPlayerMoveRelative, math::Orientation2d::kNorth,
            &grid);
  math::Transform2d player_transform = {{0, 0}, math::Orientation2d::kEast};
  Piece player = grid.CreateInstance(char_to_state['P'], player_transform);

  // Player is facing left so movement should be relative to their point of
  // view.
  // >P *
  //  * *
  //    *
  // ****
  grid.PushPiece(player, math::Orientation2d::kNorth,
                 Grid::Perspective::kPiece);
  grid.DoUpdate(&random);
  player_transform.position += math::Vector2d::East();
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform))
      << grid.ToString();
  //  >P*
  //  * *
  //    *
  // ****
  grid.PushPiece(player, math::Orientation2d::kNorth,
                 Grid::Perspective::kPiece);
  grid.DoUpdate(&random);
  player_transform.position += math::Vector2d::East();
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform))
      << grid.ToString();
  //   v*
  //  *P*
  //    *
  // ****
  grid.PushPiece(player, math::Orientation2d::kEast, Grid::Perspective::kPiece);
  grid.DoUpdate(&random);
  player_transform.position += math::Vector2d::South();
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform))
      << grid.ToString();
  //    *
  //  *v*
  //   P*
  // ****
  grid.PushPiece(player, math::Orientation2d::kEast, Grid::Perspective::kPiece);
  grid.DoUpdate(&random);
  player_transform.position += math::Vector2d::South();
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform))
      << grid.ToString();
  //    *
  //  * *
  //  P<*
  // ****
  grid.PushPiece(player, math::Orientation2d::kSouth,
                 Grid::Perspective::kPiece);
  grid.DoUpdate(&random);
  player_transform.position += math::Vector2d::West();
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform))
      << grid.ToString();
  //    *
  //  * *
  // P< *
  // ****
  grid.PushPiece(player, math::Orientation2d::kSouth,
                 Grid::Perspective::kPiece);
  grid.DoUpdate(&random);
  player_transform.position += math::Vector2d::West();
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform))
      << grid.ToString();
  //    *
  // P* *
  // ^  *
  // ****
  grid.PushPiece(player, math::Orientation2d::kWest, Grid::Perspective::kPiece);
  grid.DoUpdate(&random);
  player_transform.position += math::Vector2d::North();
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform))
      << grid.ToString();
  // P  *
  // ^* *
  //    *
  // ****
  grid.PushPiece(player, math::Orientation2d::kWest, Grid::Perspective::kPiece);
  grid.DoUpdate(&random);
  player_transform.position += math::Vector2d::North();
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform))
      << grid.ToString();
}

constexpr const absl::string_view kPlayerCantMoveOffGrid = R"(
   *
 * *
   *
****
)";

TEST(GridTest, PlayerCantMoveOffGridWorks) {
  std::mt19937_64 random;
  const World world(CreateWorldArgs());
  GridView view = CreateGridView(world, /*left=*/3, /*right=*/3,
                                 /*forward=*/6, /*backward=*/1);
  CharMap char_to_state = {};
  char_to_state['P'] = world.states().ToHandle("Player");
  char_to_state['*'] = world.states().ToHandle("Wall");

  Grid grid(world, GetSize2dOfText(kPlayerCantMoveOffGrid),
            GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, kPlayerCantMoveOffGrid, math::Orientation2d::kNorth,
            &grid);
  math::Transform2d player_transform = {{-1, -1}, math::Orientation2d::kNorth};
  Piece player = grid.CreateInstance(char_to_state['P'], player_transform);
  ASSERT_FALSE(player.IsEmpty());
  for (auto orientation :
       {math::Orientation2d::kNorth, math::Orientation2d::kEast,
        math::Orientation2d::kSouth, math::Orientation2d::kWest}) {
    grid.PushPiece(player, orientation, Grid::Perspective::kGrid);
    grid.PushPiece(player, orientation, Grid::Perspective::kPiece);
  }
}

constexpr const absl::string_view kCanMoveInisible = R"(
   *
 * *
   *
****
)";

TEST(GridTest, CanMoveInisible) {
  std::mt19937_64 random;
  const World world(CreateWorldArgs());
  GridView view = CreateGridView(world, /*left=*/3, /*right=*/3,
                                 /*forward=*/6, /*backward=*/1);
  CharMap char_to_state = {};
  char_to_state['S'] = world.states().ToHandle("Spawn");
  char_to_state['*'] = world.states().ToHandle("Wall");

  Grid grid(world, GetSize2dOfText(kCanMoveInisible),
            GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, kCanMoveInisible, math::Orientation2d::kNorth,
            &grid);
  math::Transform2d spawn_transform = {{0, 0}, math::Orientation2d::kEast};
  Piece spawn = grid.CreateInstance(char_to_state['S'], spawn_transform);
  grid.PushPiece(spawn, math::Orientation2d::kWest, Grid::Perspective::kGrid);
  grid.PushPiece(spawn, math::Orientation2d::kEast, Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  spawn_transform.position += math::Vector2d::East();
  EXPECT_THAT(grid.GetPieceTransform(spawn), Eq(spawn_transform));
}

TEST(GridTest, PlayerCanRotate) {
  std::mt19937_64 random;
  const World world(CreateWorldArgs());
  GridView view = CreateGridView(world, /*left=*/3, /*right=*/3,
                                 /*forward=*/6, /*backward=*/1);
  auto player_state = world.states().ToHandle("Player");

  Grid grid(world, math::Size2d{1, 1}, GridShape::Topology::kBounded);
  math::Transform2d player_transform = {{0, 0}, math::Orientation2d::kNorth};
  Piece player = grid.CreateInstance(player_state, player_transform);
  EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform));
  for (auto orientation :
       {math::Orientation2d::kNorth, math::Orientation2d::kEast,
        math::Orientation2d::kSouth, math::Orientation2d::kWest}) {
    grid.SetPieceOrientation(player, orientation);
    grid.DoUpdate(&random);
    player_transform.orientation = orientation;
    EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform));
    EXPECT_THAT(
        grid.AllSpriteInstances(
                player_transform
                    .position)[world.layers().ToHandle("pieces").Value()]
            .orientation,
        Eq(orientation));
  }

  for (auto orientation :
       {math::Orientation2d::kNorth, math::Orientation2d::kEast,
        math::Orientation2d::kSouth, math::Orientation2d::kWest}) {
    for (auto rotate : {math::Rotate2d::k0, math::Rotate2d::k90,
                        math::Rotate2d::k180, math::Rotate2d::k270}) {
      grid.SetPieceOrientation(player, orientation);
      grid.RotatePiece(player, rotate);
      grid.DoUpdate(&random);
      player_transform.orientation = orientation + rotate;
      EXPECT_THAT(grid.GetPieceTransform(player), Eq(player_transform));
      EXPECT_THAT(
          grid.AllSpriteInstances(
                  player_transform
                      .position)[world.layers().ToHandle("pieces").Value()]
              .orientation,
          Eq(orientation + rotate));
    }
  }
}

//
constexpr const absl::string_view kCanHitBeam = R"(
************
           *
           *
           *
           *
           *
           *
           *
************
)";

constexpr const absl::string_view kCanHitBeamExpect = R"(
************
oooooooo   *
ooooooooo  *
oooooooooo *
Poooooooooo*
oooooooooo *
ooooooooo  *
oooooooo   *
************
)";

TEST(GridTest, CanHitBeam) {
  std::mt19937_64 random;
  World::Args args = CreateWorldArgs();
  args.render_order.push_back("hitLayer0");
  args.hits["Hit0"] = World::HitArg{"hitLayer0", "ohit"};
  const World world(args);
  CharMap char_to_state = {};
  char_to_state['*'] = world.states().ToHandle("Wall");
  Grid grid(world, GetSize2dOfText(kCanHitBeam), GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, kCanHitBeam, math::Orientation2d::kNorth, &grid);
  math::Transform2d player_transform = {{0, 4}, math::Orientation2d::kEast};
  State player_state = world.states().ToHandle("Player");
  Hit hit = world.hits().ToHandle("Hit0");
  Piece player = grid.CreateInstance(player_state, player_transform);
  grid.HitBeam(player, hit, /*length=*/10, /*radius=*/3);
  grid.DoUpdate(&random);
  EXPECT_THAT(RemoveLeadingAndTrailingNewLines(grid.ToString()),
              Eq(RemoveLeadingAndTrailingNewLines(kCanHitBeamExpect)));
}

constexpr const absl::string_view CanHitBeamCallBack0 = R"(
************
           *
  *        *
           *
        *  *
           *
   *       *
 *         *
************
)";

constexpr const absl::string_view kCanHitBeamCallBack0Expect = R"(
************
oooooooo   *
oo*        *
oooooooooo *
Pooooooo*  *
oooooooooo *
ooo*       *
o*         *
************
)";

TEST(GridTest, CanHitBeamCallBack0) {
  std::mt19937_64 random;
  World::Args args = CreateWorldArgs();
  args.render_order.push_back("hitLayer0");
  args.hits["Hit0"] = World::HitArg{"hitLayer0", "ohit"};
  const World world(args);
  CharMap char_to_state = {};
  State wall_state = world.states().ToHandle("Wall");
  char_to_state['*'] = wall_state;
  Grid grid(world, GetSize2dOfText(CanHitBeamCallBack0),
            GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, CanHitBeamCallBack0, math::Orientation2d::kNorth,
            &grid);

  auto mock_wall_state_callback = std::make_unique<MockStateCallback>();
  EXPECT_CALL(*mock_wall_state_callback.get(), OnHit(_, _, _))
      .WillRepeatedly(Return(Grid::HitResponse::kBlocked));
  grid.SetCallback(wall_state, std::move(mock_wall_state_callback));

  math::Transform2d player_transform = {{0, 4}, math::Orientation2d::kEast};
  State player_state = world.states().ToHandle("Player");
  Hit hit = world.hits().ToHandle("Hit0");
  Piece player = grid.CreateInstance(player_state, player_transform);
  grid.HitBeam(player, hit, /*length=*/10, /*radius=*/3);
  grid.DoUpdate(&random);
  EXPECT_THAT(RemoveLeadingAndTrailingNewLines(grid.ToString()),
              Eq(RemoveLeadingAndTrailingNewLines(kCanHitBeamCallBack0Expect)));
}

constexpr const absl::string_view kCanHitBeamCallBack1 = R"(
************
           *
*          *
           *
           *
           *
           *
           *
************
)";

constexpr const absl::string_view kCanHitBeamCallBack1Expect = R"(
************
           *
*          *
oooooooooo *
Poooooooooo*
oooooooooo *
ooooooooo  *
oooooooo   *
************
)";

TEST(GridTest, CanHitBeamCallBack1) {
  std::mt19937_64 random;
  World::Args args = CreateWorldArgs();
  args.render_order.push_back("hitLayer0");
  args.hits["Hit0"] = World::HitArg{"hitLayer0", "ohit"};
  const World world(args);
  CharMap char_to_state = {};
  State wall_state = world.states().ToHandle("Wall");
  char_to_state['*'] = wall_state;
  Grid grid(world, GetSize2dOfText(kCanHitBeamCallBack1),
            GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, kCanHitBeamCallBack1, math::Orientation2d::kNorth,
            &grid);

  auto mock_wall_state_callback = std::make_unique<MockStateCallback>();
  EXPECT_CALL(*mock_wall_state_callback.get(), OnHit(_, _, _))
      .WillRepeatedly(Return(Grid::HitResponse::kBlocked));
  grid.SetCallback(wall_state, std::move(mock_wall_state_callback));

  math::Transform2d player_transform = {{0, 4}, math::Orientation2d::kEast};
  State player_state = world.states().ToHandle("Player");
  Hit hit = world.hits().ToHandle("Hit0");
  Piece player = grid.CreateInstance(player_state, player_transform);
  grid.HitBeam(player, hit, /*length=*/10, /*radius=*/3);
  grid.DoUpdate(&random);
  EXPECT_THAT(RemoveLeadingAndTrailingNewLines(grid.ToString()),
              Eq(RemoveLeadingAndTrailingNewLines(kCanHitBeamCallBack1Expect)));
}

constexpr const absl::string_view kImmediateModeRendering = "***";

TEST(GridTest, ImmediateModeRenderingAfterUpdate) {
  std::mt19937_64 random;
  World::Args args = CreateWorldArgs();
  args.custom_sprites = {"0custom", "1custom"};
  args.render_order = {"pieces", "customLayer0", "customLayer1"};
  const World world(args);
  Sprite custom0 = world.sprites().ToHandle("0custom");
  Sprite custom1 = world.sprites().ToHandle("1custom");
  Layer layer0 = world.layers().ToHandle("customLayer0");
  Layer layer1 = world.layers().ToHandle("customLayer1");
  State wall_state = world.states().ToHandle("Wall");
  CharMap char_to_state = {};
  char_to_state['*'] = wall_state;
  Grid grid(world, GetSize2dOfText(kImmediateModeRendering),
            GridShape::Topology::kBounded);
  PlaceGrid(char_to_state, kImmediateModeRendering, math::Orientation2d::kNorth,
            &grid);
  grid.SetSpriteImmediate({{0, 0}, math::Orientation2d::kNorth}, layer0,
                          custom0);
  grid.SetSpriteImmediate({{0, 0}, math::Orientation2d::kNorth}, layer1,
                          custom1);
  EXPECT_THAT(grid.ToString(), Eq("1**\n"));
  grid.DoUpdate(&random);
  EXPECT_THAT(grid.ToString(), Eq("***\n"));
  grid.SetSpriteImmediate({{0, 0}, math::Orientation2d::kNorth}, layer0,
                          custom1);
  grid.SetSpriteImmediate({{0, 0}, math::Orientation2d::kNorth}, layer1,
                          custom0);
  EXPECT_THAT(grid.ToString(), Eq("0**\n"));
  grid.DoUpdate(&random);
  EXPECT_THAT(grid.ToString(), Eq("***\n"));
}

TEST(GridTest, ConnectedSimple) {
  World::Args args = {};
  args.states["Player"] = World::StateArg{};
  const World world(args);
  Grid grid(world, math::Size2d{1, 1}, GridShape::Topology::kBounded);
  State player_state = world.states().ToHandle("Player");
  math::Transform2d trans = {{-1, -1}, math::Orientation2d::kNorth};

  Piece p0 = grid.CreateInstance(player_state, trans);
  Piece p1 = grid.CreateInstance(player_state, trans);
  Piece p2 = grid.CreateInstance(player_state, trans);
  Piece p3 = grid.CreateInstance(player_state, trans);
  Piece p4 = grid.CreateInstance(player_state, trans);
  auto list_connected = [&grid](Piece root) {
    std::vector<Piece> visited;
    grid.VisitConnected(
        root, [&visited](Piece handle) { visited.push_back(handle); });
    return visited;
  };
  std::mt19937_64 random;
  grid.Connect(p0, p1);
  grid.DoUpdate(&random);
  EXPECT_THAT(list_connected(p0), ElementsAre(p0, p1));
  grid.Connect(p1, p2);
  grid.DoUpdate(&random);
  EXPECT_THAT(list_connected(p0), ElementsAre(p0, p1, p2));
  grid.Connect(p2, p3);
  grid.DoUpdate(&random);
  EXPECT_THAT(list_connected(p0), ElementsAre(p0, p1, p2, p3));
  grid.Connect(p3, p4);
  grid.DoUpdate(&random);
  EXPECT_THAT(list_connected(p0), ElementsAre(p0, p1, p2, p3, p4));
  grid.Disconnect(p3);
  grid.DoUpdate(&random);
  EXPECT_THAT(list_connected(p0), ElementsAre(p0, p1, p2, p4));
  grid.Disconnect(p0);
  grid.DoUpdate(&random);
  EXPECT_THAT(list_connected(p1), ElementsAre(p1, p2, p4));
  grid.Disconnect(p2);
  grid.DoUpdate(&random);
  EXPECT_THAT(list_connected(p1), ElementsAre(p1, p4));
  grid.Disconnect(p4);
  grid.DoUpdate(&random);
  EXPECT_THAT(list_connected(p1), ElementsAre(p1));
}

TEST(GridTest, ConnectedTwoRings) {
  World::Args args = {};
  args.states["Player"] = World::StateArg{};
  const World world(args);
  Grid grid(world, math::Size2d{1, 1}, GridShape::Topology::kBounded);
  State player_state = world.states().ToHandle("Player");
  math::Transform2d trans = {{-1, -1}, math::Orientation2d::kNorth};

  Piece p00 = grid.CreateInstance(player_state, trans);
  Piece p01 = grid.CreateInstance(player_state, trans);
  Piece p02 = grid.CreateInstance(player_state, trans);
  Piece p10 = grid.CreateInstance(player_state, trans);
  Piece p11 = grid.CreateInstance(player_state, trans);
  Piece p12 = grid.CreateInstance(player_state, trans);

  grid.Connect(p00, p01);
  grid.Connect(p01, p02);
  grid.Connect(p10, p11);
  grid.Connect(p11, p12);
  std::mt19937_64 random;
  grid.DoUpdate(&random);
  auto list_connected = [&grid](Piece root) {
    std::vector<Piece> visited;
    grid.VisitConnected(
        root, [&visited](Piece handle) { visited.push_back(handle); });
    return visited;
  };
  EXPECT_THAT(list_connected(p00), ElementsAre(p00, p01, p02));
  EXPECT_THAT(list_connected(p10), ElementsAre(p10, p11, p12));
  grid.Connect(p00, p12);
  grid.DoUpdate(&random);
  EXPECT_THAT(list_connected(p00), ElementsAre(p00, p01, p02, p10, p11, p12));
}

TEST(GridTest, ReleaseWithConnectWorks) {
  World::Args args = {};
  args.states["Player"] = World::StateArg{};
  const World world(args);
  Grid grid(world, math::Size2d{1, 1}, GridShape::Topology::kBounded);
  State player_state = world.states().ToHandle("Player");
  math::Transform2d trans = {{-1, -1}, math::Orientation2d::kNorth};
  Piece p0 = grid.CreateInstance(player_state, trans);
  Piece p1 = grid.CreateInstance(player_state, trans);
  Piece p2 = grid.CreateInstance(player_state, trans);
  grid.Connect(p0, p1);
  grid.Connect(p0, p2);
  grid.ReleaseInstance(p1);
  std::mt19937_64 random;
  grid.DoUpdate(&random);
  auto list_connected = [&grid](Piece root) {
    std::vector<Piece> visited;
    grid.VisitConnected(
        root, [&visited](Piece handle) { visited.push_back(handle); });
    return visited;
  };
  EXPECT_THAT(list_connected(p0), ElementsAre(p0, p2));
}

TEST(GridTest, ConnectedToSelf) {
  World::Args args = {};
  args.states["Player"] = World::StateArg{};
  const World world(args);
  Grid grid(world, math::Size2d{1, 1}, GridShape::Topology::kBounded);
  State player_state = world.states().ToHandle("Player");
  math::Transform2d trans = {{-1, -1}, math::Orientation2d::kNorth};

  Piece p0 = grid.CreateInstance(player_state, trans);
  Piece p1 = grid.CreateInstance(player_state, trans);
  Piece p2 = grid.CreateInstance(player_state, trans);
  Piece p3 = grid.CreateInstance(player_state, trans);
  Piece p4 = grid.CreateInstance(player_state, trans);
  Piece p5 = grid.CreateInstance(player_state, trans);

  grid.Connect(p0, p1);
  grid.Connect(p1, p2);
  grid.Connect(p2, p3);
  grid.Connect(p3, p4);
  grid.Connect(p4, p5);

  std::mt19937_64 random;
  grid.DoUpdate(&random);
  auto list_connected = [&grid](Piece root) {
    std::vector<Piece> visited;
    grid.VisitConnected(
        root, [&visited](Piece handle) { visited.push_back(handle); });
    return visited;
  };

  EXPECT_THAT(list_connected(p0), ElementsAre(p0, p1, p2, p3, p4, p5));
  grid.Connect(p0, p1);
  grid.DoUpdate(&random);
  EXPECT_THAT(list_connected(p0), ElementsAre(p0, p1, p2, p3, p4, p5));
  grid.Connect(p0, p2);
  grid.DoUpdate(&random);
  EXPECT_THAT(list_connected(p0), ElementsAre(p0, p1, p2, p3, p4, p5));
  grid.Connect(p0, p3);
  grid.DoUpdate(&random);
  EXPECT_THAT(list_connected(p0), ElementsAre(p0, p1, p2, p3, p4, p5));
  grid.Connect(p0, p4);
  grid.DoUpdate(&random);
  EXPECT_THAT(list_connected(p0), ElementsAre(p0, p1, p2, p3, p4, p5));
  grid.Connect(p0, p5);
  grid.DoUpdate(&random);
  EXPECT_THAT(list_connected(p0), ElementsAre(p0, p1, p2, p3, p4, p5));
}

TEST(GridTest, DisconnectAll) {
  World::Args args = {};
  args.states["Player"] = World::StateArg{};
  const World world(args);
  Grid grid(world, math::Size2d{1, 1}, GridShape::Topology::kBounded);
  State player_state = world.states().ToHandle("Player");
  math::Transform2d trans = {{-1, -1}, math::Orientation2d::kNorth};

  Piece p0 = grid.CreateInstance(player_state, trans);
  Piece p1 = grid.CreateInstance(player_state, trans);
  Piece p2 = grid.CreateInstance(player_state, trans);
  Piece p3 = grid.CreateInstance(player_state, trans);
  Piece p4 = grid.CreateInstance(player_state, trans);
  Piece p5 = grid.CreateInstance(player_state, trans);
  grid.Connect(p0, p1);
  grid.Connect(p1, p2);
  grid.Connect(p2, p3);
  grid.Connect(p3, p4);
  grid.Connect(p4, p5);
  std::mt19937_64 random;
  grid.DoUpdate(&random);
  auto list_connected = [&grid](Piece root) {
    std::vector<Piece> visited;
    grid.VisitConnected(
        root, [&visited](Piece handle) { visited.push_back(handle); });
    return visited;
  };

  EXPECT_THAT(list_connected(p0), ElementsAre(p0, p1, p2, p3, p4, p5));
  grid.DisconnectAll(p0);
  grid.DoUpdate(&random);
  EXPECT_THAT(list_connected(p0), ElementsAre(p0));
  EXPECT_THAT(list_connected(p1), ElementsAre(p1));
  EXPECT_THAT(list_connected(p2), ElementsAre(p2));
  EXPECT_THAT(list_connected(p3), ElementsAre(p3));
  EXPECT_THAT(list_connected(p4), ElementsAre(p4));
  EXPECT_THAT(list_connected(p5), ElementsAre(p5));
}

TEST(GridTest, ConnectedMove) {
  World::Args args = {};
  args.states["Player"] = World::StateArg{"piece", "player"};
  args.states["Wall"] = World::StateArg{"piece", "Wall"};
  const World world(args);
  Grid grid(world, math::Size2d{10, 10}, GridShape::Topology::kBounded);
  State player_state = world.states().ToHandle("Player");
  math::Transform2d trans = {{0, 0}, math::Orientation2d::kNorth};
  std::array<std::array<Piece, 3>, 3> pieces;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      trans.position = {j, i};
      pieces[i][j] = grid.CreateInstance(player_state, trans);
      if (i != 0 || j != 0) {
        grid.Connect(pieces[i][j], pieces[0][0]);
      }
    }
  }
  std::mt19937_64 random;

  grid.PushPiece(pieces[1][1], math::Orientation2d::kEast,
                 Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      trans.position = {j + 1, i};
      EXPECT_THAT(grid.GetPieceTransform(pieces[i][j]), Eq(trans));
    }
  }

  grid.PushPiece(pieces[1][1], math::Orientation2d::kSouth,
                 Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      trans.position = {j + 1, i + 1};
      EXPECT_THAT(grid.GetPieceTransform(pieces[i][j]), Eq(trans));
    }
  }

  grid.PushPiece(pieces[1][1], math::Orientation2d::kWest,
                 Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      trans.position = {j, i + 1};
      EXPECT_THAT(grid.GetPieceTransform(pieces[i][j]), Eq(trans));
    }
  }

  grid.PushPiece(pieces[1][1], math::Orientation2d::kNorth,
                 Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      trans.position = {j, i};
      EXPECT_THAT(grid.GetPieceTransform(pieces[i][j]), Eq(trans));
    }
  }
}

TEST(GridTest, ConnectedCantMove) {
  World::Args args = {};
  args.states["Player"] = World::StateArg{"piece", "player"};
  args.states["Wall"] = World::StateArg{"piece", "Wall"};
  const World world(args);
  Grid grid(world, math::Size2d{10, 10}, GridShape::Topology::kBounded);

  auto mock_wall_state_callback = std::make_unique<MockStateCallback>();
  auto* mock = mock_wall_state_callback.get();

  testing::Sequence seq;

  State player_state = world.states().ToHandle("Player");
  grid.SetCallback(player_state, std::move(mock_wall_state_callback));
  math::Transform2d trans = {{0, 0}, math::Orientation2d::kNorth};
  std::array<std::array<Piece, 3>, 3> pieces;
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      trans.position = {j + 1, i + 1};
      pieces[i][j] = grid.CreateInstance(player_state, trans);
      if (i != 0 || j != 0) {
        grid.Connect(pieces[i][j], pieces[0][0]);
      }
    }
  }

  std::mt19937_64 random;
  // Disconnect West Piece only allowing East movement.
  grid.Disconnect(pieces[1][0]);

  // Can't move North West or South again as pieces[1][0] is in the way.
  for (math::Orientation2d orientation :
       {math::Orientation2d::kWest, math::Orientation2d::kNorth,
        math::Orientation2d::kSouth}) {
    grid.PushPiece(pieces[1][1], orientation, Grid::Perspective::kGrid);
    EXPECT_CALL(*mock, OnBlocked(pieces[1][1], pieces[1][0]));
    grid.DoUpdate(&random);
    for (int i = 0; i < 3; ++i) {
      for (int j = 0; j < 3; ++j) {
        trans.position = {j + 1, i + 1};
        EXPECT_THAT(grid.GetPieceTransform(pieces[i][j]), Eq(trans));
      }
    }
  }

  // Can move East still.
  grid.PushPiece(pieces[1][1], math::Orientation2d::kEast,
                 Grid::Perspective::kGrid);

  grid.DoUpdate(&random);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      if (i == 1 && j == 0) {
        trans.position = {j + 1, i + 1};
      } else {
        trans.position = {j + 2, i + 1};
      }
      EXPECT_THAT(grid.GetPieceTransform(pieces[i][j]), Eq(trans));
    }
  }

  // Now can move West.
  grid.PushPiece(pieces[1][1], math::Orientation2d::kWest,
                 Grid::Perspective::kGrid);
  grid.DoUpdate(&random);
  for (int i = 0; i < 3; ++i) {
    for (int j = 0; j < 3; ++j) {
      trans.position = {j + 1, i + 1};
      EXPECT_THAT(grid.GetPieceTransform(pieces[i][j]), Eq(trans));
    }
  }
}

std::string RenderRaycastToWholeGrid(const Grid& grid, math::Position2d start,
                                     Layer layer) {
  const math::Size2d grid_size = grid.GetShape().GridSize2d();
  std::string ray_cast_result;
  ray_cast_result.reserve(grid_size.Area() + grid_size.height + 1);
  ray_cast_result += '\n';
  for (int i = 0; i < grid_size.height; ++i) {
    for (int j = 0; j < grid_size.width; ++j) {
      const math::Position2d end{j, i};
      auto hit = grid.RayCast(layer, start, end);
      if (hit.has_value()) {
        if (hit->position == end) {
          ray_cast_result += '*';
        } else {
          ray_cast_result += 'x';
        }
      } else {
        ray_cast_result += '.';
      }
    }
    ray_cast_result += '\n';
  }
  return ray_cast_result;
}

constexpr const absl::string_view kRayCastTest = R"(
         *
         *
         *
         *
         *
****     *
         *
   ****  *
         *
)";

constexpr const absl::string_view kRayCastResultExpectedFromTopLeft = R"(
.........*
.........*
.........*
.........*
.........*
****.....*
xxxxx....*
xxxxxx*..*
xxxxxxxx.*
)";

constexpr const absl::string_view kRayCastResultExpectedFromTopRight = R"(
.........*
.........x
.........x
.........x
.........x
xxx*.....x
xxxx.....x
xxx****..x
xxxxxxx..x
)";

constexpr const absl::string_view kRayCastResultExpectedFromBottomRight = R"(
x........x
xx.......x
xxx......x
xxxx.....x
xxxxx....x
xxxxxx...x
xxxxxxx..x
xxxxxx*..x
.........*
)";

constexpr const absl::string_view kRayCastResultExpectedFromBottomLeft = R"(
xxxxxxxxxx
xxxxxxxxxx
xxxxxxxxx*
xxxxxxx..x
xxxxxx.xxx
****xxxxxx
....xxxxxx
...*xxxxxx
.........*
)";

TEST(GridTest, RayCast) {
  std::mt19937_64 random;
  World::Args args = CreateWorldArgs();
  const World world(args);
  CharMap char_to_state = {};
  State wall_state = world.states().ToHandle("Wall");
  char_to_state['*'] = wall_state;
  math::Size2d grid_size = GetSize2dOfText(kRayCastTest);
  Grid grid(world, grid_size, GridShape::Topology::kBounded);

  PlaceGrid(char_to_state, kRayCastTest, math::Orientation2d::kNorth, &grid);

  Layer piece_layer = world.layers().ToHandle("pieces");

  std::string raycasts_top_left =
      RenderRaycastToWholeGrid(grid, {0, 0}, piece_layer);

  EXPECT_THAT(raycasts_top_left, Eq(kRayCastResultExpectedFromTopLeft))
      << "Expected:" << kRayCastResultExpectedFromTopLeft  //
      << "Actual:" << raycasts_top_left;

  std::string raycasts_top_right =
      RenderRaycastToWholeGrid(grid, {grid_size.width - 2, 0}, piece_layer);

  EXPECT_THAT(raycasts_top_right, Eq(kRayCastResultExpectedFromTopRight))
      << "Expected:" << kRayCastResultExpectedFromTopRight  //
      << "Actual:" << raycasts_top_right;

  std::string raycasts_bottom_right = RenderRaycastToWholeGrid(
      grid, {grid_size.width - 2, grid_size.height - 1}, piece_layer);

  EXPECT_THAT(raycasts_bottom_right, Eq(kRayCastResultExpectedFromBottomRight))
      << "Expected:" << kRayCastResultExpectedFromBottomRight  //
      << "Actual:" << raycasts_bottom_right;

  std::string raycasts_bottom_left =
      RenderRaycastToWholeGrid(grid, {0, grid_size.height - 1}, piece_layer);

  EXPECT_THAT(raycasts_bottom_left, Eq(kRayCastResultExpectedFromBottomLeft))
      << "Expected:" << kRayCastResultExpectedFromBottomLeft  //
      << "Actual:" << raycasts_bottom_left;
}

constexpr const absl::string_view kRayCastTorusTest =
    "+ *     * \n"
    " *       *\n"
    "*         \n"
    "          \n"
    "          \n"
    "          \n"
    "          \n"
    "          \n"
    "*        *\n"
    " *      * \n";
TEST(GridTest, RayCastTorus) {
  std::mt19937_64 random;
  World::Args args = CreateWorldArgs();
  args.states["Wall"].group_names.push_back("walls");
  const World world(args);
  CharMap char_to_state = {};
  State wall_state = world.states().ToHandle("Wall");
  char_to_state['*'] = wall_state;
  math::Size2d grid_size = GetSize2dOfText(kRayCastTorusTest);
  Grid grid(world, grid_size, GridShape::Topology::kTorus);
  PlaceGrid(char_to_state, kRayCastTorusTest, math::Orientation2d::kNorth,
            &grid);
  Layer piece_layer = world.layers().ToHandle("pieces");
  const math::Position2d targets[] = {{8, 0}, {9, 1}, {0, 8},
                                      {1, 9}, {8, 9}, {9, 8}};
  constexpr auto zero = math::Position2d{0, 0};
  // Allowed to short cut.
  for (auto target : targets) {
    auto hit = grid.RayCast(piece_layer, zero, target);
    ASSERT_THAT(hit.has_value(), IsTrue());
    EXPECT_THAT(grid.GetPieceTransform(hit->piece).position, Eq(target));
  }

  // Not allowed to short cut.
  for (auto target : targets) {
    auto hit = grid.RayCastDirection(piece_layer, zero, target - zero);
    ASSERT_THAT(hit.has_value(), IsTrue());
    EXPECT_THAT(grid.GetPieceTransform(hit->piece).position, Ne(target));
  }
}

TEST(GridTest, RayCastToOutOfBounds) {
  std::mt19937_64 random;
  World::Args args = CreateWorldArgs();
  const World world(args);
  Grid grid(world, {10, 10}, GridShape::Topology::kBounded);
  Layer piece_layer = world.layers().ToHandle("pieces");
  auto leave_grid = grid.RayCast(piece_layer, {5, 5}, {11, 5});
  ASSERT_THAT(leave_grid.has_value(), IsTrue());
  EXPECT_THAT(leave_grid->piece, Eq(Piece()));
  EXPECT_THAT(leave_grid->position, Eq(math::Position2d{9, 5}));
}

TEST(GridTest, RayCastFromOutOfBounds) {
  std::mt19937_64 random;
  World::Args args = CreateWorldArgs();
  const World world(args);
  Grid grid(world, {10, 10}, GridShape::Topology::kBounded);
  Layer piece_layer = world.layers().ToHandle("pieces");
  auto leave_grid = grid.RayCast(piece_layer, {11, 5}, {5, 5});
  ASSERT_THAT(leave_grid.has_value(), IsTrue());
  EXPECT_THAT(leave_grid->piece, Eq(Piece()));
  EXPECT_THAT(leave_grid->position, Eq(math::Position2d{11, 5}));
}

constexpr const absl::string_view kDiscFindAllTest = R"(
************
*****P     *
**PPPP     *
**PPPP     *
**PPPP     *
*PPPPP     *
)";

TEST(GridTest, DiscFindAllTest) {
  World::Args args = CreateWorldArgs();
  const World world(args);
  math::Size2d grid_size = GetSize2dOfText(kDiscFindAllTest);
  Grid grid(world, grid_size, GridShape::Topology::kBounded);
  CharMap char_to_state = {};
  State wall_state = world.states().ToHandle("Wall");
  State player_state = world.states().ToHandle("Player");
  Layer pieces_layer = world.layers().ToHandle("pieces");
  char_to_state['*'] = wall_state;
  char_to_state['P'] = player_state;
  PlaceGrid(char_to_state, kDiscFindAllTest, math::Orientation2d::kNorth,
            &grid);
  Piece out_side_piece = grid.GetPieceAtPosition(pieces_layer, {2, 2});

  int num_p = std::count(kDiscFindAllTest.begin(), kDiscFindAllTest.end(), 'P');
  auto hits4 = grid.DiscFindAll(pieces_layer, {5, 5}, 4);
  EXPECT_THAT(hits4.size(), Eq(num_p - 1));
  for (auto& hit : hits4) {
    EXPECT_THAT(hit.piece, Ne(out_side_piece));
    EXPECT_THAT(grid.GetState(hit.piece), Eq(player_state));
  }

  auto hits5 = grid.DiscFindAll(pieces_layer, {5, 5}, 5);
  auto hit_outside = std::count_if(hits5.begin(), hits5.end(),
                                   [out_side_piece](const auto& hit) {
                                     return hit.piece == out_side_piece;
                                   });
  EXPECT_THAT(hit_outside, Eq(1));
}

constexpr const absl::string_view kDiamondFindAllTest = R"(
************
*****P     *
****PP     *
**PPPP     *
**PPPP     *
*PPPPP     *
)";

TEST(GridTest, DiamondFindAllTest) {
  World::Args args = CreateWorldArgs();
  const World world(args);
  math::Size2d grid_size = GetSize2dOfText(kDiamondFindAllTest);
  Grid grid(world, grid_size, GridShape::Topology::kBounded);
  CharMap char_to_state = {};
  State wall_state = world.states().ToHandle("Wall");
  State player_state = world.states().ToHandle("Player");
  Layer pieces_layer = world.layers().ToHandle("pieces");
  char_to_state['*'] = wall_state;
  char_to_state['P'] = player_state;
  PlaceGrid(char_to_state, kDiamondFindAllTest, math::Orientation2d::kNorth,
            &grid);
  Piece out_side_piece = grid.GetPieceAtPosition(pieces_layer, {2, 3});
  int num_p =
      std::count(kDiamondFindAllTest.begin(), kDiamondFindAllTest.end(), 'P');
  auto hits4 = grid.DiamondFindAll(pieces_layer, {5, 5}, 4);
  EXPECT_THAT(hits4.size(), Eq(num_p - 1));
  for (auto& hit : hits4) {
    EXPECT_THAT(hit.piece, Ne(out_side_piece));
    EXPECT_THAT(grid.GetState(hit.piece), Eq(player_state));
  }

  auto hits5 = grid.DiamondFindAll(pieces_layer, {5, 5}, 5);
  auto hit_outside = std::count_if(hits5.begin(), hits5.end(),
                                   [out_side_piece](const auto& hit) {
                                     return hit.piece == out_side_piece;
                                   });
  EXPECT_THAT(hit_outside, Eq(1));
}

TEST(GridTest, RectangleBoundedFindAllTest) {
  World::Args args = CreateWorldArgs();
  const World world(args);
  math::Size2d grid_size = {10, 10};
  Grid grid(world, grid_size, GridShape::Topology::kBounded);
  State player_state = world.states().ToHandle("Player");
  Layer pieces_layer = world.layers().ToHandle("pieces");
  auto orientation = math::Orientation2d::kNorth;
  grid.CreateInstance(player_state, math::Transform2d{{0, 0}, orientation});

  auto piece_1_1 =
      grid.CreateInstance(player_state, math::Transform2d{{1, 1}, orientation});

  auto piece_3_1 =
      grid.CreateInstance(player_state, math::Transform2d{{3, 1}, orientation});

  auto piece_3_3 =
      grid.CreateInstance(player_state, math::Transform2d{{3, 3}, orientation});

  auto piece_4_3 =
      grid.CreateInstance(player_state, math::Transform2d{{4, 3}, orientation});

  grid.CreateInstance(player_state, math::Transform2d{{3, 4}, orientation});
  auto hits = grid.RectangleFindAll(pieces_layer, {1, 1}, {4, 3});
  std::sort(
      hits.begin(), hits.end(),
      [](const Grid::FindPieceResult& lhs, const Grid::FindPieceResult& rhs) {
        return lhs.piece < rhs.piece;
      });
  ASSERT_THAT(hits.size(), Eq(4));

  EXPECT_THAT(hits[0].piece, Eq(piece_1_1));
  EXPECT_THAT(hits[0].position, Eq(math::Position2d{1, 1}));
  EXPECT_THAT(hits[1].piece, Eq(piece_3_1));
  EXPECT_THAT(hits[1].position, Eq(math::Position2d{3, 1}));
  EXPECT_THAT(hits[2].piece, Eq(piece_3_3));
  EXPECT_THAT(hits[2].position, Eq(math::Position2d{3, 3}));
  EXPECT_THAT(hits[3].piece, Eq(piece_4_3));
  EXPECT_THAT(hits[3].position, Eq(math::Position2d{4, 3}));
}

TEST(GridTest, RectangleTorusFindAllTest) {
  World::Args args = CreateWorldArgs();
  const World world(args);
  math::Size2d grid_size = {10, 10};
  Grid grid(world, grid_size, GridShape::Topology::kTorus);
  State player_state = world.states().ToHandle("Player");
  Layer pieces_layer = world.layers().ToHandle("pieces");
  auto orientation = math::Orientation2d::kNorth;
  grid.CreateInstance(player_state, math::Transform2d{{0, 0}, orientation});

  auto piece_1_1 =
      grid.CreateInstance(player_state, math::Transform2d{{1, 1}, orientation});

  auto piece_3_1 =
      grid.CreateInstance(player_state, math::Transform2d{{3, 1}, orientation});

  auto piece_3_3 =
      grid.CreateInstance(player_state, math::Transform2d{{3, 3}, orientation});

  auto piece_4_3 =
      grid.CreateInstance(player_state, math::Transform2d{{4, 3}, orientation});

  grid.CreateInstance(player_state, math::Transform2d{{3, 4}, orientation});
  auto hits =
      grid.RectangleFindAll(pieces_layer, {10 + 1, 1 - 20}, {10 + 4, 3 - 20});
  std::sort(
      hits.begin(), hits.end(),
      [](const Grid::FindPieceResult& lhs, const Grid::FindPieceResult& rhs) {
        return lhs.piece < rhs.piece;
      });
  ASSERT_THAT(hits.size(), Eq(4));

  EXPECT_THAT(hits[0].piece, Eq(piece_1_1));
  EXPECT_THAT(hits[1].piece, Eq(piece_3_1));
  EXPECT_THAT(hits[2].piece, Eq(piece_3_3));
  EXPECT_THAT(hits[3].piece, Eq(piece_4_3));
}

TEST(GridTest, UserState) {
  World::Args args = CreateWorldArgs();
  const World world(args);
  math::Size2d grid_size = {10, 10};
  Grid grid(world, grid_size, GridShape::Topology::kBounded);
  State player_state = world.states().ToHandle("Player");
  auto piece = grid.CreateInstance(
      player_state, math::Transform2d{{0, 0}, math::Orientation2d::kNorth});
  grid.SetUserState(piece, 10);
  for (int i = 1; i < 10; ++i) {
    for (int j = 1; j < 10; ++j) {
      grid.CreateInstance(
          player_state, math::Transform2d{{0, 0}, math::Orientation2d::kNorth});
    }
  }
  EXPECT_THAT(absl::any_cast<int>(grid.GetUserState(piece)), Eq(10));
}

}  // namespace
}  // namespace deepmind::lab2d
