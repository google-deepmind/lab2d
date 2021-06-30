// Copyright (C) 2018-2019 The DMLab2D Authors.
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

#include "dmlab2d/lib/env_lua_api/events.h"

#include <utility>

#include "dmlab2d/lib/lua/class.h"
#include "dmlab2d/lib/lua/lua.h"
#include "dmlab2d/lib/lua/read.h"
#include "dmlab2d/lib/support/logging.h"
#include "dmlab2d/lib/system/tensor/lua/tensor.h"
#include "dmlab2d/lib/system/tensor/tensor_view.h"

namespace deepmind::lab2d {
namespace {

class LuaEventsModule : public lua::Class<LuaEventsModule> {
  friend class Class;
  static const char* ClassName() { return "deepmind.lab.Events"; }

 public:
  // '*ctx' owned by the caller and should out-live this object.
  explicit LuaEventsModule(Events* ctx) : ctx_(ctx) {}

  // Registers classes metatable with Lua.
  static void Register(lua_State* L) {
    const Class::Reg methods[] = {{"add", Member<&LuaEventsModule::Add>}};
    Class::Register(L, methods);
  }

 private:
  template <typename T>
  void AddTensorObservation(int id, const tensor::TensorView<T>& view) {
    const auto& shape = view.shape();
    std::vector<int> out_shape(shape.begin(), shape.end());

    std::vector<T> out_values;
    out_values.reserve(view.num_elements());
    view.ForEach([&out_values](T v) { out_values.push_back(v); });
    ctx_->AddObservation(id, std::move(out_shape), std::move(out_values));
  }

  // Signature events:add(eventName, [obs1, [obs2 ...] ...])
  // Called with an event name and a list of observations. Each observation
  // maybe one of string, ByteTensor, Int32Tensor, Int64Tensor or DoubleTensor.
  // [-(2 + #observations), 0, e]
  lua::NResultsOr Add(lua_State* L) {
    int top = lua_gettop(L);
    std::string name;
    if (!lua::Read(L, 2, &name)) {
      return "Event name must be a string";
    }
    int id = ctx_->Add(std::move(name));
    for (int i = 3; i <= top; ++i) {
      std::string string_arg;
      if (lua::Read(L, i, &string_arg)) {
        ctx_->AddObservation(id, std::move(string_arg));
      } else if (auto* double_tensor =
                     tensor::LuaTensor<double>::ReadObject(L, i)) {
        AddTensorObservation(id, double_tensor->tensor_view());
      } else if (auto* byte_tensor =
                     tensor::LuaTensor<unsigned char>::ReadObject(L, i)) {
        AddTensorObservation(id, byte_tensor->tensor_view());
      } else if (auto* int32_tensor =
                     tensor::LuaTensor<int32_t>::ReadObject(L, i)) {
        AddTensorObservation(id, int32_tensor->tensor_view());
      } else if (auto* int64_tensor =
                     tensor::LuaTensor<int64_t>::ReadObject(L, i)) {
        AddTensorObservation(id, int64_tensor->tensor_view());
      } else if (lua_isnumber(L, i)) {
        ctx_->AddObservation(id, {}, std::vector<double>{lua_tonumber(L, i)});
      } else {
        return "[event] - Observation type not supported. Must be one of "
               "string|ByteTensor|DoubleTensor.";
      }
    }
    return 0;
  }

