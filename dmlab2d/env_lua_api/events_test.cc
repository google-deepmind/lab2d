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
#include "dmlab2d/env_lua_api/events.h"

#include <cstring>
#include <random>

#include "absl/strings/string_view.h"
#include "absl/types/span.h"
#include "dmlab2d/lua/bind.h"
#include "dmlab2d/lua/call.h"
#include "dmlab2d/lua/lua.h"
#include "dmlab2d/lua/n_results_or_test_util.h"
#include "dmlab2d/lua/push_script.h"
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
using ::testing::ElementsAre;
using ::testing::Eq;
using ::testing::StrEq;

class EventsTest : public lua::testing::TestWithVm {
 protected:
  EventsTest() {
    LuaRandom::Register(L);
    void* default_fs = const_cast<DeepMindReadOnlyFileSystem*>(
        util::DefaultReadOnlyFileSystem());
    vm()->AddCModuleToSearchers(
        "system.sys_random", &lua::Bind<LuaRandom::Require>,
        {&prbg_, reinterpret_cast<void*>(static_cast<std::uintptr_t>(0))});
    tensor::LuaTensorRegister(L);
    vm()->AddCModuleToSearchers("system.tensor", tensor::LuaTensorConstructors,
                                {default_fs});
    vm()->AddCModuleToSearchers("system.events", &lua::Bind<Events::Module>,
                                {&events_});
  }

  Events events_;

 private:
  std::mt19937_64 prbg_;
};

constexpr char kStringEvents[] = R"(
local tensor = require 'system.tensor'
local events = require 'system.events'

events:add("eventString", "Hello")
events:add("eventString", "Hello2")
events:add("eventStrings", "He", "llo", "World")
)";

