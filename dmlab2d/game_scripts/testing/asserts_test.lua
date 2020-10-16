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

local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local args = require 'common.args'

local tests = {}

local shouldFail = asserts.shouldFail

function tests.thatSuccess()
  asserts.that(7, args.numberType)
  asserts.that(7, 7)
  asserts.that(7, args.nonNegative)
  asserts.that(7, args.gt(6))
  asserts.that(7, args.all(args.gt(5), args.lt(9)))
end

function tests.thatFailure()
  shouldFail(function() asserts.that(7, args.stringType) end,
      'type expected: "string", actual: "number"')

  shouldFail(function() asserts.that(7, 6) end,
      '7 is not equal to 6')

  shouldFail(function() asserts.that(7, args.nonPositive) end,
      '7 is not less-equal to 0')
  shouldFail(
      function()
        asserts.that(7, args.all(args.gt(9), args.lt(11)))
      end,
      '7 is not greater-than 9')
end

function tests.EQSuccess()
  asserts.EQ(7, 7)
  asserts.EQ(7, 7, 'message')
  asserts.EQ(true, true)
  asserts.EQ(false, false)
  asserts.EQ("a string", "a string")
  asserts.EQ(nil, nil)
  local tbl = {2, 4, 6}
  asserts.EQ(tbl, tbl)
end

function tests.EQFailure()
  shouldFail(function() asserts.EQ(7, 8) end)
  shouldFail(function() asserts.EQ(true, false) end)
  shouldFail(function() asserts.EQ(false, true) end)
  shouldFail(function() asserts.EQ("a string", "a different string") end)
  shouldFail(function() asserts.EQ(nil, {}) end)

  local tbl = {2, 4, 6}
  shouldFail(function() asserts.EQ({2, 4, 6}, {2, 4, 6}) end)

  shouldFail(function() asserts.EQ(7, true) end)
  shouldFail(function() asserts.EQ(true, 7) end)
  shouldFail(function() asserts.EQ(7, false) end)
  shouldFail(function() asserts.EQ(false, 7) end)
  shouldFail(function() asserts.EQ(7, "a string") end)
  shouldFail(function() asserts.EQ("a string", 7) end)
  shouldFail(function() asserts.EQ(7, {2, 4, 6}) end)
  shouldFail(function() asserts.EQ({2, 4, 6}, 7) end)

  shouldFail(function() asserts.EQ(true, "a string") end)
  shouldFail(function() asserts.EQ("a string", true) end)
  shouldFail(function() asserts.EQ(true, {2, 4, 6}) end)
  shouldFail(function() asserts.EQ({2, 4, 6}, true) end)

  shouldFail(function() asserts.EQ(false, "a string") end)
  shouldFail(function() asserts.EQ("a string", false) end)
  shouldFail(function() asserts.EQ(false, {2, 4, 6}) end)
  shouldFail(function() asserts.EQ({2, 4, 6}, false) end)

  shouldFail(function() asserts.EQ("a string", {2, 4, 6}) end)
  shouldFail(function() asserts.EQ({2, 4, 6}, "a string") end)


  shouldFail(function() asserts.EQ(7, 8, 'monkey') end,
      {'7', '8', 'monkey'})
end


function tests.NESuccess()
  asserts.NE(7, 8)
  asserts.NE(true, false)
  asserts.NE(false, true)
  asserts.NE("a string", "a different string")
  asserts.NE(nil, {})
  asserts.NE({2, 4, 6}, {2, 4, 6})

  asserts.NE(7, true)
  asserts.NE(true, 7)
  asserts.NE(7, false)
  asserts.NE(false, 7)
  asserts.NE(7, "a string")
  asserts.NE("a string", 7)
  asserts.NE(7, {2, 4, 6})
  asserts.NE({2, 4, 6}, 7)

  asserts.NE(true, "a string")
  asserts.NE("a string", true)
  asserts.NE(true, {2, 4, 6})
  asserts.NE({2, 4, 6}, true)

  asserts.NE(false, "a string")
  asserts.NE("a string", false)
  asserts.NE(false, {2, 4, 6})
  asserts.NE({2, 4, 6}, false)

  asserts.NE("a string", {2, 4, 6})
  asserts.NE({2, 4, 6}, "a string")
end

function tests.NEFailure()
  shouldFail(function() asserts.NE(7, 7) end)
  shouldFail(function() asserts.NE(7, 7, 'message') end)
  shouldFail(function() asserts.NE(true, true) end)
  shouldFail(function() asserts.NE(false, false) end)
  shouldFail(function() asserts.NE("a string", "a string") end)
  shouldFail(function() asserts.NE(nil, nil) end)
  local tbl = {2, 4, 6}
  shouldFail(function() asserts.NE(tbl, tbl) end)

  shouldFail(function() asserts.NE("monkey", "monkey") end,
      'Expected values to differ: monkey')

  shouldFail(function() asserts.NE("monkey", "monkey", "magic") end,
      'Expected values to differ: monkey magic')
