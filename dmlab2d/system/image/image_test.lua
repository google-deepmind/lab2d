--[[ Copyright (C) 2018-2019 The DMLab2D Authors.

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

local image = require 'system.image'
local file_system = require 'system.file_system'
local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local tensor = require 'system.tensor'
local paths = require 'common.paths'

local TEST_DIR = test_runner.testDir(...)

local tests = {}

function tests:loadImageRGB()
  local image = image.load(TEST_DIR .. "testRGB.png")
  local expected = tensor.ByteTensor(32, 32 * 3, 3)
  expected:narrow(2, 1, 32):select(3, 1):fill(255)
  expected:narrow(2, 33, 32):select(3, 2):fill(255)
  expected:narrow(2, 65, 32):select(3, 3):fill(255)
  asserts.EQ(expected, image)
end

function tests:loadImageRGBA()
  local image = image.load(TEST_DIR .. "testRGBA.png")
  local expected = tensor.ByteTensor(32 * 2, 32 * 3, 4)
  expected:narrow(2, 1, 32):select(3, 1):fill(255)
  expected:narrow(2, 33, 32):select(3, 2):fill(255)
  expected:narrow(2, 65, 32):select(3, 3):fill(255)
  expected:narrow(1, 1, 32):select(3, 4):fill(255)
  expected:narrow(1, 33, 32):select(3, 4):fill(127)
  asserts.EQ(expected, image)
end

function tests:loadImageL()
  local image = image.load(TEST_DIR .. "testL.png")
  local expected = tensor.ByteTensor(32, 32, 1)
  expected:applyIndexed(function (val, index)
      return index[1] + index[2] - 2
    end)
  asserts.EQ(expected, image)
end

function tests:loadImageLFromString()
  local filename = TEST_DIR .. "testL.png"
  local contents = file_system:loadFileToString(filename)
  local image = image.load('content:.png', contents)
  local expected = tensor.ByteTensor(32, 32, 1)
  expected:applyIndexed(function (val, index)
      return index[1] + index[2] - 2
    end)
  asserts.EQ(expected, image)
end

return test_runner.run(tests)
