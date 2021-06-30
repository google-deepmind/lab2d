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

#include "dmlab2d/lib/system/grid_world/world.h"

#include <algorithm>
#include <utility>
#include <vector>

#include "absl/container/btree_set.h"
#include "absl/strings/string_view.h"
#include "dmlab2d/lib/system/grid_world/handles.h"

namespace deepmind::lab2d {
namespace {

void MakeOrderedUnique(std::vector<std::string>* vec) {
  std::sort(vec->begin(), vec->end());
  vec->erase(std::unique(vec->begin(), vec->end()), vec->end());
  vec->shrink_to_fit();
}

}  // namespace

struct World::ProcessedArgs {
  ProcessedArgs(const Args& args) {
    absl::btree_set<std::string> layer_names_set;
    sprite_names = args.custom_sprites;
    out_of_bounds_sprite = args.out_of_bounds_sprite;
    if (!out_of_bounds_sprite.empty()) {
      sprite_names.push_back(out_of_bounds_sprite);
    }
    out_of_view_sprite = args.out_of_view_sprite;
    if (!out_of_view_sprite.empty()) {
      sprite_names.push_back(out_of_view_sprite);
    }
    state_names.reserve(args.states.size());
    state_args.reserve(args.states.size());
    for (const auto& hit_arg : args.hits) {
      hit_names.push_back(hit_arg.first);
      if (!hit_arg.second.sprite.empty()) {
        sprite_names.push_back(hit_arg.second.sprite);
      }
      if (!hit_arg.second.layer.empty()) {
        layer_names_set.insert(hit_arg.second.layer);
      }
    }

    for (const auto& [state_name, state] : args.states) {
      state_names.push_back(state_name);
      for (const auto& group : state.group_names) {
        group_names.push_back(group);
      }
      if (!state.sprite.empty()) {
        sprite_names.push_back(state.sprite);
      }
      if (!state.layer.empty()) {
        layer_names_set.insert(state.layer);
      }
      if (!state.contact.empty()) {
        contact_names.push_back(state.contact);
      }
      state_args.push_back(state);
    }

    MakeOrderedUnique(&hit_names);
    MakeOrderedUnique(&group_names);
    MakeOrderedUnique(&contact_names);
    MakeOrderedUnique(&sprite_names);

    hit_args.reserve(hit_names.size());
    for (const auto& name : hit_names) {
      auto it = args.hits.find(name);
      if (it != args.hits.end()) {
        hit_args.push_back(it->second);
      } else {
        hit_args.emplace_back();
      }
    }

    layer_names.reserve(layer_names_set.size());
    num_render_layers = args.render_order.size();
    // Extract render_order layer names and place them first in layer_names.
    for (const auto& name : args.render_order) {
      layer_names_set.erase(name);
      layer_names.push_back(name);
    }
    // Place remaining layer names in lexagraphical order.
    for (const auto& layer : layer_names_set) {
      layer_names.push_back(layer);
    }

    update_names.reserve(args.update_order.size());
    update_functions.reserve(args.update_order.size());
    for (const auto& update_order : args.update_order) {
      update_names.push_back(update_order.name);
      update_functions.push_back(!update_order.function.empty()
                                     ? update_order.function
                                     : update_order.name);
    }
  }
  std::vector<std::string> state_names;
  std::vector<std::string> sprite_names;
  std::vector<std::string> layer_names;
  std::vector<std::string> group_names;
  std::vector<std::string> update_names;
  std::vector<std::string> update_functions;
  std::vector<std::string> contact_names;
  std::vector<std::string> hit_names;
  std::vector<StateArg> state_args;
  std::vector<HitArg> hit_args;
  std::string out_of_bounds_sprite;
  std::string out_of_view_sprite;
  std::size_t num_render_layers;
};

std::vector<World::StateData> World::MakeStates(
    const std::vector<World::StateArg>& state_args) {
  std::vector<StateData> states;
  states.reserve(state_args.size());

  for (const auto& state_arg : state_args) {
    auto& state = states.emplace_back();
    state.layer = layers().ToHandle(state_arg.layer);
    state.sprite_handle = sprites().ToHandle(state_arg.sprite);
    state.groups = named_groups_.ToHandles(state_arg.group_names);
    state.contact_handle = contacts().ToHandle(state_arg.contact);
  }
  return states;
}

std::vector<World::HitData> World::MakeHitData(
    const std::vector<World::HitArg>& hit_args) {
  std::vector<HitData> hit_datas;
  hit_datas.reserve(hit_args.size());
  for (const auto& hit_arg : hit_args) {
    auto& hit_data = hit_datas.emplace_back();
    hit_data.layer = layers().ToHandle(hit_arg.layer);
    hit_data.sprite_handle = sprites().ToHandle(hit_arg.sprite);
  }
  return hit_datas;
}

World::World(const World::Args& args) : World(ProcessedArgs(args)) {}

World::World(World::ProcessedArgs processed_args)
    : named_layers_(std::move(processed_args.layer_names)),
      named_groups_(std::move(processed_args.group_names)),
      named_updates_(std::move(processed_args.update_names)),
      named_contacts_(std::move(processed_args.contact_names)),
      named_hits_(std::move(processed_args.hit_names)),
      named_sprites_(std::move(processed_args.sprite_names)),
      named_states_(std::move(processed_args.state_names)),
      // Must be initialised after named_layers_, named_sprites_ and
      // named_states_.
      state_data_(MakeStates(processed_args.state_args)),
      // Must be initialised after named_layers_, and named_sprites_.
      hit_data_(MakeHitData(processed_args.hit_args)),
      update_functions_(std::move(processed_args.update_functions)),
      out_of_bounds_sprite_(
          sprites().ToHandle(processed_args.out_of_bounds_sprite)),
      out_of_view_sprite_(
          sprites().ToHandle(processed_args.out_of_view_sprite)),
      num_render_layers_(processed_args.num_render_layers) {}

}  // namespace deepmind::lab2d
