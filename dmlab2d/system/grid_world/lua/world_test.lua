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

local grid_world = require 'system.grid_world'
local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'

local tests = {}

function tests:createWorld()
  local grid_world = require 'system.grid_world'
  grid_world.World{
      renderOrder = {'layer2', 'layer0'},
      types = {
          type0 = {
              layer = 'layer0',
              sprite = 'sprite0'
          },
          type1 = {
              layer = 'layer1',
              sprite = 'sprite1'
          },
          type2 = {
              layer = 'layer2',
              sprite = 'sprite2'
          }
      }
  }
end

return test_runner.run(tests)
