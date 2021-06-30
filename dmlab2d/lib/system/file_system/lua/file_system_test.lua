--[[ Copyright (C) 2018-2020 The DMLab2D Authors.

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

local file_system = require 'system.file_system'
local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local paths = require 'common.paths'

local FILE_NAME = ...
local TEST_DIR = test_runner.testDir(FILE_NAME)
local EMPTY_FILE_NAME = TEST_DIR .. "empty_test_file"
local TEXT_FILE_NAME = TEST_DIR .. "file_with_text.txt"

local tests = {}

local function readFile(file)
  local f = io.open(file, "rb")
  local content = f:read("*all")
  f:close()
  return content
end

function tests:readEmpty()
  local emptyFileContent = file_system:loadFileToString(EMPTY_FILE_NAME)
  asserts.EQ(emptyFileContent, '')
end

function tests:readBinaryFile()
  local textFileContent = file_system:loadFileToString(TEXT_FILE_NAME)
  asserts.EQ(textFileContent, readFile(TEXT_FILE_NAME))
end

function tests:readSelf()
  local textFileContent = file_system:loadFileToString(FILE_NAME)
  asserts.EQ(textFileContent, readFile(FILE_NAME))
end

return test_runner.run(tests)
