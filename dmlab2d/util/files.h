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

#ifndef DMLAB2D_UTIL_FILES_H_
#define DMLAB2D_UTIL_FILES_H_

#include <string>

#include "absl/strings/string_view.h"

namespace deepmind::lab2d::util {

// Recursively builds the directory for the `path` specified.
// `path` shall be in canonicalised form.
// Returns whether the folder was successfully made.
bool MakeDirectory(const std::string& path);

// Recursively removes the directory for the `path` specified.
void RemoveDirectory(const std::string& path);

// Returns an existing temporary directory.
std::string GetTempDirectory();

// A file is written with `contents` into a temporary file first, then renamed
// to `file_name`. The directory name of `file_name` must be an existing
// directory. The temporary file is created in the `scratch_directory` if it is
// not null or empty, otherwise it is created in the system temporary directory.
// Returns whether the file was successfully created with contents.
bool SetContents(const std::string& file_name, absl::string_view contents,
                 const char* scratch_directory = nullptr);

// File at `file_name` is read into `contents`.
// Returns whether `file_name` was successfully read.
bool GetContents(const std::string& file_name, std::string* contents);

}  // namespace deepmind::lab2d::util

#endif  // DMLAB2D_UTIL_FILES_H_
