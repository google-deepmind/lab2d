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
#include "dmlab2d/env_lua_api/observations.h"

#include <cstdint>
#include <random>

#include "absl/types/span.h"
#include "dmlab2d/lua/bind.h"
#include "dmlab2d/lua/call.h"
#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/n_results_or_test_util.h"
#include "dmlab2d/lua/push_script.h"
#include "dmlab2d/lua/table_ref.h"
#include "dmlab2d/lua/vm.h"
#include "dmlab2d/lua/vm_test_util.h"
#include "dmlab2d/system/random/lua/random.h"
#include "dmlab2d/system/tensor/lua/tensor.h"
#include "dmlab2d/util/default_read_only_file_system.h"
#include "dmlab2d/util/file_reader_types.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include "third_party/rl_api/env_c_api.h"

namespace deepmind::lab2d {
namespace {

using ::deepmind::lab2d::lua::testing::IsOkAndHolds;
using ::testing::Eq;
using ::testing::StrEq;

class ObservationsTest : public lua::testing::TestWithVm {
 protected:
  ObservationsTest() {
    LuaRandom::Register(L);
    void* default_fs = const_cast<DeepMindReadOnlyFileSystem*>(
        util::DefaultReadOnlyFileSystem());
    vm()->AddCModuleToSearchers(
        "system.sys_random", &lua::Bind<LuaRandom::Require>,
        {&prbg_, reinterpret_cast<void*>(static_cast<std::uintptr_t>(0))});
    tensor::LuaTensorRegister(L);
    vm()->AddCModuleToSearchers("system.tensor", tensor::LuaTensorConstructors,
                                {default_fs});
  }

