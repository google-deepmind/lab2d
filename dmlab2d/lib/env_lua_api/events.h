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

#ifndef DMLAB2D_LIB_ENV_LUA_API_EVENTS_H_
#define DMLAB2D_LIB_ENV_LUA_API_EVENTS_H_

#include <cstdint>
#include <string>
#include <vector>

#include "absl/container/node_hash_map.h"
#include "dmlab2d/lib/lua/lua.h"
#include "dmlab2d/lib/lua/n_results_or.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind::lab2d {

// Support class for storing events generated from Lua. These events can be read
// out of DM Lab using the events part of the EnvCApi. (See: env_c_api.h.)
//
// Each event contains a list of observations. Each observation type is one of
// EnvCApi_Observation{Doubles,Bytes,String}.
class Events {
 public:
  // Returns an event module. A pointer to Events must exist in the up
  // value. [0, 1, -]
  static lua::NResultsOr Module(lua_State* L);

  // Adds event returning its index.
  int Add(std::string name);

  // Adds string observation to event at index 'event_id'.
  void AddObservation(int event_id, std::string string_value);

  // Adds DoubleTensor observation to event at index 'event_id'.
  void AddObservation(int event_id, std::vector<int> shape,
                      std::vector<double> double_tensor);

  // Adds ByteTensor observation to event at index 'event_id'.
  void AddObservation(int event_id, std::vector<int> shape,
                      std::vector<unsigned char> byte_tensor);

  // Adds Int32Tensor observation to event at index 'event_id'.
  void AddObservation(int event_id, std::vector<int> shape,
                      std::vector<std::int32_t> int32_tensor);

  // Adds Int64Tensor observation to event at index 'event_id'.
  void AddObservation(int event_id, std::vector<int> shape,
                      std::vector<std::int64_t> int64_tensor);

  // Exports an event at 'event_idx', which must be in range [0, Count()), to an
  // EnvCApi_Event structure. Observations within the `event` are invalidated by
  // calls to non-const methods.
  void Export(int event_idx, EnvCApi_Event* event);

  // Returns the number of events created since last call to ClearEvents().
  int Count() const { return events_.size(); }

  // Returns the number of event types.
  int TypeCount() const { return names_.size(); }

  // Returns the name of the event associated with event_type_id, which must be
  // in range [0, EventTypeCount()). New events types maybe added at any point
  // but the event_type_ids remain stable.
  const char* TypeName(int event_type_id) const {
    return names_[event_type_id];
  }

  // Clears all the events and their observations.
  void Clear();

 private:
  struct Event {
    int type_id;  // Event type id.

    struct Observation {
      EnvCApi_ObservationType_enum type;

      int shape_id;  // Index in shapes_ for shape of this observation.

      // Index of observation data. The array depends on type.
      // If type == EnvCApi_ObservationDoubles then index in doubles_.
      // If type == EnvCApi_ObservationBytes then index in bytes_.
      // If type == EnvCApi_ObservationString then index in string_.
      int array_id;
    };

    // List of observations associated with event.
    std::vector<Observation> observations;
  };

  // Events generated since construction or last call to Clear().
  std::vector<Event> events_;

  // Bidirectional lookup for the mapping between type_id and event_name.
  // Strings in the primary container (the map) need to be stable.
  std::vector<const char*> names_;
  absl::node_hash_map<std::string, int> name_to_id_;

  // Event observation storage.
  std::vector<std::vector<int>> shapes_;
  std::vector<std::vector<unsigned char>> bytes_;
  std::vector<std::vector<double>> doubles_;
  std::vector<std::string> strings_;
  std::vector<std::vector<std::int32_t>> int32s_;
  std::vector<std::vector<std::int64_t>> int64s_;

  // Temporary EnvCApi_Observation observation storage.
  std::vector<EnvCApi_Observation> observations_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_ENV_LUA_API_EVENTS_H_