end

function tests.shouldFailSpecialCharacters()
  shouldFail(
      function() error("Message with [] () -.") end,
      'Message with [] () -.')
end

function tests.shouldFailList()
  shouldFail(
    function()
      shouldFail(
          function() error("cat dog bike") end,
          'cat', 'dog', 'cow')
    end,
    'Expected "cow" to appear')
end

function tests.shouldFailTable()
  shouldFail(
    function()
      shouldFail(
          function() error("cat dog bike") end,
          {'cat', 'dog', 'cow'})
    end,
    'Expected "cow" to appear')
end

function tests.GTSuccess()
  asserts.GT(1, 0)
  asserts.GT(0, -3)
  asserts.GT(3.14, 3)
end

function tests.GTFailure()
  shouldFail(function() asserts.GT(0, 1) end)
  shouldFail(function() asserts.GT(-3, 0) end)
  shouldFail(function() asserts.GT(3, 3) end)
  shouldFail(function() asserts.GT(3, 3.14) end)

  shouldFail(function() asserts.GT(0, 1) end,
      'Expected: 0 > 1')
  shouldFail(function() asserts.GT(0, 1, 'monkey') end,
      'Expected: 0 > 1 monkey')
end

function tests.GESuccess()
  asserts.GE(0, 0)
  asserts.GE(1, 0)
  asserts.GE(0, -3)
  asserts.GE(3, 3)
  asserts.GE(3.14, 3)
end

function tests.GEFailure()
  shouldFail(function() asserts.GE(0, 1) end)
  shouldFail(function() asserts.GE(-3, 0) end)
  shouldFail(function() asserts.GE(3, 3.14) end)

  shouldFail(function() asserts.GE(0, 1) end,
      'Expected: 0 >= 1')
  shouldFail(function() asserts.GE(0, 1, 'monkey') end,
      'Expected: 0 >= 1 monkey')
end

function tests.LTSuccess()
  asserts.LT(0, 1)
  asserts.LT(-3, 0)
  asserts.LT(3, 3.14)
end

function tests.LTFailure()
  shouldFail(function() asserts.LT(1, 0) end)
  shouldFail(function() asserts.LT(0, -3) end)
  shouldFail(function() asserts.LT(3, 3) end)
  shouldFail(function() asserts.LT(3.14, 3) end)

  shouldFail(function() asserts.LT(1, 0) end,
      'Expected: 1 < 0')
  shouldFail(function() asserts.LT(1, 0, 'monkey') end,
      'Expected: 1 < 0 monkey')
end

function tests.LESuccess()
  asserts.LE(0, 0)
  asserts.LE(0, 1)
  asserts.LE(-3, 0)
  asserts.LE(-3, -3)
  asserts.LE(3, 3.14)
end

function tests.LEFailure()
  shouldFail(function() asserts.LE(1, 0) end)
  shouldFail(function() asserts.LE(0, -3) end)
  shouldFail(function() asserts.LE(3.14, 3) end)

  shouldFail(function() asserts.LE(1, 0) end,
      'Expected: 1 <= 0')
  shouldFail(function() asserts.LE(1, 0, 'monkey') end,
      'Expected: 1 <= 0 monkey')
end


function tests.TablesEQSuccess()
  asserts.tablesEQ({}, {})
  asserts.tablesEQ({2, 4, 6}, {2, 4, 6})
  local tbl = {'foo', 'bar'}
  asserts.tablesEQ(tbl, tbl)
  asserts.tablesEQ(
      {pet = 'monkey', power = 'magic'},
      {pet = 'monkey', power = 'magic'})

  asserts.tablesEQ({2, 4, {nested = 6}}, {2, 4, {nested = 6}})
end

