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

#ifndef DMLAB2D_SYSTEM_GRID_WORLD_HANDLES_H_
#define DMLAB2D_SYSTEM_GRID_WORLD_HANDLES_H_

#include "dmlab2d/system/grid_world/collections/handle.h"

namespace deepmind::lab2d {

// Handle assoiciated with a layer within the grid_world. To pieces may not
// share the same location and layer.
struct LayerTag {
  static constexpr char kName[] = "Layer";
};
using Layer = Handle<LayerTag>;

// Handle associated with a grid piece in the scene.
struct PieceTag {
  static constexpr char kName[] = "Piece";
};
using Piece = Handle<PieceTag>;

// Handle associated with piece state information.
struct StateTag {
  static constexpr char kName[] = "State";
};
using State = Handle<StateTag>;

// Handle associated with a sprite name in the grid world. Not to be confused
// with SpriteIds.
struct SpriteTag {
  static constexpr char kName[] = "Sprite";
};
using Sprite = Handle<SpriteTag>;

// Handle to a cell index in grid.
struct CellIndexTag {
  static constexpr char kName[] = "CellIndex";
};
using CellIndex = Handle<CellIndexTag>;

// Handle to a group of pieces belonging to a set of states.
struct GroupTag {
  static constexpr char kName[] = "Group";
};
using Group = Handle<GroupTag>;

struct ContactTag {
  static constexpr char kName[] = "Contact";
};
using Contact = Handle<ContactTag>;

struct HitTag {
  static constexpr char kName[] = "Hit";
};
using Hit = Handle<HitTag>;

struct UpdateTag {
  static constexpr char kName[] = "Update";
};
using Update = Handle<UpdateTag>;

}  // namespace deepmind::lab2d

#endif  // DMLAB2D_SYSTEM_GRID_WORLD_HANDLES_H_
