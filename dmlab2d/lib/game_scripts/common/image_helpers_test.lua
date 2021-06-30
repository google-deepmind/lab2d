--[[ Copyright (C) 2019 The DMLab2D Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
]]

local image_helpers = require 'common.image_helpers'
local tensor = require 'system.tensor'
local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'

local tests = {}

function tests:generateRotationsWorks()
  local sprite = tensor.ByteTensor{range = {1, 25}}:reshape{5, 5}
  local rotations = image_helpers.generateRotations(sprite)
  asserts.tablesEQ(rotations:shape(), {4, 5, 5})
  asserts.EQ(sprite, rotations(1))
  asserts.EQ(rotations(1),
             tensor.ByteTensor{{1, 2, 3, 4, 5},
                               {6, 7, 8, 9, 10},
                               {11, 12, 13, 14, 15},
                               {16, 17, 18, 19, 20},
                               {21, 22, 23, 24, 25}})

  asserts.EQ(rotations(2),
             tensor.ByteTensor{{21, 16, 11, 6, 1},
                               {22, 17, 12, 7, 2},
                               {23, 18, 13, 8, 3},
                               {24, 19, 14, 9, 4},
                               {25, 20, 15, 10, 5}})

  asserts.EQ(rotations(3),
             tensor.ByteTensor{{25, 24, 23, 22, 21},
                               {20, 19, 18, 17, 16},
                               {15, 14, 13, 12, 11},
                               {10, 9, 8, 7, 6},
                               {5, 4, 3, 2, 1}})

  asserts.EQ(rotations(4),
             tensor.ByteTensor{{5, 10, 15, 20, 25},
                               {4, 9, 14, 19, 24},
                               {3, 8, 13, 18, 23},
                               {2, 7, 12, 17, 22},
                               {1, 6, 11, 16, 21}})
end

function tests:colorToSpriteWorks()
  local sprite = image_helpers.colorToSprite(
      {height = 3, width = 2}, {1, 2, 3, 4})
  asserts.tablesEQ(sprite:shape(), {3, 2, 4})
  asserts.EQ(sprite,
             tensor.ByteTensor{{{1, 2, 3, 4}, {1, 2, 3, 4}},
                               {{1, 2, 3, 4}, {1, 2, 3, 4}},
                               {{1, 2, 3, 4}, {1, 2, 3, 4}}})
end

function tests:textToSpriteWorks()
  local text = [[
      aaaaa
      ..b..
      ccccc
      .....
  ]]
  local palette = {a = {1, 0, 0}, b = {0, 0, 2}, c = {0, 3, 0}}
  local sprite, shape = image_helpers.textToSprite(text, palette)
  asserts.tablesEQ(sprite:shape(), {4, 5, 3})
  asserts.tablesEQ(shape, {height = 4, width = 5})
  asserts.EQ(sprite, tensor.ByteTensor{
      {{1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}},
      {{0, 0, 0}, {0, 0, 0}, {0, 0, 2}, {0, 0, 0}, {0, 0, 0}},
      {{0, 3, 0}, {0, 3, 0}, {0, 3, 0}, {0, 3, 0}, {0, 3, 0}},
      {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}})
end

return test_runner.run(tests)
