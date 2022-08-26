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

#ifndef DMLAB2D_LIB_SYSTEM_GRID_WORLD_GRID_SHAPE_H_
#define DMLAB2D_LIB_SYSTEM_GRID_WORLD_GRID_SHAPE_H_

#include "absl/log/log.h"
#include "dmlab2d/lib/system/grid_world/handles.h"
#include "dmlab2d/lib/system/math/math2d.h"

namespace deepmind::lab2d {

// Stores the shape of a 2D grid with layers.
class GridShape {
 public:
  enum class Topology { kBounded, kTorus };
  constexpr GridShape(math::Size2d grid_size_2d, int layer_count,
                      Topology topology)
      : grid_size_2d_(grid_size_2d),
        layer_count_(layer_count),
        topology_(topology) {}

  // Returns whether `position` is within the bounds of the grid.
  constexpr bool InBounds(math::Position2d position) const {
    return (topology_ == Topology::kTorus) || grid_size_2d_.Contains(position);
  }

  constexpr int ModuloWidth(int x) const {
    return PositiveModulo(x, grid_size_2d_.width);
  }

  constexpr int ModuloHeight(int y) const {
    return PositiveModulo(y, grid_size_2d_.height);
  }

  constexpr math::Position2d Normalised(math::Position2d position) const {
    if (topology_ == Topology::kTorus) {
      return {ModuloWidth(position.x), ModuloHeight(position.y)};
    } else {
      return position;
    }
  }

  // Return a smallest vector between start and end. If the distance is the same
  // in two directions it will favour the negative one.
  math::Vector2d SmallestVector(math::Position2d start,
                                math::Position2d end) const {
    switch (topology_) {
      case Topology::kBounded:
        return end - start;
      case Topology::kTorus: {
        math::Vector2d diff = end - start;
        int hwidth = grid_size_2d_.width / 2;
        int hheight = grid_size_2d_.height / 2;
        diff.x = ModuloWidth(diff.x + hwidth) - hwidth;
        diff.y = ModuloHeight(diff.y + hheight) - hheight;
        return diff;
      }
    }
    LOG(FATAL) << "Invalid Topology! " << static_cast<int>(topology_);
  }

  // Returns cell-index corresponding to the `position` and `layer`.
  CellIndex ToCellIndex(math::Position2d position, Layer layer) const {
    if (topology_ == Topology::kTorus) {
      position.x = ModuloWidth(position.x);
      position.y = ModuloHeight(position.y);
    }
    return CellIndex((position.y * grid_size_2d_.width + position.x) *
                         layer_count() +
                     layer.Value());
  }

  // If position is in bounds and layer is valid, it returns a non-empty
  // cell-index pointing to a valid cell. Otherwise returns the empty
  // cell-index.
  CellIndex TryToCellIndex(math::Position2d position, Layer layer) const {
    return (InBounds(position) &&          //
            !layer.IsEmpty() &&            //
            layer.Value() < layer_count_)  //
               ? ToCellIndex(position, layer)
               : CellIndex();
  }

  // Returns the number of cells in the grid.
  constexpr int GetCellCount() const {
    return grid_size_2d_.Area() * layer_count_;
  }

  // Returns the width and height of the grid.
  constexpr math::Size2d GridSize2d() const { return grid_size_2d_; }

  // Returns the number of layers in the grid.
  constexpr int layer_count() const { return layer_count_; }

  // Returns whether the grid is a torus.
  constexpr Topology topology() const { return topology_; }

 private:
  // Returns the remainder in the range [0, divisor]. The `divisor` must be
  // positive.
  static constexpr int PositiveModulo(int value, int divisor) {
    int output = value % divisor;
    if (output < 0) {
      output += divisor;
    }
    return output;
  }

  const math::Size2d grid_size_2d_;
  const int layer_count_;
  const Topology topology_;
};

}  // namespace deepmind::lab2d
#endif  // DMLAB2D_LIB_SYSTEM_GRID_WORLD_GRID_SHAPE_H_
