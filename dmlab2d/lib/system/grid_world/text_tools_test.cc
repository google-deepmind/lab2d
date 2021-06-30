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

#include "dmlab2d/lib/system/grid_world/text_tools.h"

#include "gmock/gmock.h"
#include "gtest/gtest.h"

namespace deepmind::lab2d {
namespace {
using testing::Eq;

TEST(CharMap, Works) {
  CharMap map;
  EXPECT_THAT(map['A'], Eq(State()));
  map['A'] = State(4);
  EXPECT_THAT(map['A'], Eq(State(4)));
  for (int i = 0; i < 26; ++i) {
    map['A' + i] = State(i);
    map['a' + i] = State(i);
  }
  EXPECT_THAT(map['Z'], Eq(map['z']));
}

TEST(RemoveLeadingAndTrailingNewLines, NoopWorks) {
  constexpr absl::string_view before = "aaa\naaa\naaa";
  constexpr absl::string_view after = "aaa\naaa\naaa";
  EXPECT_THAT(RemoveLeadingAndTrailingNewLines(before), Eq(after));
}

TEST(RemoveLeadingAndTrailingNewLines, TrailingWorks) {
  constexpr absl::string_view before =
      "aaa\naaa\naaa"
      "\n\n";
  constexpr absl::string_view after = "aaa\naaa\naaa";
  EXPECT_THAT(RemoveLeadingAndTrailingNewLines(before), Eq(after));
}

TEST(RemoveLeadingAndTrailingNewLines, PrefixWorks) {
  constexpr absl::string_view before =
      "\n\n\n"
      "aaa\naaa\naaa";
  constexpr absl::string_view after = "aaa\naaa\naaa";
  EXPECT_THAT(RemoveLeadingAndTrailingNewLines(before), Eq(after));
}

TEST(RemoveLeadingAndTrailingNewLines, BothWorks) {
  constexpr absl::string_view before =
      "\n\n\n"
      "aaa\naaa\naaa"
      "\n\n";
  constexpr absl::string_view after = "aaa\naaa\naaa";
  EXPECT_THAT(RemoveLeadingAndTrailingNewLines(before), Eq(after));
}

TEST(RemoveLeadingAndTrailingNewLines, EmptyWorks) {
  EXPECT_THAT(RemoveLeadingAndTrailingNewLines(""), Eq(""));
}

constexpr absl::string_view kSquare43 = R"(
1234
2...
3...
)";

TEST(GetSize2dOfText, SquareWorks) {
  EXPECT_THAT(GetSize2dOfText(kSquare43), Eq(math::Size2d{4, 3}));
}

constexpr absl::string_view kJagged43 = R"(
0
1234
3.
)";

TEST(GetSize2dOfText, JaggedWorks) {
  EXPECT_THAT(GetSize2dOfText(kJagged43), Eq(math::Size2d{4, 3}));
}

constexpr absl::string_view kEmptyLines43 = R"(
1234

3
)";

TEST(GetSize2dOfText, EmptyLinesWorks) {
  EXPECT_THAT(GetSize2dOfText(kEmptyLines43), Eq(math::Size2d{4, 3}));
}

TEST(GetSize2dOfText, EmptyWorks) {
  EXPECT_THAT(GetSize2dOfText(""), Eq(math::Size2d{0, 0}));
}

}  // namespace
}  // namespace deepmind::lab2d
