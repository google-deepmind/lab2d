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
local mocking = require 'testing.mocking'
local class = require 'common.class'
local args = require 'common.args'

-- Function for creating a mocking object. Accepts a template object.
local mock = mocking.mock

-- Used to declare matchers for funtion calls and the actions to take.
local when = mocking.when

-- Library of capture functions.
local capture = mocking.capture

-- Function that returns an anyValue capture object.
local anyValue = mocking.capture.anyValue

-- Function that returns an numberType capture object.
local numberType = mocking.capture.numberType

-- Function that will capture all remaining arguments. A capture object is
-- allowed to be passed to this function adding a requirement that all values
-- must match this capture object.
local dotDotDot = mocking.capture.dotDotDot

-- Resets a mock object to be used again.
local reset = mocking.reset

-- Used to verify a function was called with certain parameters.
local verify = mocking.verify

-- Used to verify all actions from the mock object has been checked.
local verifyNoMoreInteractions = mocking.verifyNoMoreInteractions

local tests = {}


function tests.example1()
  -- Create a mock object.
  local obj = mock()
  when(obj).sum().thenReturn(0)
  when(obj).sum(5, 5).thenReturn(10)
  -- Call function that requires sum.
  asserts.EQ(obj.sum(), 0)
  asserts.EQ(obj.sum(5, 5), 10)
end
local verify = mocking.verify

function tests.example2()
  -- Create a mock object.
  local obj = mock()
  when(obj).sum().thenReturn(0)
  when(obj).sum(5, 5).thenReturn(10)
  -- Call function that requires sum.
  asserts.EQ(obj.sum(), 0)
  asserts.EQ(obj.sum(5, 5), 10)
  verify(obj).sum()
  verify(obj).sum(5, 5)
end

local inOrder = mocking.inOrder

function tests.example3()
  -- Create a mock object.
  local obj = mock()
  when(obj).sum().thenReturn(0)
  when(obj).product().thenReturn(10)
  asserts.EQ(obj.sum(), 0)
  asserts.EQ(obj.product(), 10)
  local seq = inOrder(obj)
  seq.verify().sum()
  seq.verify().product()
end

function tests.example4()
  -- Create a mock object.
  local obj = mock()
  local anyValue1 = capture.anyValue()
  local numberType1 = capture.numberType()

  -- More general matchers should be placed first.
  when(obj).sum(anyValue1).thenReturn(8)

  -- More specific matchers should be placed last.
  when(obj).sum(numberType1).thenReturn(4)

  -- 4 is returned as this call matched the seconds matcher.
  asserts.EQ(obj.sum(4), 4)

  -- 4 is returned as this call matched the first matcher.
  asserts.EQ(obj.sum('string1'), 8)
  asserts.EQ(obj.sum(8), 4)
  asserts.EQ(obj.sum('string2'), 8)

  -- numberType1 should have captured two values from the two calls above.
  asserts.EQ(numberType1.get(), 4)
  asserts.EQ(numberType1.get(), 8)

  -- anyValue1 should have captured two values from the two calls above.
  asserts.EQ(anyValue1.get(), 'string1')
  asserts.EQ(anyValue1.get(), 'string2')

  -- Verify the first call to sum was with 4.
  verify(obj).sum(4)

  -- Verify is allowed to use capture arguments too.
  local stringType1 = capture.stringType()
  -- This will verify the second call to sum was called with stringType1.
  verify(obj).sum(stringType1)
  -- And stringType1 has string 'string1'. This may be useful for a partial
  -- string match.
  asserts.EQ(stringType1.get(), 'string1')
  verify(obj).sum(8)
  verify(obj).sum('string2')
end


-- Tests that mocking with specified argument values works.
function tests.withValues()
  local obj = mock()
  -- Create catch all matcher that returns nil if none of the othe matchers
  -- succeed.
  when(obj).sum(dotDotDot())
  when(obj).sum(5, 5).thenReturn(10)

  asserts.EQ(obj.sum(), nil)
  asserts.EQ(obj.sum(4, 6), nil)
  asserts.EQ(obj.sum(5, 5), 10)
  asserts.EQ(obj.sum(5), nil)
  asserts.EQ(obj.sum(5, 5, 0), nil)
  asserts.EQ(obj.sum(5, 'a'), nil)