 private:
  std::mt19937_64 prbg_;
};

constexpr char kStringSpec[] = R"(
return {
    observationSpec = function(_)
      return {{
          name = 'TEXT',
          type = 'String',
      }}
    end,
    observation = function(_, index)
      assert(index == 1, 'index is ' .. index)
      return "world"
    end
})";

TEST_F(ObservationsTest, ReadStringSpec) {
  ASSERT_THAT(lua::PushScript(L, kStringSpec, "kStringSpec"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef table;
  ASSERT_TRUE(IsFound(Read(L, 1, &table)));
  lua_settop(L, 0);
  Observations observations;
  EXPECT_THAT(observations.BindApi(table), IsOkAndHolds(0));

  EXPECT_THAT(observations.Count(), Eq(1));
  EXPECT_THAT(observations.Name(0), StrEq("TEXT"));
  EnvCApi_ObservationSpec spec;
  observations.Spec(0, &spec);
  ASSERT_THAT(spec.dims, Eq(0));
  ASSERT_THAT(spec.type, Eq(EnvCApi_ObservationString));

  EnvCApi_Observation obs;
  observations.Observation(0, &obs);
  ASSERT_THAT(obs.spec.dims, Eq(1));
  ASSERT_THAT(obs.spec.shape[0], Eq(std::strlen("world")));
  ASSERT_THAT(obs.spec.type, Eq(EnvCApi_ObservationString));
  ASSERT_THAT(obs.payload.string, StrEq("world"));
}

constexpr char kTensorSpec[] = R"(
local tensor = require 'system.tensor'
local observation = tensor.Int32Tensor{range={6}}:reshape{2, 3}
return {
    observationSpec = function(_)
      return {{
          name = 'TENSOR',
          type = observation:type(),
          shape = observation:shape(),
      }}
    end,
    observation = function(_, index)
      assert(index == 1, 'index is ' .. index)
      return observation
    end
})";

TEST_F(ObservationsTest, ReadTensorSpec) {
  ASSERT_THAT(lua::PushScript(L, kTensorSpec, "kTensorSpec"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef table;
  ASSERT_TRUE(IsFound(Read(L, 1, &table)));
  lua_settop(L, 0);
  Observations observations;
  EXPECT_THAT(observations.BindApi(table), IsOkAndHolds(0));

  EXPECT_THAT(observations.Count(), Eq(1));
  EXPECT_THAT(observations.Name(0), StrEq("TENSOR"));
  EnvCApi_ObservationSpec spec;
  observations.Spec(0, &spec);
  auto spec_shape = absl::MakeConstSpan(spec.shape, spec.dims);
  ASSERT_THAT(spec_shape, testing::ElementsAre(2, 3));
  ASSERT_THAT(spec.type, Eq(EnvCApi_ObservationInt32s));

  EnvCApi_Observation obs;
  observations.Observation(0, &obs);
  auto obs_shape = absl::MakeConstSpan(obs.spec.shape, obs.spec.dims);
  ASSERT_THAT(obs_shape, Eq(spec_shape));
  ASSERT_THAT(obs.spec.type, Eq(spec.type));

  auto obs_payload = absl::MakeConstSpan(obs.payload.int32s, 6);
  ASSERT_THAT(obs_payload, testing::ElementsAre(1, 2, 3, 4, 5, 6));
}

constexpr char kDoubleSpec[] = R"(
return {
    observationSpec = function(_)
      return {{
          name = 'DOUBLE',
          type = 'tensor.DoubleTensor',
          shape = {},
      }}
    end,
    observation = function(_, index)
      assert(index == 1, 'index is ' .. index)
      return 5.0
    end
})";

TEST_F(ObservationsTest, ReadDoubleSpec) {
  ASSERT_THAT(lua::PushScript(L, kDoubleSpec, "kDoubleSpec"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef table;
  ASSERT_TRUE(IsFound(Read(L, 1, &table)));
  lua_settop(L, 0);
  Observations observations;
  EXPECT_THAT(observations.BindApi(table), IsOkAndHolds(0));

  EXPECT_THAT(observations.Count(), Eq(1));
  EXPECT_THAT(observations.Name(0), StrEq("DOUBLE"));
  EnvCApi_ObservationSpec spec;
  observations.Spec(0, &spec);
  ASSERT_THAT(spec.dims, Eq(0));
  ASSERT_THAT(spec.type, Eq(EnvCApi_ObservationDoubles));

  EnvCApi_Observation obs;
  observations.Observation(0, &obs);
  ASSERT_THAT(obs.spec.dims, Eq(spec.dims));
  ASSERT_THAT(obs.spec.type, Eq(spec.type));
  auto obs_payload = absl::MakeConstSpan(obs.payload.doubles, 1);
  ASSERT_THAT(obs_payload, testing::ElementsAre(5));
}

constexpr char kInt32Spec[] = R"(
return {
    observationSpec = function(_)
      return {{
          name = 'INT',
          type = 'tensor.Int32Tensor',
          shape = {},
      }}
    end,
    observation = function(_, index)
      assert(index == 1, 'index is ' .. index)
      return 5
    end
})";

TEST_F(ObservationsTest, ReadInt32Spec) {
  ASSERT_THAT(lua::PushScript(L, kInt32Spec, "kInt32Spec"), IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef table;
  ASSERT_TRUE(IsFound(Read(L, 1, &table)));
  lua_settop(L, 0);
  Observations observations;
  EXPECT_THAT(observations.BindApi(table), IsOkAndHolds(0));

  EXPECT_THAT(observations.Count(), Eq(1));
  EXPECT_THAT(observations.Name(0), StrEq("INT"));
  EnvCApi_ObservationSpec spec;
  observations.Spec(0, &spec);
  ASSERT_THAT(spec.dims, Eq(0));
  ASSERT_THAT(spec.type, Eq(EnvCApi_ObservationInt32s));

  EnvCApi_Observation obs;
  observations.Observation(0, &obs);
  ASSERT_THAT(obs.spec.dims, Eq(spec.dims));
  ASSERT_THAT(obs.spec.type, Eq(spec.type));
  auto obs_payload = absl::MakeConstSpan(obs.payload.int32s, 1);
  ASSERT_THAT(obs_payload, testing::ElementsAre(5));
}

constexpr char kGeneralSpec[] = R"(
local tensor = require 'system.tensor'
local values = {
  tensor.DoubleTensor{range={6}}:reshape{2, 3},
  tensor.ByteTensor{range={9}}:reshape{3, 3},
  tensor.Int64Tensor():fill(10),
  37
}
return {
    observationSpec = function(_)
      return {
          {
              name = 'DynamicDoubles',
              type = 'tensor.DoubleTensor',
              shape = {2, 0},
          },
          {
              name = 'Bytes',
              type = values[2]:type(),
              shape = values[2]:shape(),
          },
          {
              name = 'Scalar0Int64',
              type = values[3]:type(),
              shape = values[3]:shape(),
          },
          {
              name = 'Scalar1Int64',
              type = 'tensor.Int64Tensor',
              shape = {},
          },
      }
    end,
    observation = function(_, index)
      return values[index]
    end
})";

TEST_F(ObservationsTest, ReadGeneralSpec) {
  ASSERT_THAT(lua::PushScript(L, kGeneralSpec, "kGeneralSpec"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(1));
  lua::TableRef table;
  ASSERT_TRUE(IsFound(Read(L, 1, &table)));
  lua_settop(L, 0);
  Observations observations;
  EXPECT_THAT(observations.BindApi(table), IsOkAndHolds(0));

  ASSERT_THAT(observations.Count(), Eq(4));

  {
    EXPECT_THAT(observations.Name(0), StrEq("DynamicDoubles"));
    EnvCApi_ObservationSpec spec;
    observations.Spec(0, &spec);
    auto spec_shape = absl::MakeConstSpan(spec.shape, spec.dims);
    ASSERT_THAT(spec_shape, testing::ElementsAre(2, 0));
    EnvCApi_Observation obs;
    observations.Observation(0, &obs);
    auto obs_spec_shape = absl::MakeConstSpan(obs.spec.shape, obs.spec.dims);
    ASSERT_THAT(obs_spec_shape, testing::ElementsAre(2, 3));
    auto obs_payload = absl::MakeConstSpan(obs.payload.doubles, 2 * 3);
    ASSERT_THAT(obs_payload, testing::ElementsAre(1, 2, 3, 4, 5, 6));
  }
  {
    EXPECT_THAT(observations.Name(1), StrEq("Bytes"));
    EnvCApi_ObservationSpec spec;
    observations.Spec(1, &spec);
    auto spec_shape = absl::MakeConstSpan(spec.shape, spec.dims);
    ASSERT_THAT(spec_shape, testing::ElementsAre(3, 3));
    EnvCApi_Observation obs;
    observations.Observation(1, &obs);
    auto obs_spec_shape = absl::MakeConstSpan(obs.spec.shape, obs.spec.dims);
    ASSERT_THAT(obs_spec_shape, testing::ElementsAre(3, 3));
    auto obs_payload = absl::MakeConstSpan(obs.payload.bytes, 3 * 3);
    ASSERT_THAT(obs_payload, testing::ElementsAre(1, 2, 3, 4, 5, 6, 7, 8, 9));
  }
  {
    EXPECT_THAT(observations.Name(2), StrEq("Scalar0Int64"));
    EnvCApi_ObservationSpec spec;
    observations.Spec(2, &spec);
    auto spec_shape = absl::MakeConstSpan(spec.shape, spec.dims);
    ASSERT_THAT(spec_shape, testing::IsEmpty());
    EnvCApi_Observation obs;
    observations.Observation(2, &obs);
    auto obs_spec_shape = absl::MakeConstSpan(obs.spec.shape, obs.spec.dims);
    ASSERT_THAT(obs_spec_shape, testing::IsEmpty());
    auto obs_payload = absl::MakeConstSpan(obs.payload.int64s, 1);
    ASSERT_THAT(obs_payload, testing::ElementsAre(10));
  }
  {
    EXPECT_THAT(observations.Name(3), StrEq("Scalar1Int64"));
    EnvCApi_ObservationSpec spec;
    observations.Spec(3, &spec);
    auto spec_shape = absl::MakeConstSpan(spec.shape, spec.dims);
    ASSERT_THAT(spec_shape, testing::IsEmpty());
    EnvCApi_Observation obs;
    observations.Observation(3, &obs);
    auto obs_spec_shape = absl::MakeConstSpan(obs.spec.shape, obs.spec.dims);
    ASSERT_THAT(obs_spec_shape, testing::IsEmpty());
    auto obs_payload = absl::MakeConstSpan(obs.payload.int64s, 1);
    ASSERT_THAT(obs_payload, testing::ElementsAre(37));
  }
}

}  // namespace
}  // namespace deepmind::lab2d
