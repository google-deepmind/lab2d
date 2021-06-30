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

--[[ Asserts library.

All the following examples assume the following header.

```Lua
local asserts = require 'testing.asserts'
local args = require 'common.args'
```

## `asserts.that(target, matcher, msg)`

`matcher` must be a constraint from common.args library. Throws error if the
constraint does not match `target`. This is quite powerful and can be used
instead of most of the following assertion.

```Lua
asserts.that(5, args.all(args.gt(4), args.lt(10)))
asserts.shouldFail(asserts.that(5, args.all(args.gt(1), args.lt(4))))
```

## `asserts.EQ(target, expect, msg)`

Throws an error if *not* `target == expect`.

```Lua
asserts.EQ(5, 5)
asserts.shouldFail(asserts.EQ(5, 4))
```

## `asserts.NE(target, expect, msg)`

Throws an error if *not* `target ~= expect`.

```Lua
asserts.NE(5, 4)
asserts.shouldFail(asserts.NE(5, 5))
```

## `asserts.GT(target, expect, msg)`

Throws an error if *not* `target > expect`.

```Lua
asserts.GT(5, 4)
asserts.shouldFail(asserts.GT(5, 6))
```

## `asserts.GE(target, expect, msg)`

Throws an error if *not* `target >= expect`.

```Lua
asserts.GE(5, 5)
asserts.shouldFail(asserts.GE(5, 6))
```

## `asserts.LT(target, expect, msg)`

Throws an error if *not* `target < expect`.

```Lua
asserts.LT(4, 5)
asserts.shouldFail(asserts.LT(5, 4))
```

## `asserts.LE(target, expect, msg)`

Throws an error if *not* `target <= expect`.

```Lua
asserts.LE(3, 4)
asserts.shouldFail(asserts.LE(5, 3))
```

## `asserts.hasSubstr(source, substr, msg)`

Throws an error if `source` *does not* contain `substr` as a substring.

```Lua
asserts.hasSubstr('My string is here', 'is her')
asserts.shouldFail(
    function() asserts.hasSubstr('My string is here', 'not there') end)
```

## `asserts.tablesEQ(target, expect, msg)`

Throws an error if values of nested table `target` *do not* match `expected`.

```Lua
asserts.tablesEQ({5, a = {b = 10}}, {5, a = {b = 8}})
asserts.shouldFail(asserts.tablesEQ({5, a = {b = 8}}, {5, a = {b = 8, c = 8}}))
```

## `asserts.tablesNE(target, expect, msg)`

Throws an error if values of nested table `target` *do* match `expected`.

```Lua
asserts.tablesNE({5, a = {b = 10}}, {5, a = {b = 8, c = 8}})
asserts.shouldFail(asserts.tablesNE({5, a = {b = 8}}, {5, a = {b = 8}}))
```

## `asserts.shouldFail(fn, substrings...)`

Throws an error if calling fn *does not* raise an error. And if `substrings` are
specified that error must contain each substring in its entirety.
]]

local args = require 'common.args'
local tables = require 'common.tables'
local asserts = {}

local function fail(message, optional)
  optional = optional and (' ' .. optional) or ''
  error(message .. optional, 3)
end

function asserts.that(target, matcher, msg)
  local checker = args.makeCheck(matcher)
  if not checker.check(target) then
    fail(checker.err(target), msg)
  end
end

local function areTablesEqual(target, expect)
  local targetSize = 0
  for k, v in pairs(target) do
    targetSize = targetSize + 1
  end
  for k, v in pairs(expect) do
    if type(v) == 'table' then
      if not areTablesEqual(target[k], v) then
        return false
      end
    elseif target[k] ~= v then
      return false
    end
    targetSize = targetSize - 1
  end
  if targetSize ~= 0 then
    -- There were values in target not present in expect
    return false
  end
  return true
end

function asserts.EQ(target, expect, msg)
  if target ~= expect then
    fail('Expected: ' .. tostring(expect) .. '. ' ..
         'Actual: ' .. tostring(target) .. '.', msg)
  end
end

function asserts.NE(target, expect, msg)
  if target == expect then
    fail('Expected values to differ: ' .. tostring(expect), msg)
  end
end

-- Assert target > expect
function asserts.GT(target, expect, msg)
  if target <= expect then
    fail('Expected: ' .. tostring(target) .. ' > ' .. tostring(expect), msg)
  end
end

-- Assert target >= expect
function asserts.GE(target, expect, msg)
  if target < expect then
    fail('Expected: ' .. tostring(target) .. ' >= ' .. tostring(expect), msg)
  end
end

-- Assert target < expect
function asserts.LT(target, expect, msg)
  if target >= expect then
    fail('Expected: ' .. tostring(target) .. ' < ' .. tostring(expect), msg)
  end
end

-- Assert target <= expect
function asserts.LE(target, expect, msg)
  if target > expect then
    fail('Expected: ' .. tostring(target) .. ' <= ' .. tostring(expect), msg)
  end
end

-- Assert that a 'source' string contains a specific 'substr' substring.  Both
-- arguments must be strings.
function asserts.hasSubstr(source, substr, msg)
  if type(source) ~= 'string' then fail('1st argument should be a string') end
  if type(substr) ~= 'string' then fail('2nd argument should be a string') end
  if source:find(substr, nil, true) == nil then
    fail(string.format('String "%q" does not contain "%q".', source, substr),
         msg)
  end
end

function asserts.tablesEQ(target, expect, msg)
  if type(target) ~= 'table' then fail('1st argument should be a table') end
  if type(expect) ~= 'table' then fail('2nd argument should be a table') end
  if not areTablesEqual(target, expect) then
    fail('Expected: ' .. tables.tostringOneLine(expect) .. '. ' ..
         'Actual: ' .. tables.tostringOneLine(target) .. '.', msg)
  end
end

function asserts.tablesNE(target, expect, msg)
  if type(target) ~= 'table' then fail('1st argument should be a table') end
  if type(expect) ~= 'table' then fail('2nd argument should be a table') end
  if areTablesEqual(target, expect) then
    fail("Expected tables with different values.", msg)
  end
end

function asserts.shouldFail(fn, ...)
  local status, out = pcall(fn)
  if status then
    fail("Expected an error but call succeeded.")
  end
  if select('#', ...) > 0 then
    local message = ...
    if type(message) ~= 'table' then message = {...} end
    for _, expected in ipairs(message) do
      if not out:find(expected, nil, true) then
        fail('Expected "' .. expected .. '" to appear in "' .. out .. '"')
      end
    end
  end
end

asserts.shouldFailWithMessage = asserts.shouldFail

return asserts
