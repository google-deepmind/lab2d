// Copyright (C) 2017-2019 The DMLab2D Authors.
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

#include "dmlab2d/lib/env_lua_api/observations.h"

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iostream>
#include <utility>

#include "absl/log/check.h"
#include "absl/log/log.h"
#include "absl/strings/string_view.h"
#include "dmlab2d/lib/lua/call.h"
#include "dmlab2d/lib/lua/class.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "dmlab2d/lib/lua/push.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/system/tensor/lua/tensor.h"
#include "dmlab2d/lib/system/tensor/tensor_view.h"

namespace deepmind::lab2d {

lua::NResultsOr Observations::BindApi(lua::TableRef script_table_ref) {
  script_table_ref_ = std::move(script_table_ref);
  script_table_ref_.PushMemberFunction("observationSpec");
  lua_State* L = script_table_ref_.LuaState();
  if (lua_isnil(L, -2)) {
    lua_pop(L, 2);
    return 0;
  }
  auto result = lua::Call(L, 1);
  if (!result.ok()) {
    return result;
  }
  lua::TableRef observations;
  if (!IsFound(lua::Read(L, -1, &observations))) {
    return "[observationSpec] - Must be a table.";
  }
  auto spec_count = observations.ArraySize();
  infos_.clear();
  infos_.reserve(spec_count);
  for (std::size_t i = 0, c = spec_count; i != c; ++i) {
    lua::TableRef info;
    if (!IsFound(observations.LookUp(i + 1, &info))) {
      return "[observationSpec] - Missing table.\n";
    }
    SpecInfo spec_info;
    if (!IsFound(info.LookUp("name", &spec_info.name))) {
      return "[observationSpec] - Missing 'name = <string>'.\n";
    }
    absl::string_view type;
    if (!IsFound(info.LookUp("type", &type))) {
      return "[observationSpec] - Missing 'type = <string>'.\n";
    }
    if (type == "Doubles" || type == "tensor.DoubleTensor") {
      spec_info.type = EnvCApi_ObservationDoubles;
    } else if (type == "Bytes" || type == "tensor.ByteTensor") {
      spec_info.type = EnvCApi_ObservationBytes;
    } else if (type == "String") {
      spec_info.type = EnvCApi_ObservationString;
    } else if (type == "Int32s" || type == "tensor.Int32Tensor") {
      spec_info.type = EnvCApi_ObservationInt32s;
    } else if (type == "Int64s" || type == "tensor.Int64Tensor") {
      spec_info.type = EnvCApi_ObservationInt64s;
    } else {
      return "[observationSpec] - 'type = "
             "'Bytes'|'Doubles'|'String'|'Int32s'|'Int64s''.\n";
    }
    if (spec_info.type != EnvCApi_ObservationString &&
        !IsFound(info.LookUp("shape", &spec_info.shape))) {
      return "[observationSpec] - Missing 'shape = {<int>, ...}'.\n";
    }
    infos_.push_back(std::move(spec_info));
  }
  lua_pop(L, result.n_results());
  return 0;
}

void Observations::Observation(int idx, EnvCApi_Observation* observation) {
  lua_State* L = script_table_ref_.LuaState();
  script_table_ref_.PushMemberFunction("observation");
  // Function must exist.
  CHECK(!lua_isnil(L, -2))
      << "Observations Spec set but no observation member function";
  const auto& info = infos_[idx];
  lua::Push(L, idx + 1);
  auto result = lua::Call(L, 2);
  CHECK(result.ok()) << "[observation] - " << result.error();

  const tensor::Layout* layout = nullptr;
  observation->spec.type = info.type;
  switch (info.type) {
    case EnvCApi_ObservationDoubles: {
      constexpr char error_message[] =
          "[observation] - Must return a contiguous DoubleTensor or number "
          "while reading: '";
      if (lua_isnumber(L, -1)) {
        CHECK(IsFound(lua::Read(L, -1, &double_)))
            << error_message << info.name << "'";
        tensor_shape_.clear();
        observation->payload.doubles = &double_;
        observation->spec.dims = 0;
        observation->spec.shape = nullptr;
      } else {
        CHECK_EQ(1, result.n_results()) << error_message << info.name << "'";
        auto* double_tensor = tensor::LuaTensor<double>::ReadObject(L, -1);
        CHECK(double_tensor != nullptr) << error_message << info.name << "'";
        const auto& view = double_tensor->tensor_view();
        CHECK(view.IsContiguous()) << error_message << info.name << "'";
        layout = &view;
        observation->payload.doubles = view.storage() + view.start_offset();
      }
      break;
    }
    case EnvCApi_ObservationBytes: {
      constexpr char error_message[] =
          "[observation] - Must return a contiguous ByteTensor while reading: "
          "'";
      CHECK_EQ(1, result.n_results()) << error_message << info.name << "'";
      auto* byte_tensor = tensor::LuaTensor<std::uint8_t>::ReadObject(L, -1);
      CHECK(byte_tensor != nullptr) << error_message << info.name << "'";
      const auto& view = byte_tensor->tensor_view();
      layout = &view;
      CHECK(view.IsContiguous()) << error_message << info.name << "'";
      observation->payload.bytes = view.storage() + view.start_offset();
      break;
    }
    case EnvCApi_ObservationString: {
      constexpr char error_message[] =
          "[observation] - Must return a string while reading: '";
      CHECK_EQ(1, result.n_results()) << error_message << info.name << "'";
      CHECK(lua::Read(L, -1, &string_)) << error_message << info.name << "'";
      observation->payload.string = string_.data();
      tensor_shape_.assign(1, string_.length());
      observation->spec.dims = tensor_shape_.size();
      observation->spec.shape = tensor_shape_.data();
      break;
    }
    case EnvCApi_ObservationInt32s: {
      constexpr char error_message[] =
          "[observation] - Must return a contiguous Int32Tensor or number "
          "while reading: '";
      if (lua_isnumber(L, -1)) {
        CHECK(IsFound(lua::Read(L, -1, &int32_)))
            << error_message << info.name << "'";
        tensor_shape_.clear();
        observation->payload.int32s = &int32_;
        observation->spec.dims = 0;
        observation->spec.shape = nullptr;
      } else {
        CHECK_EQ(1, result.n_results()) << error_message << info.name << "'";
        auto* int32_tensor = tensor::LuaTensor<std::int32_t>::ReadObject(L, -1);
        CHECK(int32_tensor != nullptr) << error_message << info.name << "'";
        const auto& view = int32_tensor->tensor_view();
        layout = &view;
        CHECK(view.IsContiguous()) << error_message << info.name << "'";
        observation->payload.int32s = view.storage() + view.start_offset();
      }
      break;
    }
    case EnvCApi_ObservationInt64s: {
      constexpr char error_message[] =
          "[observation] - Must return a contiguous Int64Tensor or number "
          "while reading: '";
      if (lua_isnumber(L, -1)) {
        CHECK(IsFound(lua::Read(L, -1, &int64_)))
            << error_message << info.name << "'";
        tensor_shape_.clear();
        observation->payload.int64s = &int64_;
        observation->spec.dims = 0;
        observation->spec.shape = nullptr;
      } else {
        CHECK_EQ(1, result.n_results()) << error_message << info.name << "'";
        auto* int64_tensor = tensor::LuaTensor<std::int64_t>::ReadObject(L, -1);
        CHECK(int64_tensor != nullptr) << error_message << info.name << "'";
        const auto& view = int64_tensor->tensor_view();
        layout = &view;
        CHECK(view.IsContiguous()) << error_message << info.name << "'";
        observation->payload.int64s = view.storage() + view.start_offset();
      }
      break;
    }
    default:
      LOG(FATAL) << "Observation type: " << info.type << " not supported";
  }

  if (layout != nullptr) {
    tensor_shape_.resize(layout->shape().size());
    std::copy(layout->shape().begin(), layout->shape().end(),
              tensor_shape_.begin());
    observation->spec.dims = tensor_shape_.size();
    observation->spec.shape = tensor_shape_.data();
    // Prevent observation->payload from being destroyed during pop.
    CHECK(IsFound(lua::Read(L, -1, &tensor_))) << "Internal logic error!";
  }
  lua_pop(L, result.n_results());
}

void Observations::Spec(int idx, EnvCApi_ObservationSpec* spec) const {
  const auto& info = infos_[idx];
  spec->type = info.type;
  spec->dims = info.shape.size();
  spec->shape = info.shape.data();
}

}  // namespace deepmind::lab2d
