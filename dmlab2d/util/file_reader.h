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

#ifndef DMLAB2D_UTIL_READONLY_FILE_SYSTEM_H_
#define DMLAB2D_UTIL_READONLY_FILE_SYSTEM_H_

#include <cstddef>

#include "dmlab2d/util/file_reader_types.h"

namespace deepmind::lab2d::util {

// Used for opening/reading the contents of a file using a
// DeepMindReadOnlyFileSystem. If the operation was not successful 'Error()' may
// be called to retrieve an error message.
class FileReader {
 public:
  // Opens 'filename' for read. Users should call 'Success()' to retrieve
  // whether the file was successfully opened. 'readonly_fs' must outlive the
  // FileReader.
  FileReader(const DeepMindReadOnlyFileSystem* readonly_fs,
             const char* filename);

  FileReader(const FileReader&) = delete;
  FileReader& operator=(const FileReader&) = delete;
  ~FileReader();

  // Returns whether last operation was successful.
  bool Success() const;

  // Sets 'size' to the size of the file. Returns whether the size was
  // retrieved.
  bool GetSize(std::size_t* size);

  // Reads 'size' bytes from 'offset' in the file into 'dest_buf'. Returns
  // whether read completed successfully. If the operation was not successful,
  // 'dest_buf' may be partially written to.
  bool Read(std::size_t offset, std::size_t size, char* dest_buf);

  // Returns associated error message if an operation was not successful.
  // Only can be called after an unsuccessful operation.
  const char* Error();

 private:
  bool success_;  // Stores whether an operation was successful.
  DeepMindReadOnlyFileHandle handle_;  // Stores current file handle.
  const DeepMindReadOnlyFileSystem* readonly_fs_;
};

}  // namespace deepmind::lab2d::util

#endif  // DMLAB2D_UTIL_READONLY_FILE_SYSTEM_H_