  Events* ctx_;
};

}  // namespace

lua::NResultsOr Events::Module(lua_State* L) {
  if (auto* ctx =
          static_cast<Events*>(lua_touserdata(L, lua_upvalueindex(1)))) {
    LuaEventsModule::Register(L);
    LuaEventsModule::CreateObject(L, ctx);
    return 1;
  } else {
    return "Missing Event context!";
  }
}

int Events::Add(std::string name) {
  auto iter_inserted = name_to_id_.emplace(std::move(name), names_.size());
  if (iter_inserted.second) {
    names_.push_back(iter_inserted.first->first.c_str());
  }

  int id = events_.size();
  events_.push_back(Event{iter_inserted.first->second});
  return id;
}

void Events::AddObservation(int event_id, std::string string_value) {
  Event& event = events_[event_id];
  event.observations.emplace_back();
  auto& observation = event.observations.back();
  observation.type = EnvCApi_ObservationString;

  observation.shape_id = shapes_.size();
  std::vector<int> shape(1);
  shape[0] = string_value.size();
  shapes_.emplace_back(std::move(shape));

  observation.array_id = strings_.size();
  strings_.push_back(std::move(string_value));
}

void Events::AddObservation(int event_id, std::vector<int> shape,
                            std::vector<double> double_tensor) {
  Event& event = events_[event_id];
  event.observations.emplace_back();
  auto& observation = event.observations.back();
  observation.type = EnvCApi_ObservationDoubles;

  observation.shape_id = shapes_.size();
  shapes_.push_back(std::move(shape));

  observation.array_id = doubles_.size();
  doubles_.push_back(std::move(double_tensor));
}

void Events::AddObservation(int event_id, std::vector<int> shape,
                            std::vector<unsigned char> byte_tensor) {
  Event& event = events_[event_id];
  event.observations.emplace_back();
  auto& observation = event.observations.back();
  observation.type = EnvCApi_ObservationBytes;

  observation.shape_id = shapes_.size();
  shapes_.push_back(std::move(shape));

  observation.array_id = bytes_.size();
  bytes_.push_back(std::move(byte_tensor));
}

void Events::AddObservation(int event_id, std::vector<int> shape,
                            std::vector<std::int32_t> int32_tensor) {
  Event& event = events_[event_id];
  event.observations.emplace_back();
  auto& observation = event.observations.back();
  observation.type = EnvCApi_ObservationInt32s;

  observation.shape_id = shapes_.size();
  shapes_.push_back(std::move(shape));

  observation.array_id = int32s_.size();
  int32s_.push_back(std::move(int32_tensor));
}

void Events::AddObservation(int event_id, std::vector<int> shape,
                            std::vector<std::int64_t> int64_tensor) {
  Event& event = events_[event_id];
  event.observations.emplace_back();
  auto& observation = event.observations.back();
  observation.type = EnvCApi_ObservationInt64s;

  observation.shape_id = shapes_.size();
  shapes_.push_back(std::move(shape));

  observation.array_id = int64s_.size();
  int64s_.push_back(std::move(int64_tensor));
}

void Events::Clear() {
  events_.clear();
  strings_.clear();
  shapes_.clear();
  doubles_.clear();
  bytes_.clear();
  int32s_.clear();
  int64s_.clear();
}

void Events::Export(int event_idx, EnvCApi_Event* event) {
  const auto& internal_event = events_[event_idx];
  observations_.clear();
  observations_.reserve(internal_event.observations.size());
  for (const auto& observation : internal_event.observations) {
    observations_.emplace_back();
    auto& observation_out = observations_.back();
    observation_out.spec.type = observation.type;

    const auto& shape = shapes_[observation.shape_id];
    observation_out.spec.dims = shape.size();
    observation_out.spec.shape = shape.data();

    switch (observation.type) {
      case EnvCApi_ObservationBytes: {
        const auto& tensor = bytes_[observation.array_id];
        observation_out.payload.bytes = tensor.data();
        break;
      }
      case EnvCApi_ObservationDoubles: {
        const auto& tensor = doubles_[observation.array_id];
        observation_out.payload.doubles = tensor.data();
        break;
      }
      case EnvCApi_ObservationString: {
        const auto& string_value = strings_[observation.array_id];
        observation_out.payload.string = string_value.c_str();
        break;
      }
      case EnvCApi_ObservationInt32s: {
        const auto& tensor = int32s_[observation.array_id];
        observation_out.payload.int32s = tensor.data();
        break;
      }
      case EnvCApi_ObservationInt64s: {
        const auto& tensor = int64s_[observation.array_id];
        observation_out.payload.int64s = tensor.data();
        break;
      }
      default:
        LOG(FATAL) << "Observation type: " << observation.type
                   << " not supported";
    }
  }
  event->id = internal_event.type_id;
  event->observations = observations_.data();
  event->observation_count = observations_.size();
}

}  // namespace deepmind::lab2d
