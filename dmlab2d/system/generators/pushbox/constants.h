// Copyright (C) 2020 The DMLab2D Authors.
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

#ifndef DMLAB2D_SYSTEM_GENERATORS_PUSHBOX_CONSTANTS_H_
#define DMLAB2D_SYSTEM_GENERATORS_PUSHBOX_CONSTANTS_H_

namespace deepmind::lab2d::pushbox {

namespace room {

// Maximum retries for randomly placing the boxes and player and for
// generating the room.
static constexpr int kMaxGenerationStepRetries = 500;
static constexpr int kMaxTargetPlacementRetries = 100;
static constexpr int kMaxPlayerPlacementRetries = 50;

// Default margin to be kept as walls on each side of the room.
static constexpr int kDefaultWallMargin = 1;

static constexpr char kPlayerChar = 'P';
static constexpr char kBoxChar = 'B';
static constexpr char kTagetChar = 'X';
static constexpr char kBoxTagetChar = '&';
static constexpr char kWallChar = '*';
static constexpr char kFloorChar = ' ';

// Seed for board-state hash.
static constexpr int kZobristSeed = 4;

}  // namespace room

namespace generator {

// Maximum depth in the room state search tree, i.e. the maximum length
// allowed for the sequence of actions applied from the initial state.
static constexpr int kMaxAppliedActions = 300;

// Maximum number of room topologies to try.
static constexpr int kMaxRoomTopologies = 10;

// Maximum number of room configurations to explore per generation trial.
static constexpr int kMaxRoomConfigurations = 1000;

// Maximum number of box/player initial configurations we are going to try per
// room topology.
static constexpr int kMaxPositions = 10;

// The default probability for direction change when generating the room.
static constexpr float kDirectionChangeRatio = 0.35;

// Maximum room width or height.
static constexpr int kMaxRoomSize = 20;

// Minimum steps to take when generating the room.
static constexpr int kMinSteps = 5;

// Minimum amount of boxes in the room.
static constexpr int kMinBoxes = 1;

// Search hash_map starting memory.
static constexpr int kVisitedRoomsBucketCount = 1e7;

}  // namespace generator

}  // namespace deepmind::lab2d::pushbox

#endif  // DMLAB2D_SYSTEM_GENERATORS_PUSHBOX_CONSTANTS_H_
