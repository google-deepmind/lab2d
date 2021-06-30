// Copyright (C) 2016-2019 The DMLab2D Authors.
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

#include "dmlab2d/lib/system/tensor/lua/tensor.h"

#include "dmlab2d/lib/lua/bind.h"
#include "dmlab2d/lib/lua/table_ref.h"
#include "dmlab2d/lib/support/logging.h"

namespace deepmind::lab2d {
namespace tensor {

int LuaTensorConstructors(lua_State* L) {
  lua::TableRef table = lua::TableRef::Create(L);
  void* fs = nullptr;
  CHECK(!IsTypeMismatch(lua::Read(L, lua_upvalueindex(1), &fs)))
      << "Invalid filesystem pointer.";
  auto table_insert = [L, &table, fs](const char* name, lua_CFunction fn) {
    lua_pushlightuserdata(L, fs);
    lua_pushcclosure(L, fn, 1);
    table.InsertFromStackTop(name);
  };

  table_insert("ByteTensor", &lua::Bind<LuaTensor<std::uint8_t>::Create>);
  table_insert("CharTensor", &lua::Bind<LuaTensor<std::int8_t>::Create>);
  table_insert("Int16Tensor", &lua::Bind<LuaTensor<std::int16_t>::Create>);
  table_insert("Int32Tensor", &lua::Bind<LuaTensor<std::int32_t>::Create>);
  table_insert("Int64Tensor", &lua::Bind<LuaTensor<std::int64_t>::Create>);
  table_insert("FloatTensor", &lua::Bind<LuaTensor<float>::Create>);
  table_insert("DoubleTensor", &lua::Bind<LuaTensor<double>::Create>);
  table_insert("Tensor", &lua::Bind<LuaTensor<double>::Create>);
  lua::Push(L, table);
  return 1;
}

void LuaTensorRegister(lua_State* L) {
  LuaTensor<std::uint8_t>::Register(L);
  LuaTensor<std::int8_t>::Register(L);
  LuaTensor<std::int16_t>::Register(L);
  LuaTensor<std::int32_t>::Register(L);
  LuaTensor<std::int64_t>::Register(L);
  LuaTensor<float>::Register(L);
  LuaTensor<double>::Register(L);
}

}  // namespace tensor
}  // namespace deepmind::lab2d
