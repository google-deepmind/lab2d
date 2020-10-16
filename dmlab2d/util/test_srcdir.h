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

#ifndef DMLAB2D_UTIL_TEST_SRCDIR_H_
#define DMLAB2D_UTIL_TEST_SRCDIR_H_

#include <string>

namespace deepmind::lab2d::util {

// Returns the name of the directory in which the test binary is located.
std::string TestSrcDir();

}  // namespace deepmind::lab2d::util

#endif  // DMLAB2D_UTIL_TEST_SRCDIR_H_