end


-- Tests that mocking with argument captors works.
function tests.withCaptors()
  local obj = mock()
  when(obj).sum(dotDotDot())
  when(obj).sum(numberType(), numberType()).thenReturn(10)

  asserts.EQ(obj.sum(), nil)
  asserts.EQ(obj.sum(4, 6), 10)
  asserts.EQ(obj.sum(5, 5), 10)
  asserts.EQ(obj.sum(5), nil)
  asserts.EQ(obj.sum(5, 5, 0), nil)
  asserts.EQ(obj.sum(5, 'a'), nil)
end

-- Tests that mocking with argument dotDotDot works.
function tests.withDotDotCaptors()
  local obj = mock()
  local dotDotDot1 = dotDotDot()
  local dotDotDot2 = dotDotDot()
  when(obj).sum(dotDotDot1).thenReturn(5)
  when(obj).sum(numberType(), dotDotDot2).thenReturn(10)
  asserts.EQ(obj.sum(), 5)
  asserts.EQ(obj.sum('hello', 1, nil, 3), 5)
  asserts.EQ(obj.sum(1), 10)
  asserts.EQ(obj.sum(1, 2, 3, 4, 5), 10)
  asserts.tablesEQ({dotDotDot1.get()}, {})
  asserts.tablesEQ({dotDotDot1.get()}, {'hello', 1, nil, 3})
  asserts.tablesEQ({dotDotDot2.get()}, {})
  asserts.tablesEQ({dotDotDot2.get()}, {2, 3, 4, 5})
end

-- Tests that mocking with argument dotDotDot works.
function tests.withDotDotCaptorsCheck()
  local obj = mock()
  local numberType1 = numberType()
  local dotDotDot1 = dotDotDot()
  local dotDotDot2 = dotDotDot(numberType1)
  -- Return 5 if calling with any args.
  when(obj).sum(dotDotDot1).thenReturn(5)
  -- If args are all numbers then return 10.
  when(obj).sum(dotDotDot2).thenReturn(10)
  asserts.EQ(obj.sum('Hi'), 5)
  asserts.EQ(obj.sum(1, 'HI'), 5)
  asserts.EQ(obj.sum(nil, 2), 5)
  asserts.EQ(obj.sum(), 10)
  asserts.EQ(obj.sum(1), 10)
  asserts.EQ(obj.sum(1, 2), 10)

  asserts.tablesEQ({dotDotDot2:get()}, {})
  asserts.tablesEQ({dotDotDot2:get()}, {1})
  asserts.tablesEQ({dotDotDot2:get()}, {1, 2})

  asserts.EQ(numberType1.get(), 1)
  asserts.EQ(numberType1.get(), 1)
  asserts.EQ(numberType1.get(), 2)
end

-- Tests that verifying with argument captors works.
function tests.verifyWithCaptors()
  local obj = mock()
  when(obj).sum(anyValue(), anyValue()).thenReturn()
  obj.sum(1, 2)
  obj.sum(3, 4)

  local number1 = numberType()
  local number2 = numberType()
  verify(obj).sum(number1, number2)
  verify(obj).sum(number1, number2)
  verifyNoMoreInteractions(obj)

  asserts.EQ(number1:get(), 1)
  asserts.EQ(number1:get(), 3)
  asserts.EQ(number2:get(), 2)
  asserts.EQ(number2:get(), 4)
end


-- Tests that mocking with both specified argument values and captors works.
function tests.withValueAndCaptor()
  local obj = mock()
  when(obj).sum(dotDotDot())
  when(obj).sum(numberType(), 5).thenReturn(10)

  asserts.EQ(obj.sum(), nil)
  asserts.EQ(obj.sum(4, 6), nil)
  asserts.EQ(obj.sum(5, 5), 10)
  asserts.EQ(obj.sum(5), nil)
  asserts.EQ(obj.sum(5, 5, 0), nil)
  asserts.EQ(obj.sum(5, nil, 0), nil)
  asserts.EQ(obj.sum(5, 'a'), nil)
