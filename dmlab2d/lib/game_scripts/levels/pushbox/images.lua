--[[ Copyright (C) 2020 The DMLab2D Authors.

Licensed under the Apache License, Version 2.0 (the 'License');
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an 'AS IS' BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
]]

local PLAYER_TEXT = [[
..@@@@..
..@@@@..
..****..
.@*##*@.
.******.
.******.
..*..*..
..*..*..
]]


local WALL_TEXT = [[
...#....
...#....
...#....
########
.......#
.......#
.......#
########
]]

local GOAL = [[
########
#......#
#......#
#..##..#
#..##..#
#......#
#......#
########
]]

local BOX = [[
########
##....##
#.#..#.#
#..##..#
#..##..#
#.#..#.#
##....##
########
]]

return {
    player = function()
      return {
          palette = {
              ['*'] = {0, 0, 255, 255},  -- One pixel
              ['@'] = {0, 0, 127, 255},
              ['#'] = {255, 255, 255, 255},
              ['.'] = {0, 0, 0, 0},
            },
          text = PLAYER_TEXT,
          noRotate = true,
      }
    end,
    wall = function()
      return {
          palette = {
              ['.'] = {80, 25, 33},
              ['#'] = {153, 150, 150},
              ['*'] = {128, 128, 128}, -- One pixel
            },
          text = WALL_TEXT,
          noRotate = true,
        }
    end,
    goal = function()
      return {
          palette = {
              ['#'] = {255, 0, 0, 255},
              ['*'] = {64, 64, 64, 160}, -- One pixel
              ['.'] = {0, 0, 0, 0},
            },
          text = GOAL,
          noRotate = true,
        }
    end,
    box = function()
      return {
          palette = {
              ['.'] = {233, 157, 56},
              ['#'] = {180, 102, 23},
              ['*'] = {64, 255, 128}, -- One pixel
          },
          text = BOX,
          noRotate = true,
        }
    end
}
