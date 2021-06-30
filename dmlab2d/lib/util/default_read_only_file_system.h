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

#ifndef DMLAB2D_LIB_UTIL_DEFAULT_READ_ONLY_FILE_SYSTEM_H_
#define DMLAB2D_LIB_UTIL_DEFAULT_READ_ONLY_FILE_SYSTEM_H_

#include "dmlab2d/lib/util/file_reader_types.h"

namespace deepmind::lab2d::util {

// Returns the default DeepMindReadOnlyFileSystem implemented on std::ifstream.
const DeepMindReadOnlyFileSystem* DefaultReadOnlyFileSystem();

}  // namespace deepmind::lab2d::util

#endif  // DMLAB2D_LIB_UTIL_DEFAULT_READ_ONLY_FILE_SYSTEM_H_