TEST_F(EventsTest, ReadStringEvents) {
  events_.Clear();
  ASSERT_THAT(lua::PushScript(L, kStringEvents, "kStringEvents"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
  ASSERT_THAT(events_.TypeCount(), Eq(2));
  EXPECT_THAT(events_.TypeName(0), StrEq("eventString"));
  EXPECT_THAT(events_.TypeName(1), StrEq("eventStrings"));

  ASSERT_THAT(events_.Count(), Eq(3));
  EnvCApi_Event event;
  events_.Export(0, &event);
  ASSERT_THAT(event.observation_count, Eq(1));
  ASSERT_THAT(absl::MakeConstSpan(event.observations[0].spec.shape,
                                  event.observations[0].spec.dims),
              ElementsAre(std::strlen("Hello")));
  ASSERT_THAT(event.observations[0].spec.type, Eq(EnvCApi_ObservationString));
  EXPECT_THAT(event.observations[0].payload.string, StrEq("Hello"));

  events_.Export(1, &event);
  ASSERT_THAT(event.observation_count, Eq(1));
  ASSERT_THAT(absl::MakeConstSpan(event.observations[0].spec.shape,
                                  event.observations[0].spec.dims),
              ElementsAre(std::strlen("Hello2")));
  ASSERT_THAT(event.observations[0].spec.type, Eq(EnvCApi_ObservationString));
  EXPECT_THAT(event.observations[0].payload.string, StrEq("Hello2"));

  events_.Export(2, &event);
  ASSERT_THAT(event.observation_count, Eq(3));
  absl::string_view elements[3] = {"He", "llo", "World"};
  for (int i = 0; i < 3; ++i) {
    ASSERT_THAT(absl::MakeConstSpan(event.observations[i].spec.shape,
                                    event.observations[i].spec.dims),
                ElementsAre(elements[i].size()));
    ASSERT_THAT(event.observations[i].spec.type, Eq(EnvCApi_ObservationString));
    EXPECT_THAT(event.observations[i].payload.string, Eq(elements[i]));
  }
}

constexpr char kDoubleEvents[] = R"(
local tensor = require 'system.tensor'
local events = require 'system.events'

events:add("eventDouble", 1.0)
events:add("eventDouble", tensor.DoubleTensor():fill(2.0))
events:add("eventDoubles", tensor.DoubleTensor{range={6}}:reshape{3,2})
)";

TEST_F(EventsTest, ReadDoubleEvents) {
  events_.Clear();
  ASSERT_THAT(lua::PushScript(L, kDoubleEvents, "kDoubleEvents"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
  ASSERT_THAT(events_.TypeCount(), Eq(2));
  EXPECT_THAT(events_.TypeName(0), StrEq("eventDouble"));
  EXPECT_THAT(events_.TypeName(1), StrEq("eventDoubles"));

  ASSERT_THAT(events_.Count(), Eq(3));
  EnvCApi_Event event;
  events_.Export(0, &event);
  ASSERT_THAT(event.observation_count, Eq(1));
  ASSERT_THAT(event.observations[0].spec.dims, Eq(0));
  ASSERT_THAT(event.observations[0].spec.type, Eq(EnvCApi_ObservationDoubles));
  EXPECT_THAT(event.observations[0].payload.doubles[0], Eq(1.0));

  events_.Export(1, &event);
  ASSERT_THAT(event.observation_count, Eq(1));
  ASSERT_THAT(event.observations[0].spec.dims, Eq(0));
  ASSERT_THAT(event.observations[0].spec.type, Eq(EnvCApi_ObservationDoubles));
  EXPECT_THAT(event.observations[0].payload.doubles[0], Eq(2.0));

  events_.Export(2, &event);
  ASSERT_THAT(event.observation_count, Eq(1));
  ASSERT_THAT(absl::MakeConstSpan(event.observations[0].spec.shape,
                                  event.observations[0].spec.dims),
              ElementsAre(3, 2));
  ASSERT_THAT(event.observations[0].spec.type, Eq(EnvCApi_ObservationDoubles));
  EXPECT_THAT(absl::MakeConstSpan(event.observations[0].payload.doubles, 6),
              ElementsAre(1, 2, 3, 4, 5, 6));
}

constexpr char kMixedEvents[] = R"(
local tensor = require 'system.tensor'
local events = require 'system.events'

events:add("eventMixed", tensor.ByteTensor{range={1, 6}}:reshape{3, 2},
                         tensor.Int32Tensor{range={2, 7}}:reshape{2, 3},
                         tensor.Int64Tensor{range={3, 10}}:reshape{4, 2},
                         tensor.DoubleTensor{range={3, 11}}:reshape{3, 3},
                         "Hello")
)";

TEST_F(EventsTest, ReadMixedEvents) {
  events_.Clear();
  ASSERT_THAT(lua::PushScript(L, kMixedEvents, "kMixedEvents"),
              IsOkAndHolds(1));
  ASSERT_THAT(lua::Call(L, 0), IsOkAndHolds(0));
  ASSERT_THAT(events_.TypeCount(), Eq(1));
  EXPECT_THAT(events_.TypeName(0), StrEq("eventMixed"));

  ASSERT_THAT(events_.Count(), Eq(1));
  EnvCApi_Event event;
  events_.Export(0, &event);
  ASSERT_THAT(event.observation_count, Eq(5));

  ASSERT_THAT(absl::MakeConstSpan(event.observations[0].spec.shape,
                                  event.observations[0].spec.dims),
              ElementsAre(3, 2));
  ASSERT_THAT(event.observations[0].spec.type, Eq(EnvCApi_ObservationBytes));
  EXPECT_THAT(absl::MakeConstSpan(event.observations[0].payload.bytes, 6),
              ElementsAre(1, 2, 3, 4, 5, 6));

  ASSERT_THAT(absl::MakeConstSpan(event.observations[1].spec.shape,
                                  event.observations[1].spec.dims),
              ElementsAre(2, 3));
  ASSERT_THAT(event.observations[1].spec.type, Eq(EnvCApi_ObservationInt32s));
  EXPECT_THAT(absl::MakeConstSpan(event.observations[1].payload.int32s, 6),
              ElementsAre(2, 3, 4, 5, 6, 7));

  ASSERT_THAT(absl::MakeConstSpan(event.observations[2].spec.shape,
                                  event.observations[2].spec.dims),
              ElementsAre(4, 2));
  ASSERT_THAT(event.observations[2].spec.type, Eq(EnvCApi_ObservationInt64s));
  EXPECT_THAT(absl::MakeConstSpan(event.observations[2].payload.int64s, 8),
              ElementsAre(3, 4, 5, 6, 7, 8, 9, 10));

  ASSERT_THAT(absl::MakeConstSpan(event.observations[3].spec.shape,
                                  event.observations[3].spec.dims),
              ElementsAre(3, 3));
  ASSERT_THAT(event.observations[3].spec.type, Eq(EnvCApi_ObservationDoubles));
  EXPECT_THAT(absl::MakeConstSpan(event.observations[3].payload.doubles, 9),
              ElementsAre(3, 4, 5, 6, 7, 8, 9, 10, 11));

  ASSERT_THAT(absl::MakeConstSpan(event.observations[4].spec.shape,
                                  event.observations[4].spec.dims),
              ElementsAre(std::strlen("Hello")));
  ASSERT_THAT(event.observations[4].spec.type, Eq(EnvCApi_ObservationString));
  EXPECT_THAT(event.observations[4].payload.string, StrEq("Hello"));
}

}  // namespace
}  // namespace deepmind::lab2d
