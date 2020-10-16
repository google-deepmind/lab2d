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

#include "dmlab2d/system/tile/lua/tile.h"

#include "dmlab2d/lua/bind.h"
#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/table_ref.h"
#include "dmlab2d/system/tile/lua/tile_scene.h"
#include "dmlab2d/system/tile/lua/tile_set.h"

namespace deepmind::lab2d {

int LuaTileModule(lua_State* L) {
  LuaTileSet::Register(L);
  LuaTileScene::Register(L);

  lua::TableRef table = lua::TableRef::Create(L);
  table.Insert("Set", &lua::Bind<LuaTileSet::Create>);
  table.Insert("set", &lua::Bind<LuaTileSet::Create>);  // Deprecated.
  table.Insert("Scene", &lua::Bind<LuaTileScene::Create>);
  table.Insert("scene", &lua::Bind<LuaTileScene::Create>);  // Deprecated.
  lua::Push(L, table);
  return 1;
}

}  // namespace deepmind::lab2d
