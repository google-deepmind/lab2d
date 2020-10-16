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

local tile = require 'system.tile'
local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'

local tests = {}

function tests:renderScene()
  local tensor = require 'system.tensor'
  local tile = require 'system.tile'
  local set = tile.Set{names = {'red', 'green', 'blue'},
                       shape = {height = 1, width = 2}}

  local image = tensor.ByteTensor(1, 2, 3)
  set:setSprite{name = 'red', image = image:fill{255, 0, 0}}
  set:setSprite{name = 'green', image = image:fill{0, 255, 0}}
  set:setSprite{name = 'blue', image = image:fill{0, 0, 255}}
  local scene = tile.Scene{shape = {width = 3, height = 1}, set = set}
  local grid = tensor.Int32Tensor{range = {0, 2}}:reshape{1, 3}
  asserts.EQ(scene:render(grid),
             tensor.ByteTensor{{{255, 0, 0},
                                {255, 0, 0},
                                {0, 255, 0},
                                {0, 255, 0},
                                {0, 0, 255},
                                {0, 0, 255}}})
end

return test_runner.run(tests)
