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

#include <algorithm>
#include <cstdint>
#include <exception>
#include <map>
#include <memory>
#include <stdexcept>
#include <string>
#include <type_traits>
#include <vector>

#include "absl/container/flat_hash_map.h"
#include "absl/strings/str_cat.h"
#include "absl/types/span.h"
#include "dmlab2d/lib/dmlab2d.h"
#include "dmlab2d/lib/support/logging.h"
#include "include/pybind11/cast.h"
#include "include/pybind11/detail/common.h"
#include "include/pybind11/numpy.h"
#include "include/pybind11/pybind11.h"
#include "include/pybind11/pytypes.h"
#include "include/pybind11/stl.h"
#include "third_party/rl_api/env_c_api.h"

namespace {
namespace py = pybind11;

py::object FromArrayObservation(const EnvCApi_Observation& obs) {
  switch (obs.spec.type) {
    case EnvCApi_ObservationBytes: {
      return py::array_t<std::uint8_t>(
          absl::MakeConstSpan(obs.spec.shape, obs.spec.dims),
          obs.payload.bytes);
    }
    case EnvCApi_ObservationDoubles: {
      return py::array_t<double>(
          absl::MakeConstSpan(obs.spec.shape, obs.spec.dims),
          obs.payload.doubles);
    }
    case EnvCApi_ObservationInt32s: {
      return py::array_t<std::int32_t>(
          absl::MakeConstSpan(obs.spec.shape, obs.spec.dims),
          obs.payload.int32s);
    }
    case EnvCApi_ObservationInt64s: {
      return py::array_t<std::int64_t>(
          absl::MakeConstSpan(obs.spec.shape, obs.spec.dims),
          obs.payload.int64s);
    }
    case EnvCApi_ObservationString:
      return py::bytes(obs.payload.string, obs.spec.shape[0]);
  }
  throw std::invalid_argument(
      absl::StrCat("Unhandled observation type: ", obs.spec.type));
}

py::dict FromArrayObservationSpec(const EnvCApi_ObservationSpec& spec) {
  py::object shape;
  if (spec.dims >= 0) {
    py::tuple tuple(spec.dims);
    for (int i = 0; i < spec.dims; ++i) {
      tuple[i] = py::cast(spec.shape[i]);
    }
    shape = tuple;
  }
  switch (spec.type) {
    case EnvCApi_ObservationBytes: {
      return py::dict(py::arg("dtype") = py::dtype::of<std::uint8_t>(),
                      py::arg("shape") = shape);
    }
    case EnvCApi_ObservationDoubles: {
      return py::dict(py::arg("dtype") = py::dtype::of<double>(),
                      py::arg("shape") = shape);
    }
    case EnvCApi_ObservationInt32s: {
      return py::dict(py::arg("dtype") = py::dtype::of<int32_t>(),
                      py::arg("shape") = shape);
    }
    case EnvCApi_ObservationInt64s: {
      return py::dict(py::arg("dtype") = py::dtype::of<int64_t>(),
                      py::arg("shape") = shape);
    }
    case EnvCApi_ObservationString: {
      return py::dict(py::arg("dtype") = py::dtype("object"),
                      py::arg("shape") = py::tuple());
    }
  }
  throw std::invalid_argument(
      absl::StrCat("Unhandled observation type: ", spec.type));
}

class PyEnvCApi {
 public:
  template <typename Connect>
  static PyEnvCApi Create(Connect connect_func,
                          const std::map<std::string, std::string>& settings) {
    EnvCApi env_c_api;
    void* context = nullptr;
    connect_func(&env_c_api, &context);
    auto env = absl::make_unique<Env>(env_c_api, context);
    for (const auto& [key, value] : settings) {
      if (env->api.setting(env->ctx, key.c_str(), value.c_str()) != 0) {
        throw py::key_error(absl::StrCat("\"", key, "\"=\"", value, " - ",
                                         env->api.error_message(env->ctx)));
      }
    }

    if (env->api.init(env->ctx) != 0) {
      throw std::invalid_argument(env->api.error_message(env->ctx));
    }

    return PyEnvCApi(std::move(env));
  }

