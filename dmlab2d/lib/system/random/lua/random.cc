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

#include "dmlab2d/lib/system/random/lua/random.h"

#include <algorithm>
#include <cstdint>
#include <limits>
#include <random>
#include <vector>

#include "absl/strings/numbers.h"
#include "absl/strings/string_view.h"
#include "dmlab2d/lib/lua/class.h"
#include "dmlab2d/lib/lua/lua.h"
#include "dmlab2d/lib/lua/push.h"
#include "dmlab2d/lib/lua/read.h"

namespace deepmind::lab2d {
namespace {

// The fundamental integer type of our pseudo-random bit generator,
// an unsigned 64(ish)-bit integer.
using RbgNumType = ::std::mt19937_64::result_type;

// A range-checking version of lua::Read that results in an error if the number
// on the Lua stack is too large to be represented by a 64-bit integer.
//
// Warning: this code is subtle. E.g. it has to be "x < " below, and not "x <=",
// and we must check for "true" comparisons, not false ones, in order to catch
// NaNs.
bool ReadLargeNumber(lua_State* L, int idx, RbgNumType* num) {
  lua_Number x;

  if (lua::Read(L, idx, &x) && std::numeric_limits<RbgNumType>::min() <= x &&
      x < std::numeric_limits<RbgNumType>::max()) {
    *num = x;
    return true;
  } else {
    return false;
  }
}

// Fisher-Yates shuffle with Lua indexing.
void ShuffleInplaceTable(lua_State* L, int idx, std::mt19937_64* prbg) {
  std::size_t n = lua::ArrayLength(L, idx);
  for (int i = 1; i < n; ++i) {
    int j = std::uniform_int_distribution<int>(i, n)(*prbg);
    if (i != j) {
      lua_rawgeti(L, idx, i);
      lua_rawgeti(L, idx, j);
      lua_rawseti(L, idx, i);
      lua_rawseti(L, idx, j);
    }
  }
}

}  // namespace

lua::NResultsOr LuaRandom::Require(lua_State* L) {
  if (auto* prbg = static_cast<std::mt19937_64*>(
          lua_touserdata(L, lua_upvalueindex(1)))) {
    std::uintptr_t mixer_seed = reinterpret_cast<std::uintptr_t>(
        lua_touserdata(L, lua_upvalueindex(2)));
    LuaRandom::CreateObject(L, prbg, mixer_seed);
    return 1;
  } else {
    return "Missing std::mt19937_64 pointer in up value!";
  }
}

const char* LuaRandom::ClassName() { return "lab2d.Random"; }

void LuaRandom::Register(lua_State* L) {
  const Class::Reg methods[] = {
      {"discreteDistribution",
       &Class::Member<&LuaRandom::DiscreteDistribution>},
      {"normalDistribution", &Class::Member<&LuaRandom::NormalDistribution>},
      {"poissonDistribution", &Class::Member<&LuaRandom::PoissonDistribution>},
      {"seed", &Class::Member<&LuaRandom::Seed>},
      {"choice", &Class::Member<&LuaRandom::Choice>},
      {"uniformInt", &Class::Member<&LuaRandom::UniformInt>},
      {"uniformReal", &Class::Member<&LuaRandom::UniformReal>},
      {"shuffleInPlace", &Class::Member<&LuaRandom::ShuffleInPlace>},
      {"shuffle", &Class::Member<&LuaRandom::ShuffleToCopy>},
  };
  Class::Register(L, methods);
}

lua::NResultsOr LuaRandom::Seed(lua_State* L) {
  absl::string_view s;
  RbgNumType k;

  if (ReadLargeNumber(L, 2, &k) ||
      (lua::Read(L, 2, &s) && absl::SimpleAtoi(s, &k))) {
    prbg_->seed(k ^ mixer_seq_);
    return 0;
  }

  return absl::StrCat("Argument '", lua::ToString(L, 2),
                      "' is not a valid seed value.");
}

lua::NResultsOr LuaRandom::UniformInt(lua_State* L) {
  lua_Integer a, b;

  if (!IsFound(lua::Read(L, 2, &a)) || !IsFound(lua::Read(L, 3, &b)) ||
      !(a <= b)) {
    return absl::StrCat("Arguments ['", lua::ToString(L, 2), "', '",
                        lua::ToString(L, 3), "'] do not form a valid range.");
  }
  lua::Push(L, std::uniform_int_distribution<lua_Integer>(a, b)(*prbg_));
  return 1;
}

lua::NResultsOr LuaRandom::UniformReal(lua_State* L) {
  lua_Number a, b;
  if (!IsFound(lua::Read(L, 2, &a)) || !lua::Read(L, 3, &b) || !(a <= b) ||
      !(b - a <= std::numeric_limits<lua_Number>::max())) {
    return absl::StrCat("Arguments ['", lua::ToString(L, 2), "', '",
                        lua::ToString(L, 3), "'] do not form a valid range.");
  }
  lua::Push(L, std::uniform_real_distribution<lua_Number>(a, b)(*prbg_));
  return 1;
}

lua::NResultsOr LuaRandom::NormalDistribution(lua_State* L) {
  lua_Number mean, stddev;
  if (!IsFound(lua::Read(L, 2, &mean)) || !IsFound(lua::Read(L, 3, &stddev))) {
    return absl::StrCat("Invalid arguments '", lua::ToString(L, 2), "', '",
                        lua::ToString(L, 3), "' - 2 numbers expected.");
  }
  lua::Push(L, std::normal_distribution<lua_Number>(mean, stddev)(*prbg_));
  return 1;
}

lua::NResultsOr LuaRandom::PoissonDistribution(lua_State* L) {
  lua_Number mean;
  if (!IsFound(lua::Read(L, 2, &mean))) {
    return absl::StrCat("Invalid mean '", lua::ToString(L, 2),
                        "' - 1 number expected.");
  }
  lua::Push(L, std::poisson_distribution<>(mean)(*prbg_));
  return 1;
}

lua::NResultsOr LuaRandom::DiscreteDistribution(lua_State* L) {
  std::vector<double> weights;

  if (!IsFound(lua::Read(L, 2, &weights)) || weights.empty()) {
    return "Invalid arguments - non empty list of numeric weights expected.";
  }
  lua::Push(L, std::discrete_distribution<lua_Integer>(weights.begin(),
                                                       weights.end())(*prbg_) +
                   1);
  return 1;
}

lua::NResultsOr LuaRandom::ShuffleInPlace(lua_State* L) {
  if (lua_gettop(L) != 2 || lua_type(L, 2) != LUA_TTABLE) {
    return "Invalid arguments - Expects a Lua array.";
  }
  ShuffleInplaceTable(L, 2, prbg_);
  return 0;
}

lua::NResultsOr LuaRandom::ShuffleToCopy(lua_State* L) {
  if (lua_gettop(L) != 2 || lua_type(L, 2) != LUA_TTABLE) {
    return "Invalid arguments - Expects a Lua array.";
  }
  std::size_t n = lua::ArrayLength(L, 2);
  lua_createtable(L, n, 0);
  for (int i = 1; i <= n; ++i) {
    lua_rawgeti(L, 2, i);
    lua_rawseti(L, 3, i);
  }
  ShuffleInplaceTable(L, 3, prbg_);
  return 1;
}

lua::NResultsOr LuaRandom::Choice(lua_State* L) {
  if (lua_gettop(L) != 2 || lua_type(L, 2) != LUA_TTABLE) {
    return "Invalid arguments - Expects a Lua array.";
  }
  std::size_t n = lua::ArrayLength(L, 2);
  if (n == 0) {
    lua_pushnil(L);
    return 1;
  }
  lua_rawgeti(L, 2, std::uniform_int_distribution<>(1, n)(*prbg_));
  return 1;
}

}  // namespace deepmind::lab2d