end


-- Tests that mocked methods and fields do not interfere.
function tests.methodsAndFields()
  local obj = mock()
  when(obj).sum(dotDotDot())
  when(obj).sum().thenReturn(10)
  when(obj).product().thenError("invalid")

  obj.value = 5

  asserts.EQ(obj.sum(), 10)
  asserts.EQ(obj.sum(5), nil)
  asserts.EQ(obj.value, 5)
  asserts.EQ(obj.anothervalue, nil)
  asserts.shouldFail(obj.product, "invalid")
end


-- Tests that mocking class methods works.
function tests.classMethods()
  local obj = mock()
  when(obj):sum().thenReturn(10)

  asserts.EQ(obj:sum(), 10)
end


-- Tests that forwarding a call to another function works.
function tests.thenCallFunction()
  local lastValue

  local function sqr(x)
    lastValue = x
    if x then
      return x * x
    end
  end

  local obj = mock()
  when(obj).sqr().thenCall(sqr)
  when(obj).sqr(numberType()).thenCall(sqr)

  asserts.EQ(obj.sqr(), nil)
  asserts.EQ(lastValue, nil)

  asserts.EQ(obj.sqr(10), 100)
  asserts.EQ(lastValue, 10)
end


-- Tests that forwarding a call to another methods works.
function tests.thenCallMethod()
  local lastValue
  local obj

  local function sqr(self, x)
    asserts.EQ(self, obj)
    lastValue = x
    if x then
      return x * x
    end
  end

  obj = mock()
  when(obj):sqr().thenCall(sqr)
  when(obj):sqr(numberType()).thenCall(sqr)

  asserts.EQ(obj:sqr(), nil)
  asserts.EQ(lastValue, nil)

  asserts.EQ(obj:sqr(10), 100)
  asserts.EQ(lastValue, 10)
end


-- Tests that capturing argument values works.
function tests.valueCapture()
  local argCaptor1 = numberType()
  local argCaptor2 = numberType()
  local obj = mock()
  when(obj).sum(dotDotDot())
  when(obj).sum(argCaptor1, argCaptor2).thenReturn(10)

  obj.sum(3, 7)
  obj.sum()
  obj.sum(4, 6)

  asserts.EQ(argCaptor1:get(), 3)
  asserts.EQ(argCaptor1:get(), 4)
  asserts.EQ(argCaptor2:get(), 7)
  asserts.EQ(argCaptor2:get(), 6)
end


-- Tests that functions that return nil among their return values work
-- as expected.
function tests.nilAmongReturnValues()
  local obj = mock()
  when(obj).sum().thenReturn(10, nil, 5)

  local a, b, c = obj.sum()
  asserts.EQ(a, 10)
  asserts.EQ(b, nil)
  asserts.EQ(c, 5)
end


-- Tests that function calls with nil among their arguments work
-- as expected.
function tests.nilAmongArguments()
  local obj = mock()
  when(obj).sum(dotDotDot())
  when(obj).sum(10, nil, 5).thenReturn(15)

  asserts.EQ(obj.sum(10), nil)
  asserts.EQ(obj.sum(10, nil, 5), 15)
end


-- Tests that mocking using square brackets works.
function tests.usingIndex()
  local obj = mock()
  when(obj).sum(dotDotDot())
  when(obj)['sum'](5, 5).thenReturn(10)

  asserts.EQ(obj.sum(), nil)
  asserts.EQ(obj.sum(4, 6), nil)
  asserts.EQ(obj.sum(5, 5), 10)
end


-- Tests that mocking table templates works.
function tests.mockTableTemplate()
  local template = {x = 20}

  function template.sum()
    return 5
  end

  local obj = mock(template)
  asserts.EQ(obj.sum(), nil)

  when(obj).sum(5, 5).thenReturn(10)

  asserts.EQ(obj.sum(), nil)
  asserts.EQ(obj.sum(4, 6), nil)
  asserts.EQ(obj.sum(5, 5), 10)
  asserts.EQ(obj.x, 20)
