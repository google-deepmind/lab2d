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

#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <limits>
#include <random>
#include <string>
#include <vector>

#include "absl/flags/flag.h"
#include "absl/flags/parse.h"
#include "absl/flags/usage.h"
#include "absl/strings/ascii.h"
#include "absl/strings/match.h"
#include "absl/strings/str_cat.h"
#include "absl/strings/str_format.h"
#include "absl/strings/str_join.h"
#include "absl/strings/str_split.h"
#include "absl/strings/string_view.h"
#include "absl/types/optional.h"
#include "dmlab2d/dmlab2d.h"
#include "third_party/rl_api/env_c_api.h"

namespace {

// Map entries are separated by commas. Each map entry consists of a key and
// a value separated by an '='. The value may contain additional '='. Whitespace
// around the separators is allowed and will be stripped. The empty/whitespace
// string corresponds to an empty map.
struct SettingsMap : public std::map<std::string, std::string> {
  using std::map<std::string, std::string>::map;

  friend std::string AbslUnparseFlag(const SettingsMap& value) {
    return absl::StrJoin(value, ",", absl::PairFormatter("="));
  }

  friend bool AbslParseFlag(absl::string_view text, SettingsMap* out,
                            std::string* error) {
    out->clear();
    text = absl::StripAsciiWhitespace(text);
    if (!text.empty()) {
      for (const absl::string_view item : absl::StrSplit(text, ',')) {
        std::vector<absl::string_view> parts =
            absl::StrSplit(item, absl::MaxSplits('=', 1));
        if (parts.size() != 2) {
          *error = absl::StrCat("Invalid setting ", item);
          return false;
        }
        std::string key(absl::StripAsciiWhitespace(parts[0]));
        std::string value(absl::StripAsciiWhitespace(parts[1]));
        out->insert_or_assign(std::move(key), std::move(value));
      }
    }
    return true;
  }
};

}  // namespace

ABSL_FLAG(std::string, level_name, "", "Level name");

ABSL_FLAG(SettingsMap, settings, {},
          "Comma separated list of key=value settings");

ABSL_FLAG(std::vector<std::string>, observations, {},
          "Comma separated list of observations");

ABSL_FLAG(bool, print_spec, false,
          "Prints observation and action spec and exits");

ABSL_FLAG(bool, print_actions, false, "Prints generated actions for each step");

ABSL_FLAG(bool, print_observations, false,
          "Prints observation requested observations");

ABSL_FLAG(bool, print_events, false, "Prints events generated each frame");

ABSL_FLAG(std::string, runfiles_directory, "", "Overrides runfiles driectory.");

ABSL_FLAG(std::string, print_property, "", "Prints properties after start");

ABSL_FLAG(SettingsMap, write_properties, {},
          "Comma separated list of key=value properties to write after start");

ABSL_FLAG(int, episodes, 1, "Number of episodes before termination.");

ABSL_FLAG(int, seed, 0x600D5EED,
          "Initial seed used to generate per episode seeds.");

namespace {

struct EnvCApiWithContext {
  EnvCApi api;
  void* ctx;