function tests.TablesEQFailure()
  shouldFail(function() asserts.tablesEQ({}, {1}) end)
  shouldFail(function() asserts.tablesEQ({1}, {}) end)
  shouldFail(function() asserts.tablesEQ({2, 4, 6}, {2, 4}) end)
  shouldFail(function() asserts.tablesEQ({2, 4}, {2, 4, 6}) end)
  shouldFail(function() asserts.tablesEQ({2, 4, 6}, {3, 4, 6}) end)
  shouldFail(function() asserts.tablesEQ({2, 4, 6}, {3, 4, 6}) end)
  shouldFail(function() asserts.tablesEQ({2, 4, 6}, {2, 5, 6}) end)
  shouldFail(function() asserts.tablesEQ({2, 4, 6}, {2, 4, 7}) end)
  shouldFail(function() asserts.tablesEQ({2, 4, 6}, {4, 2, 6}) end)

  shouldFail(function()
    asserts.tablesEQ(
        {2, 4, {nested = 6}},
        {2, 4, {nested = 7}})
  end)

  shouldFail(function()
    asserts.tablesEQ(
        {pet = 'monkey', power = 'magic'},
        {pet = 'dog', power = 'slobber'})
  end)

  shouldFail(function() asserts.tablesEQ({}, {1}) end,
      'Expected: {1}. Actual: {}.')

  shouldFail(function() asserts.tablesEQ({}, {1}, 'OOPS') end,
      'Expected: {1}. Actual: {}. OOPS')
end

function tests.TablesNESuccess()
  asserts.tablesNE({}, {1})
  asserts.tablesNE({1}, {})
  asserts.tablesNE({2, 4, 6}, {2, 4})
  asserts.tablesNE({2, 4}, {2, 4, 6})
  asserts.tablesNE({2, 4, 6}, {3, 4, 6})
  asserts.tablesNE({2, 4, 6}, {3, 4, 6})
  asserts.tablesNE({2, 4, 6}, {2, 5, 6})
  asserts.tablesNE({2, 4, 6}, {2, 4, 7})
  asserts.tablesNE({2, 4, 6}, {4, 2, 6})

  asserts.tablesNE({2, 4, {nested = 6}}, {2, 4, {nested = 7}})
  asserts.tablesNE(
      {pet = 'monkey', power = 'magic'},
      {pet = 'dog', power = 'slobber'})
end

function tests.TablesNEFailure()
  shouldFail(function() asserts.tablesNE({}, {}) end)
  shouldFail(function() asserts.tablesNE({2, 4, 6}, {2, 4, 6}) end)

  local tbl = {'foo', 'bar'}
  shouldFail(function() asserts.tablesNE(tbl, tbl) end)

  shouldFail(function()
    asserts.tablesNE(
        {pet = 'monkey', power = 'magic'},
        {pet = 'monkey', power = 'magic'})
  end)

  shouldFail(function()
    asserts.tablesNE(
        {2, 4, {nested = 6}},
        {2, 4, {nested = 6}})
  end)

  shouldFail(function() asserts.tablesNE({1}, {1}) end,
    'Expected tables with different values.')

  shouldFail(function() asserts.tablesNE({1}, {1}, 'OOPS') end,
    {'Expected tables with different values.', 'OOPS'})
end

function tests.StringHasSubstrSuccess()
  asserts.hasSubstr('My string is here', 'is her')
  asserts.hasSubstr('My string is here', 'My str')
  asserts.hasSubstr('My string is here', '')
  asserts.hasSubstr('', '')
  asserts.hasSubstr('Another string', 'er strin')
  asserts.hasSubstr('Another string', 'ing')
  asserts.hasSubstr('Another string', 'Another string')
  asserts.hasSubstr('Yet_another-one', '_')
  asserts.hasSubstr('Yet_another-one', '-')
  asserts.hasSubstr('Yet_another-one', 'Y')
  asserts.hasSubstr('142534', '42')
  asserts.hasSubstr('$^@£$£$^&', '$£')
end

function tests.StringHasSubstrFailure()
  asserts.shouldFail(
      function() asserts.hasSubstr('My string is here', 'not there') end)
  asserts.shouldFail(
      function() asserts.hasSubstr('My string is here', 'there') end)
  asserts.shouldFail(
      function() asserts.hasSubstr('My string', 'My string is here') end)
  asserts.shouldFail(
      function() asserts.hasSubstr('My string', 'This is My string') end)
  asserts.shouldFail(function() asserts.hasSubstr('', 'This is My string') end)
  asserts.shouldFail(function() asserts.hasSubstr(123423, 'a string') end)
  asserts.shouldFail(function() asserts.hasSubstr(123423, 42) end)
  asserts.shouldFail(function() asserts.hasSubstr('123423', 42) end)
  asserts.shouldFail(function() asserts.hasSubstr(123423, '42') end)
  asserts.shouldFail(function() asserts.hasSubstr({'42'}, '42') end)
  asserts.shouldFail(function() asserts.hasSubstr({1, 3, {2}}, '42') end)
  asserts.shouldFail(function() asserts.hasSubstr('123423', {4, 2}) end)
  asserts.shouldFail(function() asserts.hasSubstr('123423', {'4', '2'}) end)
end

return test_runner.run(tests)
