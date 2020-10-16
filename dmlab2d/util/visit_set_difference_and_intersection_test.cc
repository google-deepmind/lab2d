// Copyright (C) 2017-2019 The DMLab2D Authors.
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

#include "dmlab2d/util/visit_set_difference_and_intersection.h"

#include <functional>
#include <set>

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {

using ::testing::ElementsAre;
using ::testing::Eq;

TEST(VisitSetDifferencesAndIntersectionTest, Empty) {
  std::set<int> set1 = {};
  std::set<int> set2 = {};
  VisitSetDifferencesAndIntersection(
      set1.begin(), set1.end(), set2.begin(), set2.end(),
      /*f1=*/[](int i) { FAIL() << "Must not visit only in 1"; },
      /*f2=*/[](int i) { FAIL() << "Must not visit only in 2"; },
      /*f_both=*/[](int i) { FAIL() << "Must not visit in both"; });
}

TEST(VisitSetDifferencesAndIntersectionTest, OnlyIn1) {
  std::set<int> set1 = {1, 2, 3};
  std::set<int> set2 = {};

  std::vector<int> only_in_one;
  VisitSetDifferencesAndIntersection(
      set1.begin(), set1.end(), set2.begin(), set2.end(),
      /*f1=*/[&only_in_one](int i) { only_in_one.push_back(i); },
      /*f2=*/[](int i) { FAIL() << "Must not visit only in 2"; },
      /*f_both=*/[](int i) { FAIL() << "Must not visit in both"; });
  EXPECT_THAT(only_in_one, ElementsAre(1, 2, 3));
}

TEST(VisitSetDifferencesAndIntersectionTest, OnlyIn2) {
  std::set<int> set1 = {};
  std::set<int> set2 = {1, 2, 3};

  std::vector<int> only_in_two;
  VisitSetDifferencesAndIntersection(
      set1.begin(), set1.end(), set2.begin(), set2.end(),
      /*f1=*/[](int i) { FAIL() << "Must not visit only in 1"; },
      /*f2=*/[&only_in_two](int i) { only_in_two.push_back(i); },
      /*f_both=*/[](int i) { FAIL() << "Must not visit in both"; });
  EXPECT_THAT(only_in_two, ElementsAre(1, 2, 3));
}

TEST(VisitSetDifferencesAndIntersectionTest, OnlyInBoth) {
  std::set<int> set1 = {1, 2, 3};
  std::set<int> set2 = {1, 2, 3};

  std::vector<int> only_in_both;
  VisitSetDifferencesAndIntersection(
      set1.begin(), set1.end(), set2.begin(), set2.end(),
      /*f1=*/[](int i) { FAIL() << "Must not visit only in 1"; },
      /*f2=*/[](int i) { FAIL() << "Must not visit only in 2"; },
      /*f_both=*/[&only_in_both](int i) { only_in_both.push_back(i); });
  EXPECT_THAT(only_in_both, ElementsAre(1, 2, 3));
}

TEST(VisitSetDifferencesAndIntersectionTest, Distinct) {
  std::set<int> set1 = {1, 2, 3};
  std::set<int> set2 = {4, 5, 6};

  std::vector<int> only_in_one;
  std::vector<int> only_in_two;
  VisitSetDifferencesAndIntersection(
      set1.begin(), set1.end(), set2.begin(), set2.end(),
      /*f1=*/[&only_in_one](int i) { only_in_one.push_back(i); },
      /*f2=*/[&only_in_two](int i) { only_in_two.push_back(i); },
      /*f_both=*/[](int i) { FAIL() << "Must not visit in both"; });
  EXPECT_THAT(only_in_one, ElementsAre(1, 2, 3));
  EXPECT_THAT(only_in_two, ElementsAre(4, 5, 6));
}

TEST(VisitSetDifferencesAndIntersectionTest, DistinctEven) {
  std::set<int> set1 = {1, 3, 5};
  std::set<int> set2 = {2, 4, 6};

  std::vector<int> all;
  std::vector<int> only_in_one;
  std::vector<int> only_in_two;
  VisitSetDifferencesAndIntersection(
      set1.begin(), set1.end(), set2.begin(), set2.end(),
      /*f1=*/
      [&only_in_one, &all](int i) {
        only_in_one.push_back(i);
        all.push_back(i);
      },
      /*f2=*/
      [&only_in_two, &all](int i) {
        only_in_two.push_back(i);
        all.push_back(i);
      },
      /*f_both=*/[](int i) { FAIL() << "Must not visit in both"; });
  EXPECT_THAT(only_in_one, ElementsAre(1, 3, 5));
  EXPECT_THAT(only_in_two, ElementsAre(2, 4, 6));
  EXPECT_THAT(all, ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(VisitSetDifferencesAndIntersectionTest, Overlap) {
  std::set<int> set1 = {1, 2, 3};
  std::set<int> set2 = {2, 3, 4};

  std::vector<int> all;
  std::vector<int> only_in_one;
  std::vector<int> only_in_two;
  std::vector<int> in_both;
  VisitSetDifferencesAndIntersection(
      set1.begin(), set1.end(), set2.begin(), set2.end(),
      /*f1=*/
      [&only_in_one, &all](int i) {
        only_in_one.push_back(i);
        all.push_back(i);
      },
      /*f2=*/
      [&only_in_two, &all](int i) {
        only_in_two.push_back(i);
        all.push_back(i);
      },
      /*f_both=*/
      [&in_both, &all](int i) {
        in_both.push_back(i);
        all.push_back(i);
      });
  EXPECT_THAT(only_in_one, ElementsAre(1));
  EXPECT_THAT(only_in_two, ElementsAre(4));
  EXPECT_THAT(in_both, ElementsAre(2, 3));
  EXPECT_THAT(all, ElementsAre(1, 2, 3, 4));
}

TEST(VisitSetDifferencesAndIntersectionTest, Overlap2) {
  std::set<int> set1 = {1, 2, 5, 6};
  std::set<int> set2 = {2, 3, 4, 5};

  std::vector<int> all;
  std::vector<int> only_in_one;
  std::vector<int> only_in_two;
  std::vector<int> in_both;
  VisitSetDifferencesAndIntersection(
      set1.begin(), set1.end(), set2.begin(), set2.end(),
      /*f1=*/
      [&only_in_one, &all](int i) {
        only_in_one.push_back(i);
        all.push_back(i);
      },
      /*f2=*/
      [&only_in_two, &all](int i) {
        only_in_two.push_back(i);
        all.push_back(i);
      },
      /*f_both=*/
      [&in_both, &all](int i) {
        in_both.push_back(i);
        all.push_back(i);
      });
  EXPECT_THAT(only_in_one, ElementsAre(1, 6));
  EXPECT_THAT(only_in_two, ElementsAre(3, 4));
  EXPECT_THAT(in_both, ElementsAre(2, 5));
  EXPECT_THAT(all, ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(VisitSetDifferencesAndIntersectionTest, Union) {
  std::set<int> set1 = {1, 2, 5, 6};
  std::set<int> set2 = {2, 3, 4, 5};

  std::vector<int> all;
  auto all_lambda = [&all](int i) { all.push_back(i); };
  VisitSetDifferencesAndIntersection(set1.begin(), set1.end(), set2.begin(),
                                     set2.end(),
                                     /*f1=*/all_lambda,
                                     /*f2=*/all_lambda,
                                     /*f_both=*/all_lambda);
  EXPECT_THAT(all, ElementsAre(1, 2, 3, 4, 5, 6));
}

TEST(VisitSetDifferencesAndIntersectionTest, LambdaCapturedByReference) {
  std::set<int> set1 = {1, 2, 5, 6};
  std::set<int> set2 = {2, 3, 4, 5};

  std::vector<int> all;
  auto all_lambda = [&all, call_count = 0](int i) mutable {
    all.push_back(i);
    return ++call_count;
  };
  VisitSetDifferencesAndIntersection(set1.begin(), set1.end(), set2.begin(),
                                     set2.end(),
                                     /*f1=*/std::ref(all_lambda),
                                     /*f2=*/std::ref(all_lambda),
                                     /*f_both=*/std::ref(all_lambda));
  EXPECT_THAT(all, ElementsAre(1, 2, 3, 4, 5, 6));
  // Call lambda again to extract call_count.
  int out_call_count = all_lambda(0xDEAD);
  EXPECT_THAT(out_call_count, Eq(7));
}

}  // namespace
}  // namespace deepmind::lab2d
