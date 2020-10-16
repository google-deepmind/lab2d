// Copyright (C) 2020 The DMLab2D Authors.
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

#include "dmlab2d/system/generators/pushbox/lua/pushbox.h"

#include "dmlab2d/lua/bind.h"
#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/n_results_or.h"
#include "dmlab2d/lua/push.h"
#include "dmlab2d/lua/read.h"
#include "dmlab2d/lua/table_ref.h"
#include "dmlab2d/system/generators/pushbox/pushbox.h"

namespace deepmind::lab2d {
namespace {

lua::NResultsOr Generate(lua_State* L) {
  pushbox::Settings settings;
  std::uint32_t room_seed;
  std::uint32_t targets_seed;
  std::uint32_t actions_seed;

  lua::TableRef table;
  if (!IsFound(Read(L, 1, &table))) return "Missing kwags";

  if (!IsFound(table.LookUp("seed", &settings.seed))) {
    return "Missing kwarg: 'seed'";
  }
  if (!IsFound(table.LookUp("width", &settings.width))) {
    return "Missing kwarg: 'width'";
  }
  if (!IsFound(table.LookUp("height", &settings.height))) {
    return "Missing kwarg: 'height'";
  }
  if (!IsFound(table.LookUp("numBoxes", &settings.num_boxes))) {
    return "Missing kwarg: 'numBoxes'";
  }
  if (IsTypeMismatch(table.LookUp("roomSteps", &settings.room_steps))) {
    return "kwarg: 'roomSteps' must be an int.";
  }
  if (IsFound(table.LookUp("roomSeed", &room_seed))) {
    settings.room_seed = room_seed;
  }
  if (IsFound(table.LookUp("targetsSeed", &targets_seed))) {
    settings.targets_seed = targets_seed;
  }
  if (IsFound(table.LookUp("actionsSeed", &actions_seed))) {
    settings.actions_seed = actions_seed;
  }

  const auto& [level, err] = pushbox::GenerateLevel(settings);
  if (!err.empty()) {
    return err;
  }
  lua::Push(L, level);
  return 1;
}

}  // namespace

int LuaPushboxRequire(lua_State* L) {
  auto table = lua::TableRef::Create(L);
  table.Insert("generate", &lua::Bind<Generate>);
  lua::Push(L, table);
  return 1;
}

}  // namespace deepmind::lab2d