  const std::string Name() const {
    return env_->api.environment_name(env_->ctx);
  }

  const std::vector<std::string>& ObservationNames() const {
    return observation_names_;
  }

  const std::vector<std::string>& ActionDiscreteNames() const {
    return action_discrete_names_;
  }

  py::dict ActionDiscreteSpec(const std::string& action_name) {
    if (auto it = action_discrete_map_.find(action_name);
        it != action_discrete_map_.end()) {
      py::dict result;
      result["min"] = action_discrete_min_values_[it->second];
      result["max"] = action_discrete_max_values_[it->second];
      return result;
    } else {
      throw py::key_error(action_name);
    }
  }

  const std::vector<std::string>& ActionContinuousNames() const {
    return action_continuous_names_;
  }

  py::dict ActionContinuousSpec(const std::string& action_name) {
    if (auto it = action_continuous_map_.find(action_name);
        it != action_continuous_map_.end()) {
      py::dict result;
      result["min"] = action_continuous_min_values_[it->second];
      result["max"] = action_continuous_max_values_[it->second];
      return result;
    } else {
      throw py::key_error(action_name);
    }
  }

  const std::vector<std::string>& ActionTextNames() const {
    return action_text_names_;
  }

  void Start(int episode, int seed) {
    if (env_->api.start(env_->ctx, episode, seed) != 0) {
      throw std::invalid_argument(absl::StrCat(
          "Failed to start: ", env_->api.error_message(env_->ctx)));
    }
    state_ = State::kStep;
  }

  py::object Observation(const std::string& observation_name) {
    if (state_ == State::kPreStart) {
      throw std::runtime_error("Environment not started!");
    }
    if (auto it = observation_map_.find(observation_name);
        it != observation_map_.end()) {
      EnvCApi_Observation observation;
      env_->api.observation(env_->ctx, it->second, &observation);
      return FromArrayObservation(observation);
    } else {
      throw py::key_error(observation_name);
    }
  }

  py::dict ObservationSpec(const std::string& observation_name) {
    if (auto it = observation_map_.find(observation_name);
        it != observation_map_.end()) {
      EnvCApi_ObservationSpec observation_spec;
      env_->api.observation_spec(env_->ctx, it->second, &observation_spec);
      return FromArrayObservationSpec(observation_spec);
    } else {
      throw py::key_error(observation_name);
    }
  }

  void ActDiscrete(const py::array_t<int, py::array::c_style |
                                              py::array::forcecast>& action) {
    if (state_ == State::kPreStart) {
      throw std::runtime_error("Environment not started!");
    }
    if (action.size() != action_discrete_names_.size()) {
      throw std::invalid_argument(
          absl::StrCat("Invalid action shape, expected int array with shape (",
                       action_discrete_names_.size(), ",)"));
    }
    env_->api.act_discrete(env_->ctx, action.data());
  }

  void ActContinuous(
      const py::array_t<double, py::array::c_style | py::array::forcecast>&
          action) {
    if (state_ == State::kPreStart) {
      throw std::runtime_error("Environment not started!");
    }
    if (action.size() != action_continuous_names_.size()) {
      throw std::invalid_argument(
          absl::StrCat("Invalid action shape, expected int array with shape (",
                       action_continuous_names_.size(), ",)"));
    }
    env_->api.act_continuous(env_->ctx, action.data());
  }

  void ActText(const std::vector<std::string>& text_actions) {
    if (state_ == State::kPreStart) {
      throw std::runtime_error("Environment not started!");
    }
    std::vector<EnvCApi_TextAction> actions;
    actions.reserve(text_actions.size());
    for (const auto& action : text_actions) {
      actions.push_back(EnvCApi_TextAction{action.data(), action.size()});
    }
    env_->api.act_text(env_->ctx, actions.data());
  }

