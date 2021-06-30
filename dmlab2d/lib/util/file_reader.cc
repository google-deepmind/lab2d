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

#include "dmlab2d/lib/util/file_reader.h"

#include <cstddef>

#include "dmlab2d/lib/util/file_reader_types.h"

namespace deepmind::lab2d::util {

FileReader::FileReader(const DeepMindReadOnlyFileSystem* readonly_fs,
                       const char* filename)
    : readonly_fs_(readonly_fs) {
  success_ = readonly_fs_->open(filename, &handle_);
}

FileReader::~FileReader() { readonly_fs_->close(&handle_); }

bool FileReader::GetSize(std::size_t* size) {
  success_ = success_ && readonly_fs_->get_size(handle_, size);
  return success_;
}

bool FileReader::Read(std::size_t offset, std::size_t size, char* dest_buf) {
  success_ = success_ && readonly_fs_->read(handle_, offset, size, dest_buf);
  return success_;
}

bool FileReader::Success() const { return success_; }

const char* FileReader::Error() { return readonly_fs_->error(handle_); }

}  // namespace deepmind::lab2d::util
