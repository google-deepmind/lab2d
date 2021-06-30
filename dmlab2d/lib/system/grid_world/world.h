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

#ifndef DMLAB2D_LIB_SYSTEM_GRID_WORLD_WORLD_H_
#define DMLAB2D_LIB_SYSTEM_GRID_WORLD_WORLD_H_

#include <stddef.h>

#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "absl/container/btree_map.h"
#include "absl/strings/string_view.h"
#include "dmlab2d/lib/system/grid_world/collections/fixed_handle_map.h"
#include "dmlab2d/lib/system/grid_world/collections/handle_names.h"
#include "dmlab2d/lib/system/grid_world/handles.h"

namespace deepmind::lab2d {

// Stores names of handles, state information and render order.
class World {
 public:
  struct StateArg {
    std::string layer;
    std::string sprite;
    std::vector<std::string> group_names;
    std::string contact;
  };

  struct HitArg {
    std::string layer;
    std::string sprite;
  };

  struct UpdateOrder {
    std::string name;
    std::string function;  // Optional name of target function.
  };

  struct Args {
    // Stores the states in lexagraphical order.
    absl::btree_map<std::string, StateArg> states;
    absl::btree_map<std::string, HitArg> hits;

    // The render order is the observable layers.
    std::vector<std::string> render_order;
    std::vector<UpdateOrder> update_order;
    std::vector<std::string> custom_sprites;
    std::string out_of_bounds_sprite;
    std::string out_of_view_sprite;
  };

  explicit World(const Args& args);

  struct HitData {
    Layer layer;
    Sprite sprite_handle;
  };

  struct StateData {
    Layer layer;
    Sprite sprite_handle;
    std::vector<Group> groups;
    Contact contact_handle;
  };

  const HandleNames<Contact>& contacts() const { return named_contacts_; }
  const HandleNames<Hit>& hits() const { return named_hits_; }
  const HandleNames<Layer>& layers() const { return named_layers_; }
  const HandleNames<Group>& groups() const { return named_groups_; }
  const HandleNames<Update>& updates() const { return named_updates_; }
  const HandleNames<Sprite>& sprites() const { return named_sprites_; }
  const HandleNames<State>& states() const { return named_states_; }

  std::size_t NumRenderLayers() const { return num_render_layers_; }
  const StateData& state_data(State state) const { return state_data_[state]; }

  const HitData& hit_data(Hit hit) const { return hit_data_[hit]; }

  const std::string& update_functions(Update handle) const {
    return update_functions_[handle];
  }

  Sprite out_of_bounds_sprite() const { return out_of_bounds_sprite_; }
  Sprite out_of_view_sprite() const { return out_of_view_sprite_; }

 private:
  struct ProcessedArgs;
  explicit World(ProcessedArgs processed);

  std::vector<StateData> MakeStates(const std::vector<StateArg>& state_args);

  std::vector<HitData> MakeHitData(const std::vector<HitArg>& hit_args);

  // Layers are such that the first `num_render_layers_` are the
  // the render layers in the order specified in construction.
  const HandleNames<Layer> named_layers_;
  const HandleNames<Group> named_groups_;
  const HandleNames<Update> named_updates_;
  const HandleNames<Contact> named_contacts_;
  const HandleNames<Hit> named_hits_;
  const HandleNames<Sprite> named_sprites_;
  const HandleNames<State> named_states_;

  // Must be initialised after named_layers_, named_sprites_ and named_states_.
  const FixedHandleMap<State, StateData> state_data_;
  const FixedHandleMap<Hit, HitData> hit_data_;
  const FixedHandleMap<Update, std::string> update_functions_;
  const Sprite out_of_bounds_sprite_;
  const Sprite out_of_view_sprite_;

  // Stores the number of named_layers that are used as render layers.
  std::size_t num_render_layers_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_LIB_SYSTEM_GRID_WORLD_WORLD_H_