  py::tuple Advance() {
    if (state_ == State::kPreStart) {
      throw std::runtime_error("Environment not started!");
    } else if (state_ == State::kEpisodeEnded) {
      throw std::runtime_error("Episode ended must call start first!");
    }
    double reward;
    EnvCApi_EnvironmentStatus status =
        env_->api.advance(env_->ctx, /*steps=*/1, &reward);
    if (status == EnvCApi_EnvironmentStatus_Error) {
      state_ = State::kEpisodeEnded;
      throw std::runtime_error(env_->api.error_message(env_->ctx));
    }
    state_ = status == EnvCApi_EnvironmentStatus_Running ? State::kStep
                                                         : State::kEpisodeEnded;
    py::tuple result(2);
    result[0] = status;
    result[1] = reward;
    return result;
  }

  py::list Events() {
    if (state_ == State::kPreStart) {
      throw std::runtime_error("Environment not started!");
    }
    int event_count = env_->api.event_count(env_->ctx);
    py::list events(event_count);
    for (int event_index = 0; event_index < event_count; ++event_index) {
      py::tuple name_observations(2);
      EnvCApi_Event event;
      env_->api.event(env_->ctx, event_index, &event);
      name_observations[0] = env_->api.event_type_name(env_->ctx, event.id);
      py::list observations(event.observation_count);
      for (int obs_index = 0; obs_index < event.observation_count;
           ++obs_index) {
        observations[obs_index] =
            FromArrayObservation(event.observations[obs_index]);
      }
      name_observations[1] = observations;
      events[event_index] = name_observations;
    }
    return events;
  }

  py::list ListProperty(const std::string& name) {
    py::list result;
    auto list_result = env_->api.list_property(
        env_->ctx, &result, name.c_str(),
        +[](void* userdata, const char* key,
            EnvCApi_PropertyAttributes attributes) {
          auto& user_list = *static_cast<py::list*>(userdata);
          py::tuple result(2);
          result[0] = py::cast(key);
          result[1] = attributes;
          user_list.append(result);
        });
    switch (list_result) {
      case EnvCApi_PropertyResult_Success:
        return result;
      case EnvCApi_PropertyResult_PermissionDenied:
        throw std::invalid_argument(
            absl::StrCat("Permission denied listing: '", name, "'"));
      case EnvCApi_PropertyResult_InvalidArgument:
        throw std::invalid_argument(
            absl::StrCat("Invalid argument listing: '", name, "'"));
      case EnvCApi_PropertyResult_NotFound:
        throw py::key_error(absl::StrCat(name));
    }
    throw std::invalid_argument(
        absl::StrCat("Error occured while listing: '", name, "'"));
  }

  std::string ReadProperty(const std::string& name) {
    const char* result;
    switch (env_->api.read_property(env_->ctx, name.c_str(), &result)) {
      case EnvCApi_PropertyResult_Success:
        return result;
      case EnvCApi_PropertyResult_PermissionDenied:
        throw std::invalid_argument(
            absl::StrCat("Permission denied reading: '", name, "'"));
      case EnvCApi_PropertyResult_InvalidArgument:
        throw std::invalid_argument(
            absl::StrCat("Invalid argument reading: '", name, "'"));
      case EnvCApi_PropertyResult_NotFound:
        throw py::key_error(absl::StrCat(name));
    }
    throw std::invalid_argument(
        absl::StrCat("Error occured while reading: '", name, "'"));
  }

  void WriteProperty(const std::string& name, const std::string& value) {
    switch (env_->api.write_property(env_->ctx, name.c_str(), value.c_str())) {
      case EnvCApi_PropertyResult_Success:
        return;
      case EnvCApi_PropertyResult_PermissionDenied:
        throw std::invalid_argument(
            absl::StrCat("Permission denied reading: '", name, "'"));
      case EnvCApi_PropertyResult_InvalidArgument:
        throw std::invalid_argument(
            absl::StrCat("Invalid argument reading: '", name, "'"));
      case EnvCApi_PropertyResult_NotFound:
        throw py::key_error(absl::StrCat(name));
    }
    throw std::invalid_argument(
        absl::StrCat("Error occured while reading: '", name, "'"));
  }

 private:
  struct Env {
    Env() = delete;
    Env(const Env&) = delete;
    Env& operator=(const Env&) = delete;
    Env(EnvCApi env_c_api, void* context) : api(env_c_api), ctx(context) {}
    ~Env() {
      if (ctx != nullptr) {
        api.release_context(ctx);
      }
    }
    EnvCApi api;
    void* ctx;
  };

