--[[ Copyright (C) 2017-2020 The DMLab2D Authors.

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

local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'

local paths = require 'common.paths'

local tests = {}

local MY_FILE_PATH = ...

function tests.dirnameWorks()
  asserts.EQ(paths.dirname('file'), '')
  asserts.EQ(paths.dirname('/path2'), '/')
  asserts.EQ(paths.dirname('path1/path2'), 'path1/')
  asserts.EQ(paths.dirname('path1/path2/'), 'path1/path2/')
  asserts.EQ(paths.dirname('/path1/path2'), '/path1/')
  asserts.EQ(paths.dirname('/path1/path2/'), '/path1/path2/')
  asserts.EQ(paths.dirname('~/path1/path2/'), '~/path1/path2/')
  asserts.EQ(paths.dirname('~/path1/path2'), '~/path1/')
end

function tests.fileExistsWorks()
  assert(paths.fileExists(MY_FILE_PATH), 'This file does not exist?')
  assert(not paths.fileExists(MY_FILE_PATH .. 'Missing'), 'This file exists?')
end

function tests.joinWorks()
  asserts.EQ(paths.join('base', 'path'), 'base/path')
  asserts.EQ(paths.join('base/', 'path'), 'base/path')
  asserts.EQ(paths.join('base', '/path'), '/path')
  asserts.EQ(paths.join('base/', '/path'), '/path')
  asserts.EQ(paths.join('a', 'b', 'c', 'd'), 'a/b/c/d')
end

return test_runner.run(tests)
