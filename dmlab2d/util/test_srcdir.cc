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

#include "dmlab2d/util/test_srcdir.h"

#include <cstdlib>
#include <string>

#ifndef SUPPRESS_COMMANDLINE_FLAGS
#include "absl/flags/flag.h"
#include "dmlab2d/support/commandlineflags_declare.h"

DECLARE_string(test_srcdir);
#endif

namespace deepmind::lab2d::util {

std::string TestSrcDir() {
  if (const char* e = std::getenv("TEST_SRCDIR")) {
    return std::string(e);
  } else {
#ifndef SUPPRESS_COMMANDLINE_FLAGS
    return absl::GetFlag(FLAGS_test_srcdir);
#else
    return "[undefined TEST_SRCDIR environment variable]";
#endif
  }
}

}  // namespace deepmind::lab2d::util