  enum class State {
    kPreStart,
    kStep,
    kEpisodeEnded,
  };

  explicit PyEnvCApi(std::unique_ptr<Env> env_dd) : env_(std::move(env_dd)) {
    int observation_count = env_->api.observation_count(env_->ctx);
    observation_map_.reserve(observation_count);
    observation_names_.reserve(observation_count);
    for (int i = 0; i < observation_count; ++i) {
      std::string name = env_->api.observation_name(env_->ctx, i);
      observation_names_.push_back(name);
      observation_map_.emplace(std::move(name), i);
    }
    int action_discrete_count = env_->api.action_discrete_count(env_->ctx);
    action_discrete_names_.reserve(action_discrete_count);
    action_discrete_min_values_.reserve(action_discrete_count);
    action_discrete_max_values_.reserve(action_discrete_count);

    action_discrete_map_.reserve(action_discrete_count);
    for (int i = 0; i < action_discrete_count; ++i) {
      std::string name = env_->api.action_discrete_name(env_->ctx, i);
      action_discrete_names_.push_back(name);
      action_discrete_map_.emplace(std::move(name), i);
      env_->api.action_discrete_bounds(
          env_->ctx, i, &action_discrete_min_values_.emplace_back(),
          &action_discrete_max_values_.emplace_back());
    }
    int action_continuous_count = env_->api.action_continuous_count(env_->ctx);
    action_continuous_names_.reserve(action_continuous_count);
    action_continuous_min_values_.reserve(action_continuous_count);
    action_continuous_max_values_.reserve(action_continuous_count);
    action_continuous_map_.reserve(action_continuous_count);
    for (int i = 0; i < action_continuous_count; ++i) {
      std::string name = env_->api.action_continuous_name(env_->ctx, i);
      action_continuous_names_.push_back(name);
      action_continuous_map_.emplace(std::move(name), i);
      env_->api.action_continuous_bounds(
          env_->ctx, i, &action_continuous_min_values_.emplace_back(),
          &action_continuous_max_values_.emplace_back());
    }
    int action_text_count = env_->api.action_text_count(env_->ctx);
    action_text_map_.reserve(action_text_count);
    action_text_names_.reserve(action_text_count);
    for (int i = 0; i < action_text_count; ++i) {
      std::string name = env_->api.action_text_name(env_->ctx, i);
      action_text_names_.push_back(name);
      action_text_map_.emplace(std::move(name), i);
    }
  }

  std::unique_ptr<Env> env_;
  std::vector<std::string> observation_names_;
  absl::flat_hash_map<std::string, int> observation_map_;
  std::vector<std::string> action_discrete_names_;
  std::vector<int> action_discrete_min_values_;
  std::vector<int> action_discrete_max_values_;
  absl::flat_hash_map<std::string, int> action_discrete_map_;
  std::vector<std::string> action_continuous_names_;
  std::vector<double> action_continuous_min_values_;
  std::vector<double> action_continuous_max_values_;
  absl::flat_hash_map<std::string, int> action_continuous_map_;
  std::vector<std::string> action_text_names_;
  absl::flat_hash_map<std::string, int> action_text_map_;