  void CheckCall(int result, absl::string_view message) {
    if (result != 0) {
      absl::PrintF("Error - %s %s\n", message, api.error_message(ctx));
      std::abort();
    }
  }
};

[[noreturn]] void SysError(absl::string_view message) {
  absl::PrintF("Error - %s\n", message);
  std::abort();
}

std::mt19937_64 RandomEngine() {
  return std::mt19937_64(absl::GetFlag(FLAGS_seed));
}

EnvCApiWithContext ConnectToDmLab2D(const char* runfiles) {
  EnvCApiWithContext env = {};
  DeepMindLab2DLaunchParams params = {};
  params.runfiles_root = runfiles;
  if (dmlab2d_connect(&params, &env.api, &env.ctx) != 0) {
    SysError("Failed to connect RL API");
  }
  return env;
}

void AppSettings(EnvCApiWithContext* env) {
  std::string level_name = absl::GetFlag(FLAGS_level_name);
  if (level_name.empty()) {
    SysError("Missing flag 'level_name'!");
  }
  env->CheckCall(
      env->api.setting(env->ctx, "levelName", level_name.c_str()),
      absl::StrCat("Failed to apply setting 'levelName=", level_name, "'"));
  for (const auto& key_value : absl::GetFlag(FLAGS_settings)) {
    const auto& key = key_value.first;
    const auto& value = key_value.second;
    env->CheckCall(
        env->api.setting(env->ctx, key.c_str(), value.c_str()),
        absl::StrCat("Failed to apply setting '", key, "=", value, "'"));
  }
}

void PrintActionSpec(EnvCApiWithContext* env) {
  std::puts("\nActionSpecs:");
  int discrete_count = env->api.action_discrete_count(env->ctx);
  for (int id = 0; id < discrete_count; ++id) {
    int min_val, max_val;
    env->api.action_discrete_bounds(env->ctx, id, &min_val, &max_val);
    absl::PrintF("%3d - %-16s [%3d, %-3d]\n", id,
                 env->api.action_discrete_name(env->ctx, id), min_val, max_val);
  }
  int continuous_count = env->api.action_continuous_count(env->ctx);
  for (int id = 0; id < continuous_count; ++id) {
    double min_val, max_val;
    env->api.action_continuous_bounds(env->ctx, id, &min_val, &max_val);
    absl::PrintF("%3d - %-16s [%3f, %-3f]\n", id,
                 env->api.action_continuous_name(env->ctx, id), min_val,
                 max_val);
  }
  if (discrete_count == 0 && continuous_count == 0) {
    std::puts("  [None]");
  }
}

void PrintObservationSpec(EnvCApiWithContext* env) {
  std::puts("\nObservationSpecs:");
  int obs_count = env->api.observation_count(env->ctx);
  for (int id = 0; id < obs_count; ++id) {
    EnvCApi_ObservationSpec spec;
    env->api.observation_spec(env->ctx, id, &spec);
    absl::PrintF("%3d - %-16s ", id, env->api.observation_name(env->ctx, id));
    switch (spec.type) {
      case EnvCApi_ObservationDoubles:
        absl::PrintF("Doubles ");
        break;
      case EnvCApi_ObservationBytes:
        absl::PrintF("Bytes   ");
        break;
      case EnvCApi_ObservationString:
        absl::PrintF("String  ");
        break;
      case EnvCApi_ObservationInt32s:
        absl::PrintF("Int32s  ");
        break;
      case EnvCApi_ObservationInt64s:
        absl::PrintF("Int64s  ");
        break;
    }
    if (spec.dims > 0) {
      absl::PrintF("[");
      for (int i = 0; i < spec.dims; ++i) {
        if (i != 0) {
          absl::PrintF(", ");
        }
        if (spec.shape[i] > 0) {
          absl::PrintF("%4d", spec.shape[i]);
        } else {
          absl::PrintF(" dyn");
        }
      }
      absl::PrintF("]\n");
    } else {
      absl::PrintF("scalar\n");
    }
  }
  if (obs_count == 0) {
    std::puts("  [None]");
  }
}

std::vector<int> GetObservationIds(EnvCApiWithContext* env) {
  std::vector<std::string> observations = absl::GetFlag(FLAGS_observations);
  std::vector<int> observations_ids;
  int available_obs_count = env->api.observation_count(env->ctx);
  std::vector<std::string> available_observations;
  available_observations.reserve(available_obs_count);
  for (int id = 0; id < available_obs_count; ++id) {
    available_observations.push_back(env->api.observation_name(env->ctx, id));
  }

  for (const auto& observation : observations) {
    bool any_found = false;
    for (int id = 0; id < available_obs_count; ++id) {
      const std::string& available = available_observations[id];
      if (observation == available ||
          absl::StartsWith(available, observation) ||
          absl::EndsWith(available, observation)) {
        observations_ids.push_back(id);
        any_found = true;
      }
    }
    if (!any_found) {
      SysError(absl::StrCat("Missing observation: '", observation, "'"));
    }
  }
  return observations_ids;
}

template <typename T, typename ValuePrinter>
void PrintObservationDetail(const char* type, ValuePrinter value_printer,
                            int dims, const int* shape, const T* payload) {
  if (dims == 0) {
    value_printer(payload[0]);
    return;
  }
  absl::PrintF("<%s ", type);
  for (int i = 0; i < dims; ++i) {
    if (i != 0) absl::PrintF("x");
    absl::PrintF("%d", shape[i]);
  }
  absl::PrintF(">");
  if (dims == 1) {
    absl::PrintF("{");
    for (int i = 0; i < shape[0] && i < 6; ++i) {
      if (i != 0) absl::PrintF(", ");
      value_printer(payload[i]);
    }
    if (shape[0] > 6) {
      absl::PrintF(", ...");
    }
    absl::PrintF("}");
    return;
  }
}

void PrintObservation(const EnvCApi_Observation& obs) {
  switch (obs.spec.type) {
    case EnvCApi_ObservationString:
      absl::PrintF("\"%.*s\"", obs.spec.shape[0], obs.payload.string);
      break;
    case EnvCApi_ObservationDoubles:
      PrintObservationDetail(
          "Doubles", [](double v) { absl::PrintF("%f", v); }, obs.spec.dims,
          obs.spec.shape, obs.payload.doubles);
      break;
    case EnvCApi_ObservationBytes:
      PrintObservationDetail(
          "Bytes", [](unsigned char v) { absl::PrintF("0x%02x", v); },
          obs.spec.dims, obs.spec.shape, obs.payload.bytes);
      break;
    case EnvCApi_ObservationInt32s:
      PrintObservationDetail(
          "Int32s", [](std::int32_t v) { absl::PrintF("%2d", v); },
          obs.spec.dims, obs.spec.shape, obs.payload.int32s);
      break;
    case EnvCApi_ObservationInt64s:
      PrintObservationDetail(
          "Int64s", [](std::int64_t v) { absl::PrintF("%2d", v); },
          obs.spec.dims, obs.spec.shape, obs.payload.int64s);
      break;
    default:
      SysError(
          absl::StrCat("Observation type: ", obs.spec.type, " not supported"));
  }
}

void ProcessObservations(EnvCApiWithContext* env,
                         const std::vector<int>& observation_ids,
                         bool print_observations, int frame) {
  for (int observation_id : observation_ids) {
    EnvCApi_Observation observation;
    env->api.observation(env->ctx, observation_id, &observation);
    if (print_observations) {
      absl::PrintF("%5d %s ", frame,
                   env->api.observation_name(env->ctx, observation_id));
      PrintObservation(observation);
      absl::PrintF("\n");
    }
  }
}

// Prints events to stdout. Returns number printed.
int PrintEvents(EnvCApiWithContext* env) {
  int event_count = env->api.event_count(env->ctx);
  for (int e = 0; e < event_count; ++e) {
    EnvCApi_Event event;
    env->api.event(env->ctx, e, &event);
    absl::PrintF("Event %d: \"%s\" - ", e,
                 env->api.event_type_name(env->ctx, event.id));
    for (int obs_id = 0; obs_id < event.observation_count; ++obs_id) {
      if (obs_id != 0) {
        absl::PrintF(", ");
      }
      PrintObservation(event.observations[obs_id]);
    }
    absl::PrintF("\n");
  }
  return event_count;
}

void ListProperties(EnvCApiWithContext* env) {
  absl::PrintF("\nProperties:\n");
  std::vector<std::string> roots(1);
  struct UserData {
    std::vector<std::string> new_roots;
    std::vector<std::string> all_values;
  } userdata;

  while (!roots.empty()) {
    userdata.new_roots.clear();
    for (const auto& root : roots) {
      env->api.list_property(
          env->ctx, &userdata, root.c_str(),
          +[](void* userdata, const char* key,
              EnvCApi_PropertyAttributes attributes) {
            auto* userdata_ptr = static_cast<UserData*>(userdata);
            if (attributes & EnvCApi_PropertyAttributes_ReadWritable) {
              userdata_ptr->all_values.emplace_back(key);
            }
            if (attributes & EnvCApi_PropertyAttributes_Listable) {
              userdata_ptr->new_roots.emplace_back(key);
            }
          });
    }
    std::swap(roots, userdata.new_roots);
  }
  std::sort(userdata.all_values.begin(), userdata.all_values.end());
  for (const auto& key : userdata.all_values) {
    const char* value;
    EnvCApi_PropertyResult read_result =
        env->api.read_property(env->ctx, key.c_str(), &value);
    switch (read_result) {
      case EnvCApi_PropertyResult_Success:
        absl::PrintF("  '%s'='%s'\n", key, value);
        break;
      case EnvCApi_PropertyResult_PermissionDenied:
        absl::PrintF("  '%s' (write only)\n", key);
        break;
      default:
        break;
    }
  }
  if (userdata.all_values.empty()) {
    std::puts("  [None]");
  }
}

template <typename T, typename Dist>
void RandomActions(std::mt19937_64* rng, std::vector<Dist>* range_dists,
                   std::vector<T>* actions) {
  for (int i = 0; i < range_dists->size(); ++i) {
    (*actions)[i] = (*range_dists)[i](*rng);
  }
}

void RunEpisodes(EnvCApiWithContext* env) {
  auto observation_ids = GetObservationIds(env);
  int action_discrete_count = env->api.action_discrete_count(env->ctx);
  std::vector<int> action_discrete(action_discrete_count);

  int action_continuous_count = env->api.action_continuous_count(env->ctx);
  std::vector<double> action_continuous(action_continuous_count);

  std::vector<std::uniform_int_distribution<>> distrib_discrete;
  distrib_discrete.reserve(action_discrete_count);
  for (int i = 0; i < action_discrete_count; ++i) {
    int min_value, max_value;
    env->api.action_discrete_bounds(env->ctx, i, &min_value, &max_value);
    distrib_discrete.emplace_back(min_value, max_value);
  }

  std::vector<std::uniform_real_distribution<double>> distrib_continuous;
  distrib_continuous.reserve(action_continuous_count);
  for (int i = 0; i < action_continuous_count; ++i) {
    double min_value, max_value;
    env->api.action_continuous_bounds(env->ctx, i, &min_value, &max_value);
    distrib_continuous.emplace_back(min_value, max_value);
  }

  int number_of_episodes = absl::GetFlag(FLAGS_episodes);
  auto random_engine = RandomEngine();
  std::uniform_int_distribution<> seed_dist(0, std::numeric_limits<int>::max());
  bool print_observations = absl::GetFlag(FLAGS_print_observations);
  bool print_events = absl::GetFlag(FLAGS_print_events);
  for (int episode_id = 0; episode_id < number_of_episodes; ++episode_id) {
    int game_seed = seed_dist(random_engine);
    int action_seed = seed_dist(random_engine);
    std::mt19937_64 action_random_engine(action_seed);
    env->CheckCall(env->api.start(env->ctx, episode_id, game_seed),
                   "Failed to 'start':\n");
    EnvCApi_EnvironmentStatus status = EnvCApi_EnvironmentStatus_Running;
    for (int frame = 0; status == EnvCApi_EnvironmentStatus_Running; ++frame) {
      ProcessObservations(env, observation_ids, print_observations, frame);
      double reward;

      for (std::size_t i = 0; i < action_discrete.size(); ++i) {
        action_discrete[i] = distrib_discrete[i](action_random_engine);
      }

      for (std::size_t i = 0; i < action_continuous.size(); ++i) {
        action_continuous[i] = distrib_continuous[i](action_random_engine);
      }

      env->api.act_discrete(env->ctx, action_discrete.data());
      env->api.act_continuous(env->ctx, action_continuous.data());

      // Events are cleared at the begining of advance.
      if (print_events) {
        PrintEvents(env);
      }
      status = env->api.advance(env->ctx, 1, &reward);
    }
    if (status == EnvCApi_EnvironmentStatus_Error) {
      env->CheckCall(1, "Failed to 'advance':\n");
    }
  }
}

}  // namespace

int main(int argc, char** argv) {
  absl::SetProgramUsageMessage(
      "Runs a DeepMind Lab2D environment with a random agent.");
  auto free_args = absl::ParseCommandLine(argc, argv);
  if (free_args.size() != 1) {
    SysError("Command-line tool does not accept any positional arguments.");
  }

  std::string runfiles;
  if (std::string user_runfiles = absl::GetFlag(FLAGS_runfiles_directory);
      !user_runfiles.empty()) {
    runfiles = user_runfiles;
  } else {
    runfiles = absl::StrCat(free_args.front(), ".runfiles");
  }

  EnvCApiWithContext env = ConnectToDmLab2D(runfiles.c_str());
  AppSettings(&env);
  env.CheckCall(env.api.init(env.ctx), "Failed to 'init':\n");
  if (absl::GetFlag(FLAGS_print_spec)) {
    PrintActionSpec(&env);
    PrintObservationSpec(&env);
    ListProperties(&env);
  } else {
    RunEpisodes(&env);
  }

  env.api.release_context(env.ctx);
  return EXIT_SUCCESS;
}
