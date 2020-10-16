--[[ Copyright (C) 2018 The DMLab2D Authors.

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

--[[ Useful for running Lua tests in DM Lab2D.

In the build file:

```
cc_test(
    name = "test_script",
    args = ["lua_tests/test_script.lua"],
    data = [":test_script.lua"],
    deps = ["/labyrinth/testing:lua_unit_test_lib"],
)
```

In test_script.lua:

```
local tests = {}

function tests.testAssert()
  assert(true)
end

return test_runner.run(tests)
```
]]
local tables = require 'common.tables'

local test_runner = {}
local ESCAPE = string.char(27)
local GREEN = ESCAPE .. '[0;32m'
local ORANGE = ESCAPE .. '[0;33m'
local RED = ESCAPE .. '[0;31m'
local CLEAR = ESCAPE .. '[0;0m'

local FAILED = RED .. '[  FAILED  ]' .. CLEAR .. ' tests.'
local PASSED = GREEN .. '[  PASSED  ]' .. CLEAR .. ' tests.'

function test_runner.run(tests)
  local function runAllTests()
    local statusAll = true
    local errors = ''
    for name, test in tables.pairsByKeys(tests) do
      local status, msg = xpcall(test, debug.traceback)
      if not status then
        print(FAILED .. name)
        errors = errors .. 'function tests.' .. name .. '\n' .. msg .. '\n'
        statusAll = false
      else
        print(PASSED .. name)
      end
    end
    return (statusAll and 0 or 1), errors
  end
  local api = {init = runAllTests}
  return api
end

--[[ Finds test directory relative to test file.

Test directory should have the same name as the test file but with a suffix
'_data'. If your test file is 'image_test.lua' the data must be in a directory
named 'image_test_data'.

```Lua
local TEST_DIR = test_runner.testDir(...)
```
]]
function test_runner.testDir(filename)
  assert(filename:sub(#filename - 8) == '_test.lua',
         'Filename must have suffix "_test.lua"! Actual: ' .. filename)
  return filename:sub(1, #filename - 4) .. '_data/'
end

return test_runner
