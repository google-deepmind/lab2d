--[[ Copyright (C) 2020 The DMLab2D Authors.

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
local tensor = require 'system.tensor'
local class = require 'common.class'

local tests = {}

local function myKwargsFunc(kwargs)
  kwargs = args.parse(kwargs, {
      {'a', args.numberType},
      {'b', args.numberType},
      {'c', args.default(10)},
  })
  return kwargs
end

function tests.myKwargsFuncCallSuccess()
  asserts.tablesEQ(myKwargsFunc{a = 1, b = 2}, {a = 1, b = 2, c = 10})
  asserts.tablesEQ(myKwargsFunc{a = 1, b = 2, c = 3}, {a = 1, b = 2, c = 3})
end

function tests.myKwargsFuncCallFail()
  asserts.shouldFail(
      function() myKwargsFunc{a = 1} end,
      'Required argument \'b\' missing')
  asserts.shouldFail(
      function() myKwargsFunc{a = 1, b = 2, cc = 3}end,
      'Unsupported argument \'cc\'')
end

local function myPosFunc(...)
  local a, b, c = unpack(args.parse({...}, {
      {1, args.numberType},
      {2, args.numberType},
      {3, args.default(10)},
  }))
  return a, b, c
end

function tests.myPosFuncCallSuccess()
  asserts.tablesEQ({myPosFunc(1, 2)}, {1, 2, 10})
  asserts.tablesEQ({myPosFunc(1, 2, 3)}, {1, 2, 3})
end

function tests.myPosFuncCallFail()
  asserts.shouldFail(
      function() myPosFunc(1, 'a') end,
      'Error parsing 2: "a" type expected: "number", actual: "string"')
  asserts.shouldFail(
      function() myPosFunc(1) end,
      'Required argument \'2\' missing')
  asserts.shouldFail(
      function() myPosFunc(1, 2, 3, 4) end,
      'Unsupported argument \'4\'')
end

function tests.default()
  local declArgs = {{'opt', args.default('first_def')}}
  asserts.tablesEQ(args.parse({opt = 'first'}, declArgs), {opt = 'first'})
  asserts.tablesEQ(args.parse({}, declArgs), {opt = 'first_def'})
end

function tests.lazy()
  local readLazy = false
  local lazyValue = function()
    readLazy = true
    return 'first_def'
  end
  local declArgs = {{'opt', args.lazy(lazyValue)}}
  asserts.tablesEQ(args.parse({opt = 'first'}, declArgs), {opt = 'first'})
  asserts.EQ(readLazy, false)
  asserts.tablesEQ(args.parse({}, declArgs), {opt = 'first_def'})
  asserts.EQ(readLazy, true)
end

function tests.oneOfSuccess()
  local declArgs = {{'opt', args.oneOf('a', 'b', 'c')}}
  asserts.tablesEQ(args.parse({opt = 'a'}, declArgs), {opt = 'a'})
  asserts.tablesEQ(args.parse({opt = 'b'}, declArgs), {opt = 'b'})
  asserts.tablesEQ(args.parse({opt = 'c'}, declArgs), {opt = 'c'})
end

function tests.oneOfFail()
  local declArgs = {{'opt', args.oneOf('xx', 'yy', 'zz')}}
  asserts.shouldFail(
      function() args.parse({opt = 'd'}, declArgs) end,
      {'Error parsing "opt": "d" does not match any of', 'xx', 'yy', 'zz'})
end

function tests.stringTypeSuccess()
  local declArgs = {{'opt', args.stringType}}
  asserts.tablesEQ(args.parse({opt = 'a'}, declArgs), {opt = 'a'})
  asserts.tablesEQ(args.parse({opt = 'b'}, declArgs), {opt = 'b'})
  asserts.tablesEQ(args.parse({opt = 'c'}, declArgs), {opt = 'c'})
end

function tests.stringTypeFail()
  local declArgs = {{'opt', args.stringType}}
  asserts.shouldFail(
      function() args.parse({opt = 1}, declArgs) end,
      'Error parsing "opt": 1 type expected: "string", actual: "number"')
end

function tests.numberTypeSuccess()
  local declArgs = {{'opt', args.numberType}}
  asserts.tablesEQ(args.parse({opt = 1}, declArgs), {opt = 1})
  asserts.tablesEQ(args.parse({opt = 2}, declArgs), {opt = 2})
  asserts.tablesEQ(args.parse({opt = 3}, declArgs), {opt = 3})
end

function tests.numberTypeFail()
  local declArgs = {{'opt', args.numberType}}
  asserts.shouldFail(
      function() args.parse({opt = 'a'}, declArgs) end,
      'Error parsing "opt": "a" type expected: "number", actual: "string"'
  )
end

function tests.booleanTypeSuccess()
  local declArgs = {{'opt', args.booleanType}}
  asserts.tablesEQ(args.parse({opt = true}, declArgs), {opt = true})
  asserts.tablesEQ(args.parse({opt = false}, declArgs), {opt = false})
end

function tests.booleanTypeFail()
  local declArgs = {{'opt', args.booleanType}}
  asserts.shouldFail(
      function() args.parse({opt = 'a'}, declArgs) end,
      'Error parsing "opt": "a" type expected: "boolean", actual: "string"'
  )
end

function tests.tableTypeSuccess()
  local declArgs = {{'opt', args.tableType}}
  asserts.tablesEQ(args.parse({opt = {}}, declArgs), {opt = {}})
  asserts.tablesEQ(args.parse({opt = {a = 1}}, declArgs), {opt = {a = 1}})
end

function tests.tableTypeFail()
  local declArgs = {{'opt', args.tableType}}
  asserts.shouldFail(
      function() args.parse({opt = 'a'}, declArgs) end,
      'Error parsing "opt": "a" type expected: "table", actual: "string"'
  )
end

function tests.functionTypeSuccess()
  local f1 = function() end
  local declArgs = {{'opt', args.functionType}}
  asserts.tablesEQ(args.parse({opt = f1}, declArgs), {opt = f1})
end

function tests.functionTypeFail()
  local declArgs = {{'opt', args.functionType}}
  asserts.shouldFail(
      function() args.parse({opt = 'a'}, declArgs) end,
      'Error parsing "opt": "a" type expected: "function", actual: "string"'
  )
end

function tests.userdataTypeSuccess()
  local t1 = tensor.DoubleTensor{1, 2}
  local declArgs = {{'opt', args.userdataType}}
  asserts.tablesEQ(args.parse({opt = t1}, declArgs), {opt = t1})
end

function tests.userdataTypeFail()
  local declArgs = {{'opt', args.userdataType}}
  asserts.shouldFail(
      function() args.parse({opt = 'a'}, declArgs) end,
      'Error parsing "opt": "a" type expected: "userdata", actual: "string"'
  )
end

function tests.eqSuccess()
  local declArgs = {{'opt', args.eq(1)}}
  asserts.tablesEQ(args.parse({opt = 1}, declArgs), {opt = 1})
end

function tests.eqFail()
  local declArgs = {{'opt', args.eq(1)}}
  asserts.shouldFail(
      function() args.parse({opt = 0}, declArgs) end,
      'Error parsing "opt": 0 is not equal to 1'
  )
end

function tests.geSuccess()
  local declArgs = {{'opt', args.ge(1)}}
  asserts.tablesEQ(args.parse({opt = 1}, declArgs), {opt = 1})
  asserts.tablesEQ(args.parse({opt = 2}, declArgs), {opt = 2})
end

function tests.geFail()
  local declArgs = {{'opt', args.ge(1)}}
  asserts.shouldFail(
      function() args.parse({opt = 0}, declArgs) end,
      'Error parsing "opt": 0 is not greater-equal to 1'
  )
end

function tests.leSuccess()
  local declArgs = {{'opt', args.le(1)}}
  asserts.tablesEQ(args.parse({opt = 1}, declArgs), {opt = 1})
  asserts.tablesEQ(args.parse({opt = 0}, declArgs), {opt = 0})
end

function tests.leFail()
  local declArgs = {{'opt', args.le(1)}}
  asserts.shouldFail(
      function() args.parse({opt = 2}, declArgs) end,
      'Error parsing "opt": 2 is not less-equal to 1'
  )
end

function tests.gtSuccess()
  local declArgs = {{'opt', args.gt(1)}}
  asserts.tablesEQ(args.parse({opt = 2}, declArgs), {opt = 2})
end

function tests.gtFail()
  local declArgs = {{'opt', args.gt(1)}}
  asserts.shouldFail(
      function() args.parse({opt = 0}, declArgs) end,
      'Error parsing "opt": 0 is not greater-than 1'
  )
  asserts.shouldFail(
      function() args.parse({opt = 1}, declArgs) end,
      'Error parsing "opt": 1 is not greater-than 1'
  )
end

function tests.ltSuccess()
  local declArgs = {{'opt', args.lt(1)}}
  asserts.tablesEQ(args.parse({opt = 0}, declArgs), {opt = 0})
end

function tests.ltFail()
  local declArgs = {{'opt', args.lt(1)}}
  asserts.shouldFail(
      function() args.parse({opt = 2}, declArgs) end,
      'Error parsing "opt": 2 is not less-than 1'
  )
  asserts.shouldFail(
      function() args.parse({opt = 1}, declArgs) end,
      'Error parsing "opt": 1 is not less-than 1'
  )
end

function tests.neSuccess()
  local declArgs = {{'opt', args.ne(1)}}
  asserts.tablesEQ(args.parse({opt = 0}, declArgs), {opt = 0})
end

function tests.neFail()
  local declArgs = {{'opt', args.ne(1)}}
  asserts.shouldFail(
      function() args.parse({opt = 1}, declArgs) end,
      'Error parsing "opt": 1 is equal to 1'
  )
end

function tests.positiveSuccess()
  local declArgs = {{'opt', args.positive}}
  asserts.tablesEQ(args.parse({opt = 1}, declArgs), {opt = 1})
end

function tests.positiveFail()
  local declArgs = {{'opt', args.positive}}
  asserts.shouldFail(
      function() args.parse({opt = -1}, declArgs) end,
      'Error parsing "opt": -1 is not greater-than 0'
  )
  asserts.shouldFail(
      function() args.parse({opt = 0}, declArgs) end,
      'Error parsing "opt": 0 is not greater-than 0'
  )
end

function tests.negativeSuccess()
  local declArgs = {{'opt', args.negative}}
  asserts.tablesEQ(args.parse({opt = -1}, declArgs), {opt = -1})
end

function tests.negativeFail()
  local declArgs = {{'opt', args.negative}}
  asserts.shouldFail(
      function() args.parse({opt = 1}, declArgs) end,
      'Error parsing "opt": 1 is not less-than 0'
  )
  asserts.shouldFail(
      function() args.parse({opt = 0}, declArgs) end,
      'Error parsing "opt": 0 is not less-than 0'
  )
end

function tests.nonNegativeSuccess()
  local declArgs = {{'opt', args.nonNegative}}
  asserts.tablesEQ(args.parse({opt = 0}, declArgs), {opt = 0})
  asserts.tablesEQ(args.parse({opt = 1}, declArgs), {opt = 1})
end

function tests.nonNegativeFail()
  local declArgs = {{'opt', args.nonNegative}}
  asserts.shouldFail(
      function() args.parse({opt = -1}, declArgs) end,
      'Error parsing "opt": -1 is not greater-equal to 0'
  )
end

function tests.nonPositiveSuccess()
  local declArgs = {{'opt', args.nonPositive}}
  asserts.tablesEQ(args.parse({opt = 0}, declArgs), {opt = 0})
  asserts.tablesEQ(args.parse({opt = -1}, declArgs), {opt = -1})
end

function tests.nonPositiveFail()
  local declArgs = {{'opt', args.nonPositive}}
  asserts.shouldFail(
      function() args.parse({opt = 1}, declArgs) end,
      'Error parsing "opt": 1 is not less-equal to 0'
  )
end

function tests.nonZeroSuccess()
  local declArgs = {{'opt', args.nonZero}}
  asserts.tablesEQ(args.parse({opt = -1}, declArgs), {opt = -1})
  asserts.tablesEQ(args.parse({opt = 1}, declArgs), {opt = 1})
end

function tests.nonZeroFail()
  local declArgs = {{'opt', args.nonZero}}
  asserts.shouldFail(
      function() args.parse({opt = 0}, declArgs) end,
      'Error parsing "opt": 0 is equal to 0'
  )
end

function tests.hasSubStrToString()
  asserts.EQ(tostring(args.hasSubstr('hello*')), 'hasSubstr("hello*")')
end

function tests.hasSubStrSuccess()
  local declArgs = {{'opt', args.hasSubstr('hello*')}}
  asserts.tablesEQ(
      args.parse({opt = 'we have hello* sdf'}, declArgs),
      {opt = 'we have hello* sdf'})
end

function tests.hasSubStrFail()
  local declArgs = {{'opt', args.hasSubstr('hello*')}}
    asserts.shouldFail(
        function() args.parse({opt = 'we have hello'}, declArgs) end,
        '"we have hello" does not contain "hello*"')
end

function tests.tableOfSuccess()
  local declArgs = {{'opt', args.tableOf(args.stringType)}}
  local values = {'a', 'b', 'c'}
  asserts.tablesEQ(args.parse({opt = values}, declArgs), {opt = values})
end

function tests.tableOfFail()
  local declArgs = {{'opt', args.tableOf(args.stringType)}}
  local values = {'a', 20, 30}
  asserts.shouldFail(
      function() args.parse({opt = values}, declArgs) end,
      {
          'Error parsing "opt":',
          'tableOf [2]: 20',
          'type expected: "string", actual: "number"'
      })
end

function tests.instanceSuccess()
  local C = class.Class():__name__('C')
  local c = C()
  local declArgs = {{'opt', args.instance(C)}}
  asserts.tablesEQ(args.parse({opt = c}, declArgs), {opt = c})

  local D = class.Class(C):__name__('D')
  local d = D()
  asserts.tablesEQ(args.parse({opt = d}, declArgs), {opt = d})
end

function tests.instanceFail()
  local C = class.Class():__name__('C')
  local c = C()
  local declArgs = {{'opt', args.instance(C)}}
  local D = class.Class():__name__('D')
  local d = D()
  asserts.shouldFail(
      function() args.parse({opt = d}, declArgs) end,
      {
          'Error parsing "opt":',
          'instance not derrived from class.'
      })
  asserts.shouldFail(
      function() args.parse({opt = 10}, declArgs) end,
      {
          'Error parsing "opt":',
          'instance not derrived from class.'
      })
  asserts.shouldFail(
      function() args.parse({opt = {}}, declArgs) end,
      {
          'Error parsing "opt":',
          'instance not derrived from class.'
      })
end

function tests.classIsSuccess()
  local C = class.Class():__name__('C')
  local c = C()
  local declArgs = {{'opt', args.classIs(C)}}
  asserts.tablesEQ(args.parse({opt = c}, declArgs), {opt = c})
end

function tests.classIsFail()
  local C = class.Class():__name__('C')
  local c = C()
  local D = class.Class(C):__name__('D')
  local d = D()
  local declArgs = {{'opt', args.classIs(C)}}
  asserts.shouldFail(
      function() args.parse({opt = {}}, declArgs) end,
      {
          'Error parsing "opt":',
          'classIs incorrect instance class.'
      })
  asserts.shouldFail(
      function() args.parse({opt = 10}, declArgs) end,
      {
          'Error parsing "opt":',
          'classIs incorrect instance class.'
      })
  asserts.shouldFail(
      function() args.parse({opt = {}}, declArgs) end,
      {
          'Error parsing "opt":',
          'classIs incorrect instance class.'
      })
end

function tests.conditionalSuccess()
  local function isEven(val) return val % 2 == 0 end
  local declArgs = {{'opt', args.conditional(isEven, 'is odd')}}
  asserts.tablesEQ(args.parse({opt = 10}, declArgs), {opt = 10})
end

function tests.conditionalFail()
  local function isEven(val) return val % 2 == 0 end
  local declArgs = {{'opt', args.conditional(isEven, 'is odd')}}
  asserts.shouldFail(
      function() args.parse({opt = 11}, declArgs) end,
      'Error parsing "opt": 11 is odd')
end

function tests.anySuccess()
  local declArgs = {{'opt', args.any(args.stringType, args.tableType)}}
  asserts.tablesEQ(args.parse({opt = 'a'}, declArgs), {opt = 'a'})
  asserts.tablesEQ(args.parse({opt = {}}, declArgs), {opt = {}})
end

function tests.anyFail()
  local declArgs = {{'opt', args.any(args.stringType, args.tableType)}}
  asserts.shouldFail(
      function() args.parse({opt = 11}, declArgs) end,
      {'Error parsing "opt":',
       'Invalid any constraint:',
       '11 type expected: "string", actual: "number"',
       '11 type expected: "table", actual: "number"'})
end

function tests.allSuccess()
  local declArgs = {{'opt', args.all(args.ge(0), args.lt(360))}}
  asserts.tablesEQ(args.parse({opt = 0}, declArgs), {opt = 0})
  asserts.tablesEQ(args.parse({opt = 359}, declArgs), {opt = 359})
end

function tests.allFail()
  local declArgs = {{'opt', args.all(args.ge(0), args.lt(360))}}
  asserts.shouldFail(
      function() args.parse({opt = -10}, declArgs) end,
      'Error parsing "opt": -10 is not greater-equal to 0')
  asserts.shouldFail(
      function() args.parse({opt = 360}, declArgs) end,
      'Error parsing "opt": 360 is not less-than 360')
end

return test_runner.run(tests)
