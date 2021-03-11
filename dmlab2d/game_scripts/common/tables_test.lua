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

local tables = require 'common.tables'

local tests = {}

local function toKeyArrays(table)
  if table == nil then return {} end
  local arr = {}
  for k, v in pairs(table) do
    arr[#arr + 1] = k
  end
  return arr
end

local function shallowEqual(tableA, tableB)
  if tableA == nil and tableB == nil then return true end

  -- Ensure tables don't reference the same object.
  if tableA == tableB then return false end

  -- Convert to arrays
  local a = toKeyArrays(tableA)
  local b = toKeyArrays(tableB)
  if #a ~= #b then return false end

  -- Check for simple equality between key values.
  for _, k in ipairs(a) do
    if tableA[k] ~= tableB[k] then return false end
  end

  return true
end

local function assertShallowCopyIsEqual(orig)
  local copy = tables.shallowCopy(orig)
  assert(shallowEqual(orig, copy))
end

function tests.shallowCopyNil()
  assertShallowCopyIsEqual(nil)
end

function tests.shallowCopyEmptyTable()
  assertShallowCopyIsEqual({})
end

function tests.shallowCopyOneItemArray()
  assertShallowCopyIsEqual({'hello'})
end

function tests.shallowCopyFlatArray()
  assertShallowCopyIsEqual({'hello', 7, 'swha!'})
end

function tests.shallowCopyOneItemTable()
  assertShallowCopyIsEqual({myKey = 'hello'})
end

function tests.shallowCopyFlatTable()
  assertShallowCopyIsEqual({
      ['myKey'] = 'hello',
      [7] = 'there',
      ['thinking'] = 1
  })
end

function tests.shallowCopyParentChildTable()
  local leaf = {myKey = 'hello'}
  assertShallowCopyIsEqual({root = leaf})
end

function tests.shallowCopyComplexKeyTable()
  local key = {myKey = 'hello'}
  assertShallowCopyIsEqual({[key] = 'yo'})
end

function tests.shallowCopyBigTable()
  local key = {myKey = 'hello'}
  local leaf = {fancy = 'string'}
  local mid = {midKey = leaf}

  assertShallowCopyIsEqual({
      'try',
      'something',
      'here',
      [key] = leaf,
      anotherKey = mid
  })
end

local function deepEqual(tableA, tableB)
  if tableA == nil and tableB == nil then return true end

  -- Ensure tables don't reference the same object.
  if tableA == tableB then return false end

  -- Convert to arrays
  local a = toKeyArrays(tableA)
  local b = toKeyArrays(tableB)
  if #a ~= #b then return false end

  for _, k in ipairs(a) do
    -- Check for simple equality between keys
    local va = tableA[k]
    local vb = tableB[k]
    if type(va) ~= type(vb) then return false end
    if type(va) == 'table' then
      -- Check for deep equality between table values
      if not deepEqual(va, vb) then return false end
    else
      -- Check for simple equality between simple values
      if va ~= vb then return false end
    end
  end

  return true
end

local function assertDeepCopyIsEqual(orig)
  local copy = tables.deepCopy(orig)
  assert(deepEqual(orig, copy))
end

function tests.deepCopyNil()
  assertDeepCopyIsEqual(nil)
end

function tests.deepCopyEmptyTable()
  assertDeepCopyIsEqual({})
end

function tests.deepCopyOneItemArray()
  assertDeepCopyIsEqual({'hello'})
end

function tests.deepCopyFlatArray()
  assertDeepCopyIsEqual({'hello', 7, 'swha!'})
end

function tests.deepCopyOneItemTable()
  assertDeepCopyIsEqual({myKey = 'hello'})
end

function tests.deepCopyFlatTable()
  assertDeepCopyIsEqual({
      ['myKey'] = 'hello',
      [7] = 'there',
      ['thinking'] = 1
  })
end

function tests.deepCopyParentChildTable()
  local leaf = {myKey = 'hello'}
  assertDeepCopyIsEqual({root = leaf})
end

function tests.deepCopyComplexKeyTable()
  local key = {myKey = 'hello'}
  assertDeepCopyIsEqual({[key] = 'yo'})
end

function tests.deepCopyBigTable()
  local key = {myKey = 'hello'}
  local leaf = {fancy = 'string'}
  local mid = {midKey = leaf}

  assertDeepCopyIsEqual({
      'try',
      'something',
      'here',
      [key] = leaf,
      anotherKey = mid
  })
end

function tests.flattenNestedTable()
  local nestedTable = {
      sub0 = {
          sub00 = {
              a = 'hello',
              b = 10,
          },
          sub01 = {1, 2, 3},
      },
      sub1 = {
          sub10 = {
              a = 'world',
              b = 20,
          },
      },
      ['sub2.a'] = 6
  }

  asserts.tablesEQ(tables.flatten(nestedTable), {
      ['sub0.sub00.a'] = 'hello',
      ['sub0.sub00.b'] = 10,
      ['sub0.sub01.1'] = 1,
      ['sub0.sub01.2'] = 2,
      ['sub0.sub01.3'] = 3,
      ['sub1.sub10.a'] = 'world',
      ['sub1.sub10.b'] = 20,
      ['sub2.a'] = 6,
  })
end

function tests.flattenSelfReferencingTable()
  local tableA = {
      a0 = 'a0',
      a1 = 'a1',
  }
  local tableB = {
      b0 = 'b0',
      b1 = 'b1',
  }
  tableA.tableB = tableB
  tableB.tableA = tableA
  asserts.tablesEQ(tables.flatten(tableA), {
      ['a0'] = 'a0',
      ['a1'] = 'a1',
      ['tableB.b0'] = 'b0',
      ['tableB.b1'] = 'b1',
  })
end

function tests.tostring()
  asserts.EQ(tables.tostring('A'), '"A"')
  asserts.EQ(tables.tostring(10), '10')
  asserts.EQ(tables.tostring{a = 10}, '{\n  ["a"] = 10\n}')
  asserts.EQ(tables.tostring{10}, '{\n  [1] = 10\n}')
end


local tostringTableExpected = [[{
  [1] = 10
  [2] = {
    ["a"] = 11
  }
}]]
function tests.tostringTable()
    asserts.EQ(tables.tostring({10, {a = 11}}), tostringTableExpected)
end

function tests.tostringOneLine()
  asserts.EQ(tables.tostringOneLine{10, {a = 11}}, "{10, {a = 11}}")
  asserts.EQ(tables.tostringOneLine{10, {['a '] = 11}}, "{10, {['a '] = 11}}")
  asserts.EQ(tables.tostringOneLine{['1aa'] = 11}, "{['1aa'] = 11}")
  asserts.EQ(tables.tostringOneLine{_a1aa = 11}, "{_a1aa = 11}")
  asserts.EQ(tables.tostringOneLine{__tostring = 11}, "{}")
  asserts.EQ(tables.tostringOneLine{['while'] = 11}, "{['while'] = 11}")
  local tablePrinter = {__tostring = tables.tostringOneLine}
  local pointA = setmetatable({10, 20}, tablePrinter)
  local pointB = setmetatable({3, 6}, tablePrinter)
  asserts.EQ(tostring(pointA), "{10, 20}")
  asserts.EQ(tostring(pointB), "{3, 6}")
  asserts.EQ(tables.tostringOneLine{pointA, pointB}, "{{10, 20}, {3, 6}}")
  local advancedPrinter = {
      __tostring = function (self)
        return 'Point ' .. tables.tostringOneLine(self, true)
      end
  }
  local pointC = setmetatable({30, 60}, advancedPrinter)
  asserts.EQ(tostring(pointC), 'Point {30, 60}')
end

local function unpackCapture(...)
  return {args = {...}, count = select('#', ...)}
end

function tests.unpackEmptyWorks()
  local capture = unpackCapture()
  local function testUnpack(...)
    asserts.EQ(select('#', ...), 0)
  end
  testUnpack(tables.unpack(capture.args, capture.count))
end

function tests.unpackOneNilWorks()
  local capture = unpackCapture(nil)
  local function testUnpack(...)
    asserts.EQ(select('#', ...), 1)
    asserts.EQ(select(1, ...), nil)
  end
  testUnpack(tables.unpack(capture.args, capture.count))
end

function tests.unpackTwoWorks()
  local capture = unpackCapture(1, nil)
  local function testUnpack(...)
    asserts.EQ(select('#', ...), 2)
    asserts.EQ(select(1, ...), 1)
    asserts.EQ(select(2, ...), nil)
  end
  testUnpack(tables.unpack(capture.args, capture.count))
end

function tests.unpackMixedWorks()
  local capture = unpackCapture(nil, 10, nil)
  local function testUnpack(...)
    asserts.EQ(select('#', ...), 3)
    asserts.EQ(select(1, ...), nil)
    asserts.EQ(select(2, ...), 10)
    asserts.EQ(select(3, ...), nil)
  end
  testUnpack(tables.unpack(capture.args, capture.count))
end

function tests.unpackStartIndexWorks()
  local capture = unpackCapture(1, 2, 3, 4)
  local function testUnpack(...)
    asserts.EQ(select('#', ...), 3)
    asserts.EQ(select(1, ...), 2)
    asserts.EQ(select(2, ...), 3)
    asserts.EQ(select(3, ...), 4)
  end
  testUnpack(tables.unpack(capture.args, capture.count, 2))
end

function tests.unpackInfered()
  local capture = unpackCapture(10, 20, nil)
  local function testUnpack(...)
    asserts.EQ(select('#', ...), 2)
    asserts.EQ(select(1, ...), 10)
    asserts.EQ(select(2, ...), 20)
  end
  testUnpack(tables.unpack(capture.args))
end

return test_runner.run(tests)
