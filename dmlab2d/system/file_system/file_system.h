// Copyright (C) 2017-2020 The DMLab2D Authors.
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

#ifndef DMLAB2D_SYSTEM_FILE_SYSTEM_FILE_SYSTEM_H_
#define DMLAB2D_SYSTEM_FILE_SYSTEM_FILE_SYSTEM_H_

#include <string>
#include <utility>

#include "dmlab2d/util/file_reader_types.h"

namespace deepmind::lab2d {

class FileSystem {
 public:
  // `file_sytem` must exist for the life time of FileSystem.
  explicit FileSystem(std::string runfiles,
                      const DeepMindReadOnlyFileSystem* file_system)
      : runfiles_(std::move(runfiles)), read_only_file_system_(file_system) {}

  const std::string& Runfiles() const { return runfiles_; }

  const DeepMindReadOnlyFileSystem* ReadOnlyFileSystem() const {
    return read_only_file_system_;
  }

 private:
  // Path to executables runfiles.
  std::string runfiles_;

  // Readonly file_system for reading partial contents of a file.
  const DeepMindReadOnlyFileSystem* read_only_file_system_;
};

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_SYSTEM_FILE_SYSTEM_FILE_SYSTEM_H_
