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

#include "dmlab2d/lib/util/default_read_only_file_system.h"

#include <cstddef>
#include <fstream>
#include <string>

#include "absl/strings/str_cat.h"

namespace deepmind::lab2d::util {
namespace {

class FileReaderDefault {
 public:
  FileReaderDefault(const char* filename)
      : ifs_(filename, std::ifstream::binary) {
    if (!ifs_) {
      error_message_ = absl::StrCat("Failed to open file \"", filename, "\"");
    }
  }

  bool Success() const { return error_message_.empty(); }

  bool GetSize(std::size_t* size) {
    if (!Success()) {
      return false;
    }
    if (!ifs_.seekg(0, std::ios::end)) {
      error_message_ = "Failed to read file size";
      return false;
    }
    *size = ifs_.tellg();
    return true;
  }

  bool Read(std::size_t offset, std::size_t size, char* dest_buf) {
    if (!Success()) {
      return false;
    }
    if (!ifs_.seekg(offset, std::ios::beg) || !ifs_.read(dest_buf, size)) {
      error_message_ =
          absl::StrCat("Failed to read from ", offset, " to ", offset + size);
      return false;
    } else {
      return true;
    }
  }

  const char* Error() const { return error_message_.c_str(); }

 private:
  std::ifstream ifs_;
  std::string error_message_;
};

extern "C" {

static bool deepmind_open(const char* filename,
                          DeepMindReadOnlyFileHandle* handle) {
  auto* readonly_file = new FileReaderDefault(filename);
  *handle = readonly_file;
  return readonly_file->Success();
}

static bool deepmind_get_size(DeepMindReadOnlyFileHandle handle,
                              std::size_t* size) {
  return handle != nullptr &&
         static_cast<FileReaderDefault*>(handle)->GetSize(size);
}

static bool deepmind_read(DeepMindReadOnlyFileHandle handle, std::size_t offset,
                          std::size_t size, char* dest_buf) {
  return handle != nullptr &&
         static_cast<FileReaderDefault*>(handle)->Read(offset, size, dest_buf);
}

static const char* deepmind_error(DeepMindReadOnlyFileHandle handle) {
  return handle ? static_cast<const FileReaderDefault*>(handle)->Error()
                : "Invalid Handle!";
}

static void deepmind_close(DeepMindReadOnlyFileHandle* handle) {
  delete static_cast<FileReaderDefault*>(*handle);
  *handle = nullptr;
}

}  // extern "C"

constexpr DeepMindReadOnlyFileSystem kFileSystem = {
    &deepmind_open,      //
    &deepmind_get_size,  //
    &deepmind_read,      //
    &deepmind_error,     //
    &deepmind_close,     //
};

}  // namespace

const DeepMindReadOnlyFileSystem* DefaultReadOnlyFileSystem() {
  return &kFileSystem;
}

}  // namespace deepmind::lab2d::util
