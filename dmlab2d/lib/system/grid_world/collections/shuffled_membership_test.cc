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

#include "dmlab2d/lib/system/grid_world/collections/shuffled_membership.h"

#include "absl/types/span.h"
#include "dmlab2d/lib/system/grid_world/collections/handle.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {

using ::testing::IsEmpty;
using ::testing::UnorderedElementsAre;

struct TestHandleTag {
  static constexpr char kName[] = "TestHandle";
};
using TestHandle = Handle<TestHandleTag>;

TEST(ShuffledMembershipTest, AddMembership) {
  const int kNumberOfHandles = 4;
  ShuffledMembership<TestHandle, int> membership(kNumberOfHandles);

  int even = 0;
  std::vector<TestHandle> even_groups = {TestHandle(0), TestHandle(2)};
  membership.ChangeMembership(even, {}, absl::MakeConstSpan(even_groups));

  int odd = 1;
  std::vector<TestHandle> odd_groups = {TestHandle(1), TestHandle(3)};
  membership.ChangeMembership(odd, {}, absl::MakeConstSpan(odd_groups));

  int prime = 2;
  std::vector<TestHandle> prime_groups = {TestHandle(2), TestHandle(3)};
  membership.ChangeMembership(prime, {}, absl::MakeConstSpan(prime_groups));

  int all = 3;
  std::vector<TestHandle> all_groups = {TestHandle(0), TestHandle(1),
                                        TestHandle(2), TestHandle(3)};
  membership.ChangeMembership(all, {}, absl::MakeConstSpan(all_groups));

  std::mt19937_64 random;
  EXPECT_THAT(membership[TestHandle(0)].ShuffledElements(&random),
              UnorderedElementsAre(even, all));
  EXPECT_THAT(membership[TestHandle(1)].ShuffledElements(&random),
              UnorderedElementsAre(odd, all));
  EXPECT_THAT(membership[TestHandle(2)].ShuffledElements(&random),
              UnorderedElementsAre(even, prime, all));
  EXPECT_THAT(membership[TestHandle(3)].ShuffledElements(&random),
              UnorderedElementsAre(odd, prime, all));
}

TEST(ShuffledMembershipTest, RemoveMembership) {
  const int kNumberOfHandles = 4;
  ShuffledMembership<TestHandle, int> membership(kNumberOfHandles);

  int even = 0;
  std::vector<TestHandle> even_groups = {TestHandle(0), TestHandle(2)};
  membership.ChangeMembership(even, {}, absl::MakeConstSpan(even_groups));

  int odd = 1;
  std::vector<TestHandle> odd_groups = {TestHandle(1), TestHandle(3)};
  membership.ChangeMembership(odd, {}, absl::MakeConstSpan(odd_groups));

  int prime = 2;
  std::vector<TestHandle> prime_groups = {TestHandle(2), TestHandle(3)};
  membership.ChangeMembership(prime, {}, absl::MakeConstSpan(prime_groups));

  int all = 3;
  std::vector<TestHandle> all_groups = {TestHandle(0), TestHandle(1),
                                        TestHandle(2), TestHandle(3)};
  membership.ChangeMembership(all, {}, absl::MakeConstSpan(all_groups));

  // Remove even.
  membership.ChangeMembership(even, absl::MakeConstSpan(even_groups), {});

  std::mt19937_64 random;
  EXPECT_THAT(membership[TestHandle(0)].ShuffledElements(&random),
              UnorderedElementsAre(all));
  EXPECT_THAT(membership[TestHandle(1)].ShuffledElements(&random),
              UnorderedElementsAre(odd, all));
  EXPECT_THAT(membership[TestHandle(2)].ShuffledElements(&random),
              UnorderedElementsAre(prime, all));
  EXPECT_THAT(membership[TestHandle(3)].ShuffledElements(&random),
              UnorderedElementsAre(odd, prime, all));

  // Remove all.
  membership.ChangeMembership(all, absl::MakeConstSpan(all_groups), {});
  // Remove odd.
  membership.ChangeMembership(odd, absl::MakeConstSpan(odd_groups), {});

  EXPECT_THAT(membership[TestHandle(0)].ShuffledElements(&random), IsEmpty());
  EXPECT_THAT(membership[TestHandle(1)].ShuffledElements(&random), IsEmpty());
  EXPECT_THAT(membership[TestHandle(2)].ShuffledElements(&random),
              UnorderedElementsAre(prime));
  EXPECT_THAT(membership[TestHandle(3)].ShuffledElements(&random),
              UnorderedElementsAre(prime));
}

TEST(ShuffledMembershipTest, ChangeMembershipDistinct) {
  const int kNumberOfHandles = 4;
  ShuffledMembership<TestHandle, int> membership(kNumberOfHandles);

  int all = 0;
  std::vector<TestHandle> all_groups = {TestHandle(0), TestHandle(1),
                                        TestHandle(2), TestHandle(3)};
  membership.ChangeMembership(all, {}, absl::MakeConstSpan(all_groups));

  int id = 1;
  std::vector<TestHandle> even_groups = {TestHandle(0), TestHandle(2)};
  membership.ChangeMembership(id, {}, absl::MakeConstSpan(even_groups));

  std::vector<TestHandle> odd_groups = {TestHandle(1), TestHandle(3)};
  membership.ChangeMembership(id, absl::MakeConstSpan(even_groups),
                              absl::MakeConstSpan(odd_groups));

  std::mt19937_64 random;
  EXPECT_THAT(membership[TestHandle(0)].ShuffledElements(&random),
              UnorderedElementsAre(all));
  EXPECT_THAT(membership[TestHandle(1)].ShuffledElements(&random),
              UnorderedElementsAre(id, all));
  EXPECT_THAT(membership[TestHandle(2)].ShuffledElements(&random),
              UnorderedElementsAre(all));
  EXPECT_THAT(membership[TestHandle(3)].ShuffledElements(&random),
              UnorderedElementsAre(id, all));
}

TEST(ShuffledMembershipTest, ChangeMembershipOverlap) {
  const int kNumberOfHandles = 4;
  ShuffledMembership<TestHandle, int> membership(kNumberOfHandles);

  int all = 0;
  std::vector<TestHandle> all_groups = {TestHandle(0), TestHandle(1),
                                        TestHandle(2), TestHandle(3)};
  membership.ChangeMembership(all, {}, absl::MakeConstSpan(all_groups));

  int id = 1;
  std::vector<TestHandle> even_groups = {TestHandle(0), TestHandle(2)};
  membership.ChangeMembership(id, {}, absl::MakeConstSpan(even_groups));

  std::vector<TestHandle> prime_groups = {TestHandle(2), TestHandle(3)};
  membership.ChangeMembership(id, absl::MakeConstSpan(even_groups),
                              absl::MakeConstSpan(prime_groups));

  std::mt19937_64 random;
  EXPECT_THAT(membership[TestHandle(0)].ShuffledElements(&random),
              UnorderedElementsAre(all));
  EXPECT_THAT(membership[TestHandle(1)].ShuffledElements(&random),
              UnorderedElementsAre(all));
  EXPECT_THAT(membership[TestHandle(2)].ShuffledElements(&random),
              UnorderedElementsAre(id, all));
  EXPECT_THAT(membership[TestHandle(3)].ShuffledElements(&random),
              UnorderedElementsAre(id, all));
}

}  // namespace
}  // namespace deepmind::lab2d
