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
//
// Function hooks for overriding local file operations.

#ifndef DMLAB2D_UTIL_FILE_READER_TYPES_H_
#define DMLAB2D_UTIL_FILE_READER_TYPES_H_

#include <stdbool.h>
#include <stddef.h>

struct DeepMindReadOnlyFileSystem_s;

#ifdef __cplusplus
extern "C" {
#endif

typedef struct DeepMindReadOnlyFileSystem_s DeepMindReadOnlyFileSystem;
typedef void* DeepMindReadOnlyFileHandle;

// These are optional function pointers. If null they will not be used and the
// local filesystem is used instead.
//
// Implementations of these functions must provide the following thread-safety
// properties: For a given handle value, the functions are only called
// sequentially (though potentially from different threads). For different
// handle values, the functions may be called concurrently.
//
// Users of this code may, once a file is opened, only call the functions in
// sequential order for any particular handle.
//
// Whenever a function indicates that an error has occurred, error(handle) can
// be called to retrieve a corresponding error message.
struct DeepMindReadOnlyFileSystem_s {
  // Attempts to open a file named 'filename'. Returns whether file opening was
  // successful. If successful 'get_size(handle)' and 'read(handle)' may be
  // called. Whether the call was successful or not the handle must be closed by
  // calling the corresponding 'close(&handle)'.
  bool (*open)(const char* filename, DeepMindReadOnlyFileHandle* handle);

  // Attempts to retrieve the current size of the file. Returns whether size was
  // successfully retrieved. If successful 'size' contains the current size of
  // the file.
  bool (*get_size)(DeepMindReadOnlyFileHandle handle, size_t* size);

  // Attempts to read size bytes from an absolute offset in the file. Returns
  // whether read is successful. If successful 'dest_buf' has 'size' bytes
  // written to it from 'offset' bytes within the file.
  bool (*read)(DeepMindReadOnlyFileHandle handle, size_t offset, size_t size,
               char* dest_buf);

  // If any of the previous functions returns false, a call to this function
  // will return the corresponding error.
  const char* (*error)(DeepMindReadOnlyFileHandle handle);

  // All handles must be closed. No other parts of
  // DeepMindReadOnlyFileHandleSystem may be called, for a given handle, after
  // this function is called. Passed by pointer to allow handle invalidation.
  void (*close)(DeepMindReadOnlyFileHandle* handle);
};

#ifdef __cplusplus
}  // extern "C"
#endif

#endif  // DMLAB2D_UTIL_FILE_READER_TYPES_H_
