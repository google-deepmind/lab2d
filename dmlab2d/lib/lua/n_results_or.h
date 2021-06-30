// Copyright (C) 2016-2019 The DMLab2D Authors.
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

#ifndef DMLAB2D_LIB_LUA_N_RESULTS_OR_H_
#define DMLAB2D_LIB_LUA_N_RESULTS_OR_H_

#include <string>
#include <utility>

namespace deepmind::lab2d::lua {

// Used for returning the number of return values from a Lua function call or an
// error message. Implicit construction aids readability in call sites.
class NResultsOr {
 public:
  // Function call completed without error and n return values on the stack.
  NResultsOr(int n_results) : n_results_(n_results) {}

  // Function call failled with error message `error`.
  // Make sure no results are left on the stack, so that n_results() can be
  // popped regardless of error.
  NResultsOr(std::string error) : n_results_(0), error_(std::move(error)) {
    if (error_.empty()) {
      error_ = "(nil)";
    }
  }
  NResultsOr(const char* error) : NResultsOr(std::string(error)) {}

  int n_results() const { return n_results_; }
  bool ok() const { return error_.empty(); }
  const std::string& error() const { return error_; }

 private:
  int n_results_;
  std::string error_;  // If empty no error;
};

}  // namespace deepmind::lab2d::lua

#endif  // DMLAB2D_LIB_LUA_N_RESULTS_OR_H_
