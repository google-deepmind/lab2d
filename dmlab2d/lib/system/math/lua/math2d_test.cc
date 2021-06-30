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

#include "dmlab2d/lib/system/math/lua/math2d.h"

#include <array>

#include "absl/strings/string_view.h"
#include "dmlab2d/lib/lua/push.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/lua/table_ref.h"
#include "dmlab2d/lib/lua/vm_test_util.h"
#include "dmlab2d/lib/system/math/math2d.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d::math {
namespace {

using testing::Eq;
using LuaMath2dTest = lua::testing::TestWithVm;

TEST_F(LuaMath2dTest, PushRotate2) {
  Push(L, Rotate2d::k0);
  Push(L, Rotate2d::k90);
  Push(L, Rotate2d::k180);
  Push(L, Rotate2d::k270);
  int value = -1;
  EXPECT_TRUE(IsFound(lua::Read(L, 1, &value)));
  EXPECT_THAT(value, Eq(0));
  EXPECT_TRUE(IsFound(lua::Read(L, 2, &value)));
  EXPECT_THAT(value, Eq(1));
  EXPECT_TRUE(IsFound(lua::Read(L, 3, &value)));
  EXPECT_THAT(value, Eq(2));
  EXPECT_TRUE(IsFound(lua::Read(L, 4, &value)));
  EXPECT_THAT(value, Eq(3));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, ReadRotate2) {
  lua::Push(L, 0);
  lua::Push(L, 1);
  lua::Push(L, 2);
  lua::Push(L, 3);
  Rotate2d value = Rotate2d::k270;
  EXPECT_TRUE(IsFound(Read(L, 1, &value)));
  EXPECT_THAT(value, Eq(Rotate2d::k0));
  EXPECT_TRUE(IsFound(Read(L, 2, &value)));
  EXPECT_THAT(value, Eq(Rotate2d::k90));
  EXPECT_TRUE(IsFound(Read(L, 3, &value)));
  EXPECT_THAT(value, Eq(Rotate2d::k180));
  EXPECT_TRUE(IsFound(Read(L, 4, &value)));
  EXPECT_THAT(value, Eq(Rotate2d::k270));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, ReadRotate2Fail) {
  lua::Push(L, "R");
  lua_pushnil(L);
  Rotate2d value;
  EXPECT_TRUE(IsTypeMismatch(Read(L, 1, &value)));
  EXPECT_TRUE(IsNotFound(Read(L, 2, &value)));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, ReadRotate2Normalise) {
  lua::Push(L, 4);
  lua::Push(L, -3);
  lua::Push(L, -2);
  lua::Push(L, -1);
  Rotate2d value = Rotate2d::k270;
  EXPECT_TRUE(IsFound(Read(L, 1, &value)));
  EXPECT_THAT(value, Eq(Rotate2d::k0));
  EXPECT_TRUE(IsFound(Read(L, 2, &value)));
  EXPECT_THAT(value, Eq(Rotate2d::k90));
  EXPECT_TRUE(IsFound(Read(L, 3, &value)));
  EXPECT_THAT(value, Eq(Rotate2d::k180));
  EXPECT_TRUE(IsFound(Read(L, 4, &value)));
  EXPECT_THAT(value, Eq(Rotate2d::k270));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, PushOrientation2) {
  Push(L, Orientation2d::kNorth);
  Push(L, Orientation2d::kEast);
  Push(L, Orientation2d::kSouth);
  Push(L, Orientation2d::kWest);
  absl::string_view value;
  EXPECT_TRUE(IsFound(lua::Read(L, 1, &value)));
  EXPECT_THAT(value, Eq("N"));
  EXPECT_TRUE(IsFound(lua::Read(L, 2, &value)));
  EXPECT_THAT(value, Eq("E"));
  EXPECT_TRUE(IsFound(lua::Read(L, 3, &value)));
  EXPECT_THAT(value, Eq("S"));
  EXPECT_TRUE(IsFound(lua::Read(L, 4, &value)));
  EXPECT_THAT(value, Eq("W"));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, ReadOrientation2) {
  lua::Push(L, "N");
  lua::Push(L, "E");
  lua::Push(L, "S");
  lua::Push(L, "W");
  Orientation2d value = Orientation2d::kWest;
  EXPECT_TRUE(IsFound(Read(L, 1, &value)));
  EXPECT_THAT(value, Eq(Orientation2d::kNorth));
  EXPECT_TRUE(IsFound(Read(L, 2, &value)));
  EXPECT_THAT(value, Eq(Orientation2d::kEast));
  EXPECT_TRUE(IsFound(Read(L, 3, &value)));
  EXPECT_THAT(value, Eq(Orientation2d::kSouth));
  EXPECT_TRUE(IsFound(Read(L, 4, &value)));
  EXPECT_THAT(value, Eq(Orientation2d::kWest));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, ReadOrientation2Fail) {
  lua::Push(L, "Nothing");
  lua::Push(L, "Q");
  lua_pushnil(L);
  Orientation2d value;
  EXPECT_TRUE(IsTypeMismatch(Read(L, 1, &value)));
  EXPECT_TRUE(IsTypeMismatch(Read(L, 2, &value)));
  EXPECT_TRUE(IsNotFound(Read(L, 3, &value)));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, PushVector2) {
  Push(L, Vector2d::North());
  Push(L, Vector2d::East());
  Push(L, Vector2d::South());
  Push(L, Vector2d::West());
  std::array<int, 2> value;
  EXPECT_TRUE(IsFound(lua::Read(L, 1, &value)));
  EXPECT_THAT(value, Eq(std::array<int, 2>{0, -1}));
  EXPECT_TRUE(IsFound(lua::Read(L, 2, &value)));
  EXPECT_THAT(value, Eq(std::array<int, 2>{1, 0}));
  EXPECT_TRUE(IsFound(lua::Read(L, 3, &value)));
  EXPECT_THAT(value, Eq(std::array<int, 2>{0, 1}));
  EXPECT_TRUE(IsFound(lua::Read(L, 4, &value)));
  EXPECT_THAT(value, Eq(std::array<int, 2>{-1, 0}));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, ReadVector2) {
  lua::Push(L, std::array<int, 2>{0, -1});
  lua::Push(L, std::array<int, 2>{1, 0});
  lua::Push(L, std::array<int, 2>{0, 1});
  lua::Push(L, std::array<int, 2>{-1, 0});
  Vector2d value = Vector2d::West();
  EXPECT_TRUE(IsFound(Read(L, 1, &value)));
  EXPECT_THAT(value, Eq(Vector2d::North()));
  EXPECT_TRUE(IsFound(Read(L, 2, &value)));
  EXPECT_THAT(value, Eq(Vector2d::East()));
  EXPECT_TRUE(IsFound(Read(L, 3, &value)));
  EXPECT_THAT(value, Eq(Vector2d::South()));
  EXPECT_TRUE(IsFound(Read(L, 4, &value)));
  EXPECT_THAT(value, Eq(Vector2d::West()));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, ReadVector2Fail) {
  lua::Push(L, std::array<int, 1>{1});
  lua::Push(L, "Q");
  lua_pushnil(L);
  Vector2d value;
  EXPECT_TRUE(IsTypeMismatch(Read(L, 1, &value)));
  EXPECT_TRUE(IsTypeMismatch(Read(L, 2, &value)));
  EXPECT_TRUE(IsNotFound(Read(L, 3, &value)));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, PushPosition2) {
  Push(L, Position2d{0, -1});
  Push(L, Position2d{1, 0});
  Push(L, Position2d{0, 1});
  Push(L, Position2d{-1, 0});
  std::array<int, 2> value;
  EXPECT_TRUE(IsFound(lua::Read(L, 1, &value)));
  EXPECT_THAT(value, Eq(std::array<int, 2>{0, -1}));
  EXPECT_TRUE(IsFound(lua::Read(L, 2, &value)));
  EXPECT_THAT(value, Eq(std::array<int, 2>{1, 0}));
  EXPECT_TRUE(IsFound(lua::Read(L, 3, &value)));
  EXPECT_THAT(value, Eq(std::array<int, 2>{0, 1}));
  EXPECT_TRUE(IsFound(lua::Read(L, 4, &value)));
  EXPECT_THAT(value, Eq(std::array<int, 2>{-1, 0}));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, ReadPosition2) {
  lua::Push(L, std::array<int, 2>{0, -1});
  lua::Push(L, std::array<int, 2>{1, 0});
  lua::Push(L, std::array<int, 2>{0, 1});
  lua::Push(L, std::array<int, 2>{-1, 0});
  Position2d value = Position2d{-1, 0};
  EXPECT_TRUE(IsFound(Read(L, 1, &value)));
  EXPECT_THAT(value, Eq(Position2d{0, -1}));
  EXPECT_TRUE(IsFound(Read(L, 2, &value)));
  EXPECT_THAT(value, Eq(Position2d{1, 0}));
  EXPECT_TRUE(IsFound(Read(L, 3, &value)));
  EXPECT_THAT(value, Eq(Position2d{0, 1}));
  EXPECT_TRUE(IsFound(Read(L, 4, &value)));
  EXPECT_THAT(value, Eq(Position2d{-1, 0}));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, ReadPosition2Fail) {
  lua::Push(L, std::array<int, 1>{1});
  lua::Push(L, "Q");
  lua_pushnil(L);
  Position2d value;
  EXPECT_TRUE(IsTypeMismatch(Read(L, 1, &value)));
  EXPECT_TRUE(IsTypeMismatch(Read(L, 2, &value)));
  EXPECT_TRUE(IsNotFound(Read(L, 3, &value)));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, PushTransform2) {
  Push(L, Transform2d{{10, -1}, Orientation2d::kEast});
  lua::TableRef table;
  Transform2d value;
  EXPECT_TRUE(IsFound(lua::Read(L, 1, &table)));
  EXPECT_TRUE(IsFound(table.LookUp("pos", &value.position)));
  EXPECT_TRUE(IsFound(table.LookUp("orientation", &value.orientation)));
  EXPECT_THAT(value, Eq(Transform2d{{10, -1}, Orientation2d::kEast}));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, ReadTransform2) {
  {
    auto table = lua::TableRef::Create(L);
    table.Insert("pos", Position2d{-1, 10});
    table.Insert("orientation", "W");
    Push(L, table);
  }

  Transform2d value = {{0, 0}, Orientation2d::kNorth};
  EXPECT_TRUE(IsFound(Read(L, 1, &value)));
  EXPECT_THAT(value, Eq(Transform2d{{-1, 10}, Orientation2d::kWest}));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, ReadTransform2Fail) {
  lua::Push(L, std::array<int, 1>{1});
  lua::Push(L, "Q");
  {
    auto table = lua::TableRef::Create(L);
    table.Insert("pos", Position2d{-1, 10});
    Push(L, table);
  }
  {
    auto table = lua::TableRef::Create(L);
    table.Insert("orientation", "W");
    Push(L, table);
  }
  lua_pushnil(L);
  Transform2d value;
  EXPECT_TRUE(IsTypeMismatch(Read(L, 1, &value)));
  EXPECT_TRUE(IsTypeMismatch(Read(L, 2, &value)));
  EXPECT_TRUE(IsTypeMismatch(Read(L, 3, &value)));
  EXPECT_TRUE(IsTypeMismatch(Read(L, 4, &value)));
  EXPECT_TRUE(IsNotFound(Read(L, 5, &value)));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, PushSize2) {
  Push(L, Size2d{10, 1});
  lua::TableRef table;
  Size2d value;
  EXPECT_TRUE(IsFound(lua::Read(L, 1, &table)));
  EXPECT_TRUE(IsFound(table.LookUp("width", &value.width)));
  EXPECT_TRUE(IsFound(table.LookUp("height", &value.height)));
  EXPECT_THAT(value, Eq(Size2d{10, 1}));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, ReadSize2) {
  {
    auto table = lua::TableRef::Create(L);
    table.Insert("width", 1);
    table.Insert("height", 10);
    Push(L, table);
  }

  Size2d value = {0, 0};
  EXPECT_TRUE(IsFound(Read(L, 1, &value)));
  EXPECT_THAT(value, Eq(Size2d{1, 10}));
  lua_settop(L, 0);
}

TEST_F(LuaMath2dTest, ReadSize2Fail) {
  lua::Push(L, std::array<int, 1>{1});
  lua::Push(L, "Q");
  {
    auto table = lua::TableRef::Create(L);
    table.Insert("width", 10);
    Push(L, table);
  }
  {
    auto table = lua::TableRef::Create(L);
    table.Insert("height", 10);
    Push(L, table);
  }
  lua_pushnil(L);
  Transform2d value;
  EXPECT_TRUE(IsTypeMismatch(Read(L, 1, &value)));
  EXPECT_TRUE(IsTypeMismatch(Read(L, 2, &value)));
  EXPECT_TRUE(IsTypeMismatch(Read(L, 3, &value)));
  EXPECT_TRUE(IsTypeMismatch(Read(L, 4, &value)));
  EXPECT_TRUE(IsNotFound(Read(L, 5, &value)));
  lua_settop(L, 0);
}

}  // namespace
}  // namespace deepmind::lab2d::math