  // Stores environment state to enforce preconditions.
  State state_ = State::kPreStart;
};

PYBIND11_MODULE(dmlab2d_pybind, m) {
  m.doc() = "DeepMind Lab2D";
  py::class_<PyEnvCApi>(m, "Lab2d")
      .def(py::init([](const std::string runfiles_root,
                       const std::map<std::string, std::string>& settings) {
             return PyEnvCApi::Create(
                 [&runfiles_root](EnvCApi* env_c_api, void** context) {
                   DeepMindLab2DLaunchParams params;
                   params.runfiles_root = runfiles_root.c_str();
                   if (dmlab2d_connect(&params, env_c_api, context) != 0) {
                     throw std::invalid_argument(params.runfiles_root);
                   }
                 },
                 settings);
           }),
           py::arg("runfiles_root"), py::arg("settings"))
      .def("name", &PyEnvCApi::Name, "Name of the environment.")
      .def("start", &PyEnvCApi::Start, py::arg("episode"), py::arg("seed"),
           "Launches an episode using 'episode_id' and 'seed'.")
      .def("observation_names", &PyEnvCApi::ObservationNames,
           "Returns all observations available.")
      .def("observation", &PyEnvCApi::Observation, py::arg("name"),
           "Returns observation. May not be called until a successful call to "
           "start.")
      .def("observation_spec", &PyEnvCApi::ObservationSpec, py::arg("name"),
           "Returns a dictionary containing the shape and dtype of an "
           "observations. Strings are specified as objects.")
      .def("action_discrete_names", &PyEnvCApi::ActionDiscreteNames,
           "Returns a list of discrete action names.")
      .def("action_discrete_spec", &PyEnvCApi::ActionDiscreteSpec,
           "Returns a dictionary specifying the 'min' and 'max' value for a "
           "given discrete action name.")
      .def("action_continuous_names", &PyEnvCApi::ActionContinuousNames,
           "Returns a list of continuous action names.")
      .def("action_continuous_spec", &PyEnvCApi::ActionContinuousSpec,
           "Returns a dictionary specifying the 'min' and 'max' value for a "
           "given continuous action name.")
      .def("action_text_names", &PyEnvCApi::ActionTextNames,
           "Returns a list of text action names.")
      .def("act_discrete", &PyEnvCApi::ActDiscrete, py::arg("action"),
           "Sets the discrete actions for the next call to advance.")
      .def("act_continuous", &PyEnvCApi::ActContinuous, py::arg("action"),
           "Sets the continuous actions for the next call to advance.")
      .def("act_text", &PyEnvCApi::ActText, py::arg("action"),
           "Sets the text actions for the next call to advance.")
      .def("advance", &PyEnvCApi::Advance,
           "Advances the environment by one frame")
      .def("events", &PyEnvCApi::Events,
           "Returns the events generated during start or last advance.")
      .def("list_property", &PyEnvCApi::ListProperty, py::arg("key"),
           "Returns a list the properties under specified hey name. Empty "
           "string is often used as the root.")
      .def("read_property", &PyEnvCApi::ReadProperty, py::arg("key"),
           "Returns the value of a given property converted to a string.")
      .def("write_property", &PyEnvCApi::WriteProperty, py::arg("key"),
           py::arg("value"),
           "Sets the value of a given property, converted from string.");

  py::enum_<EnvCApi_EnvironmentStatus>(m, "EnvironmentStatus")
      .value("RUNNING", EnvCApi_EnvironmentStatus_Running)
      .value("TERMINATED", EnvCApi_EnvironmentStatus_Terminated)
      .value("INTERRUPTED", EnvCApi_EnvironmentStatus_Interrupted)
      .export_values();

  py::enum_<EnvCApi_PropertyAttributes>(m, "PropertyAttribute",
                                        py::arithmetic())
      .value("NONE", EnvCApi_PropertyAttributes(0))
      .value("READABLE", EnvCApi_PropertyAttributes_Readable)
      .value("WRITABLE", EnvCApi_PropertyAttributes_Writable)
      .value("READABLE_WRITABLE",
             EnvCApi_PropertyAttributes(EnvCApi_PropertyAttributes_Readable |
                                        EnvCApi_PropertyAttributes_Writable))
      .value("LISTABLE", EnvCApi_PropertyAttributes_Listable)
      .value("READABLE_LISTABLE",
             EnvCApi_PropertyAttributes(EnvCApi_PropertyAttributes_Readable |
                                        EnvCApi_PropertyAttributes_Listable))
      .value("WRITABLE_LISTABLE",
             EnvCApi_PropertyAttributes(EnvCApi_PropertyAttributes_Writable |
                                        EnvCApi_PropertyAttributes_Listable))
      .value("READABLE_WRITABLE_LISTABLE",
             EnvCApi_PropertyAttributes(EnvCApi_PropertyAttributes_Readable |
                                        EnvCApi_PropertyAttributes_Writable |
                                        EnvCApi_PropertyAttributes_Listable));
}

}  // namespace