end


-- Tests that verifying function calls works.
function tests.verifyCalls()
  local obj = mock()
  when(obj).sum(dotDotDot())
  when(obj).sum(numberType(), numberType()).thenReturn(10)

  obj.sum(3, 7)
  obj.sum()
  obj.sum(4, 6)

  verify(obj).sum(numberType(), 7)
  verify(obj).sum()
  verify(obj).sum(4, 6)
end


-- Tests that verifying function calls throws an error if necessary.
function tests.verify_errorNoCall()
  local obj = mock()
  when(obj).sum(numberType(), numberType()).thenReturn(10)

  asserts.shouldFail(
      function() verify(obj).sum(3, 7) end,
      "Expected call")
end

-- Tests that verifying function calls throws an error if necessary.
function tests.verify_errorWrongCall()
  local obj = mock()
  when(obj).sum(numberType(), numberType()).thenReturn(10)
  obj.sum(3, 8)

  asserts.shouldFail(
      function() verify(obj).sum(3, 7) end,
      "Expected call")
end


-- Tests that verifying function calls throws an error if necessary.
function tests.verify_functionWithMethodVerify()
  local obj = mock()
  when(obj).sum(numberType(), numberType()).thenReturn(10)
  obj.sum(3, 8)

  asserts.shouldFail(
      function() verify(obj):sum(3, 8) end,
      "Expected call")
end


-- Tests that verifying function calls throws an error if necessary.
function tests.verify_methodWithFunctionVerify()
  local obj = mock()
  when(obj):sum(numberType(), numberType()).thenReturn(10)
  obj:sum(3, 8)

  asserts.shouldFail(
      function()
        verify(obj).sum(3, 8)
      end,
      "Expected call")
end


-- Tests that verifying that there were no more interactions works.
function tests.verifyNoMoreInteractions()
  local obj = mock()
  when(obj).sum(numberType(), numberType()).thenReturn(10)
  obj.sum(3, 7)

  verify(obj).sum(3, 7)
  verifyNoMoreInteractions(obj)
end


-- Tests that ignoring stubs works.
function tests.ignoreOnVerify()
  local obj = mock()
  when(obj).product(numberType(), numberType()).thenReturn(20)
  when(obj).sum(numberType(), numberType()).thenReturn(10).ignoreOnVerify()
  asserts.EQ(obj.sum(3, 7), 10)
  asserts.EQ(obj.product(5, 4), 20)
  asserts.EQ(obj.sum(3, 7), 10)
  verify(obj).product(5, 4)
  verifyNoMoreInteractions(obj)
end


-- Tests that verifying that there were no more interactions works.
function tests.verifyNoMoreInteractions_fail()
  local obj = mock()
  when(obj).sum(numberType(), numberType()).thenReturn(10)

  obj.sum(3, 7)

  asserts.shouldFail(
      function()
        verifyNoMoreInteractions(obj)
      end,
      "More interactions found")
end

-- Tests that resetting a mocking object works.
function tests.reset()
  local obj = mock()
  when(obj).sum(numberType(), numberType()).thenReturn(10)

  obj.sum(1, 2)
  obj.sum(3, 4)

  verify(obj).sum(1, 2)

  reset(obj)
  obj.sum(5, 6)
  verify(obj).sum(5, 6)
  verifyNoMoreInteractions(obj)
end

function tests.inOrder()
  local obj = mock()
  when(obj).a(dotDotDot())
  when(obj).b(dotDotDot())
  when(obj).c(dotDotDot())
  when(obj).d(dotDotDot())
  obj.d(1)
  obj.c(2, 2)
  obj.b(3, 3, 3)
  obj.b(4, 4, 4, 4)
  obj.a(5, 5, 5, 5, 5)
  local inOrder = mocking.inOrder(obj)
  inOrder.verify()
      .d(1)
      .c(2, 2)
      .b(3, 3, 3)
      .b(4, 4, 4, 4)
      .a(5, 5, 5, 5, 5)
  inOrder.verifyNoMoreInteractions()
