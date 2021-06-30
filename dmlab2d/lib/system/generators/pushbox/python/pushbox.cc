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

#include "dmlab2d/lib/system/generators/pushbox/pushbox.h"

#include <optional>

#include "absl/types/optional.h"
#include "include/pybind11/pybind11.h"
#include "include/pybind11/stl.h"
#include "include/pybind11/stl_bind.h"

namespace deepmind::lab2d {
namespace {

namespace py = pybind11;

// Forwards call to pushbox::GenerateLevel.
std::string GenerateLevel(std::uint32_t seed, int width, int height,
                          int num_boxes, int room_steps,
                          std::optional<std::uint32_t> room_seed,
                          std::optional<std::uint32_t> targets_seed,
                          std::optional<std::uint32_t> actions_seed) {
  pushbox::Settings settings{seed,       width,     height,       num_boxes,
                             room_steps, room_seed, targets_seed, actions_seed};
  const auto& [level, err] = pushbox::GenerateLevel(settings);
  if (!err.empty()) {
    throw std::invalid_argument(err);
  }
  return level;
}

PYBIND11_MODULE(pushbox, m) {
  m.doc() = "Pushbox level generator";
  pushbox::Settings settings;
  m.def("Generate", &GenerateLevel, "Generates level.", py::arg("seed"),
        py::arg("width") = settings.width, py::arg("height") = settings.height,
        py::arg("num_boxes") = settings.num_boxes,
        py::arg("room_steps") = settings.room_steps,
        py::arg("room_seed") = absl::nullopt,
        py::arg("targets_seed") = absl::nullopt,
        py::arg("actions_seed") = absl::nullopt);
}

}  // namespace
}  // namespace deepmind::lab2d
