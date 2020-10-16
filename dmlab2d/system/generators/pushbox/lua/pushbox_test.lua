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

local pushbox = require 'system.generators.pushbox'
local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'

local tests = {}

function tests.generateLevel()
  local layout = pushbox.generate{
      seed = 10,
      width = 14,
      height = 11,
      numBoxes = 5,
  }
  local height, i = 0, 0
  local counters = {
    ['&'] = 0,
    ['X'] = 0,
    ['B'] = 0,
    ['P'] = 0,
  }
  for i = 1, #layout do
    local c = layout:sub(i, i)
    local count = counters[c] or 0
    counters[c] = count + 1
  end
  asserts.EQ(layout:find("\n") - 1, 14)
  asserts.EQ(11, counters['\n'] + 1, 11)

  -- Test goal count
  asserts.EQ(counters['X'] + counters['&'], 5)
  -- Test box count
  asserts.EQ(counters['B'] + counters['&'], 5)
  -- Test spawn point count
  asserts.EQ(counters['P'], 1)
end

function tests.badInputs()
  asserts.shouldFail(
      function()
        pushbox.Generate{
            seed = 10,
            height = 5,
            width = 5,
            num_boxes = 36
        }
      end)
end


return test_runner.run(tests)