end

function tests.classMethodForwarded()
  local MyClass = class.Class()
  function MyClass:double(val)
    return 2 * val
  end
  local myInstance = MyClass()
  local obj = mock(myInstance)
  asserts.EQ(obj:double(0), nil)
  asserts.EQ(obj:double(5), nil)
  when(obj):double(numberType()).thenCallRealMethod()
  when(obj):double(0).thenReturn('Zero')
  asserts.EQ(obj:double(0), 'Zero')
  asserts.EQ(obj:double(5), 10)
end

function tests.baseClassMethodForwarded()
  local MyBase = class.Class()
  function MyBase:doItBase(val)
    return val * 1
  end

  function MyBase:doIt(val)
    return val * 2
  end

  local MyClass = class.Class(MyBase)
  function MyClass:doIt(val)
    return val * 3
  end

  function MyClass:doItAlt(val)
    return self:doItBase(val)
  end

  local myInstance = MyClass()
  local obj = mock(myInstance)
  asserts.EQ(obj:doItBase(5), nil)
  asserts.EQ(obj:doIt(5), nil)
  verify(obj):doItBase(5)
  verify(obj):doIt(5)
  when(obj):doIt(numberType()).thenCallRealMethod()

  asserts.EQ(obj:doIt(1), 3)
  verify(obj):doIt(1)

  when(obj):doItAlt(numberType()).thenCallRealMethod()
  asserts.EQ(obj:doItAlt(7), nil)
  verify(obj):doItAlt(7)
  verify(obj):doItBase(7)
  when(obj):doItBase(numberType()).thenCallRealMethod()
  asserts.EQ(obj:doItAlt(9), 9)
  verify(obj):doItAlt(9)
  verify(obj):doItBase(9)
  verifyNoMoreInteractions(obj)
end

function tests.mockUserData()
  local random = mocking.mockLibrary('system.random')
  asserts.EQ(random:uniformReal(0, 1), nil)
  verify(random):uniformReal(0, 1)
  when(random):uniformReal(0, 1)
      .thenReturn(20)
      .thenReturn(1)
      .thenCallRealMethod()
  asserts.EQ(random:uniformReal(0, 1), 20)
  asserts.EQ(random:uniformReal(0, 1), 1)
  asserts.LT(random:uniformReal(0, 1), 1)
  verify(random):uniformReal(0, 1)
  verify(random):uniformReal(0, 1)
  verify(random):uniformReal(0, 1)
  verifyNoMoreInteractions(random)
end

function tests.mockUserData()
  local random = mocking.mockLibrary('system.random')
  asserts.EQ(random:uniformReal(0, 1), nil)
  verify(random):uniformReal(0, 1)
  when(random):uniformReal(0, 1)
      .thenReturn(20)
      .thenReturn(1)
      .thenCallRealMethod()
  asserts.EQ(random:uniformReal(0, 1), 20)
  asserts.EQ(random:uniformReal(0, 1), 1)
  asserts.LT(random:uniformReal(0, 1), 1)
  verify(random):uniformReal(0, 1)
  verify(random):uniformReal(0, 1)
  verify(random):uniformReal(0, 1)
  verifyNoMoreInteractions(random)
end

function tests.spyUserData()
  local random = mocking.spyLibrary('system.random')
  asserts.that(random:uniformReal(0, 1), args.all(args.ge(0), args.lt(1)))
  verify(random):uniformReal(0, 1)
  when(random):uniformReal(0, 1)
      .thenReturn(20)
      .thenReturn(1)
      .thenCallRealMethod()
  asserts.EQ(random:uniformReal(0, 1), 20)
  asserts.EQ(random:uniformReal(0, 1), 1)
  asserts.LT(random:uniformReal(0, 1), 1)
  verify(random):uniformReal(0, 1)
  verify(random):uniformReal(0, 1)
  verify(random):uniformReal(0, 1)
  verifyNoMoreInteractions(random)
end

return test_runner.run(tests)
