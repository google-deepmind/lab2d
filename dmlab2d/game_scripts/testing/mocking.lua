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

--[[ Lightweight mocking library.

See lua_tests/mocking_test.lua for more examples.

Recommended usage:

```Lua
local asserts = require 'testing.asserts'
local mocking = require 'testing.mocking'

local mock = mocking.mock
local when = mocking.when

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
```

When mocking it might be worth verifying a function was called.

```Lua
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
```

You can also verify the order functions are called across multiple functions.

```Lua
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
```

There are also ways of capturing and constraining arguments. This can be done
during calls or during verification. Capturing arguments match all that is
available in args but values are converted to functions to allow independent
captures.

```Lua
local capture = mocking.capture

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
```
]]

local args = require 'common.args'
local strings = require 'common.strings'
local tables = require 'common.tables'

--[[ Example.]]
local mockTables = setmetatable({}, {__mode = 'k'}) -- Weak keys

local function fail(depth, message, ...)
  error(string.format('Error: ' .. message, ...), depth + 1)
end

local capture = {
    _ = args.anyValue,
    positive = function() return args.gt(0) end,
    negative = function() return args.lt(0) end,
    nonNegative = function() return args.ge(0) end,
    nonPositive = function() return args.le(0) end,
    nonZero = function() return args.ne(0) end,
    stringType = function() return args.typeConstraint('string') end,
    numberType = function() return args.typeConstraint('number') end,
    booleanType = function() return args.typeConstraint('boolean') end,
    tableType = function() return args.typeConstraint('table') end,
    functionType = function() return args.typeConstraint('function') end,
    userdataType = function() return args.typeConstraint('userdata') end,
}

local mtDotDotDot = {
  __tostring = function(self) return tostring(self.matcher) .. '...' end,
  __index = {check = function() return true end},
}

local function _argToCheck(checker)
  if getmetatable(checker) ~= mtDotDotDot then
    checker = args.makeCheck(checker)
    if checker.capture == nil or checker.get == nil then
      local _capture = {}
      local count = 0
      checker.get = function()
        count = count + 1
        return _capture[count]
      end
      checker.capture = function(value)
        table.insert(_capture, value)
      end
    end
  end
  return checker
end


function capture.dotDotDot(matcher)
  local checker = {
      matcher = _argToCheck(matcher) or ''
  }
  local _capture = {}
  local count = 0
  if matcher then
    checker.check = function(...)
      for i = 1, select('#', ...) do
        if not matcher.check(select(i, ...)) then
          return false
        end
      end
      return true
    end
  end
  checker.capture = function(...)
    if matcher then
      for i = 1, select('#', ...) do
        matcher.capture(select(i, ...))
      end
    end
    table.insert(_capture, {args = {...}, count = select('#', ...)})
  end
  checker.get = function()
    count = count + 1
    local result = _capture[count]
    return tables.unpack(result.args, result.count)
  end
  return setmetatable(checker, mtDotDotDot)
end

for name, constraint in pairs(args) do
  local skipArgs = {
      parse = true,
      parseInto = true,
      makeCheck = true,
  }
  if not skipArgs[name] and type(constraint) == 'function' then
    capture[name] = constraint
  end
end

local function _join(sep, seq, count)
  count = count or #seq
  local result = ''
  -- Ensure nils are printed.
  for i = 1, count do
    local v = strings.quote(seq[i])
    if i > 1 then
      result = result .. sep
    end
    result = result .. v
  end
  return result
end

local function _argsToCheck(...)
  local result = {}
  for i = 1, select('#', ...) do
    result[i] = _argToCheck(select(i, ...))
  end
  return result
end

local callRecordMt = {
  __tostring = function(self)
    return self.func .. '(' .. _join(', ', self.args, self.count) .. ')'
  end
}

local function _matches(callRecord, checkArgs)
  if (getmetatable(checkArgs[#checkArgs]) ~= mtDotDotDot and
      #checkArgs ~= callRecord.count) or #checkArgs - 1 > callRecord.count then
    return false
  end
  for i, a in ipairs(checkArgs) do
    if getmetatable(a) == mtDotDotDot then
      if not a.check(tables.unpack(callRecord.args, callRecord.count, i)) then
        return false
      end
    elseif not a.check(callRecord.args[i]) then
      return false
    end
  end
  return true
end

local function _capture(callRecord, checkArgs)
  for i, a in ipairs(checkArgs) do
    if getmetatable(a) ~= mtDotDotDot then
      a.capture(callRecord.args[i])
    else
      a.capture(tables.unpack(callRecord.args, callRecord.count, i))
    end
  end
end

local function _callCallbacks(matcher, callRecord)
  local ret = nil
  local index = #matcher.callRecords + 1
  matcher.callRecords[index] = callRecord
  if index > #matcher._callBacks then
    index = #matcher._callBacks
  end
  if index > 0 then
    return matcher._callBacks[index](
        tables.unpack(callRecord.args, callRecord.count))
  end
end

local whenMt = {
  __index = function(self, func)
    local mockObj = rawget(self, '_mockObj')
    local mockTable = mockTables[mockObj]
    local when = mockTable.whens[func]
    if not when then
      when = {
          matchers = {},
          callRecords = {},
          verifyCount = 0,
      }
      mockTable.whens[func] = when
    end
    local matcher = {
      _name = func,
      _callBacks = {},
      checkArgs = {},
      callRecords = {},
      verify = true
    }
    table.insert(when.matchers, matcher)
    function matcher:__tostring()
      return matcher._name .. '(' .. _join(', ', matcher.checkArgs) .. ')'
    end
    function matcher.thenCall(f)
      table.insert(matcher._callBacks, f)
      return matcher
    end
    function matcher.thenCallRealMethod()
      local realMethod = mockTable.realMethods[func]
      table.insert(matcher._callBacks, function(...)
        local count = select('#', ...)
        local convertedArgs = {}
        for i = 1, count do
          local arg = select(i, ...)
          convertedArgs[i] = arg == mockObj and mockTable.template or arg
        end
        return realMethod(tables.unpack(convertedArgs))
      end)
    end
    function matcher.thenReturn(...)
      local ret = {...}
      local count = select('#', ...)
      table.insert(matcher._callBacks,
          function()
            return tables.unpack(ret, count)
          end)
      return matcher
    end
    function matcher.thenError(message)
      table.insert(matcher._callBacks, function() fail(2, message) end)
      return matcher
    end
    function matcher.ignoreOnVerify(self)
      matcher.verify = false
      return matcher
    end

    mockObj[func] = function(...)
      local callRecord = setmetatable({
          func = func,
          args = {...},
          count = select('#', ...)
      }, callRecordMt)

      -- Match most recent first.
      for i = #when.matchers, 1, -1 do
        local matcher = when.matchers[i]
        local checkArgs = matcher.checkArgs
        if _matches(callRecord, checkArgs) then
          if matcher.verify then
            table.insert(mockTable.callRecords, callRecord)
            table.insert(when.callRecords, callRecord)
          end
          _capture(callRecord, checkArgs)
          return _callCallbacks(matcher, callRecord)
        end
      end
      fail(2, 'No matcher exists for call matcher %q', tostring(callRecord))
      table.insert(mockTable.callRecords, callRecord)
      table.insert(when.callRecords, callRecord)
      return nil
    end

    return function(self2, ...)
      if self2 ~= nil or select('#', ...) > 0 then
        if self2 == self then
          self2 = mockObj
        end
        matcher.checkArgs = _argsToCheck(self2, ...)
      end
      return matcher
    end
  end
}

local verifyMt = {
  __index = function(self, func)
    local mockObj = rawget(self, '_mockObj')
    local mockTable = mockTables[mockObj]
    local when = mockTable.whens[func]
    local counter = rawget(self, '_counter')
    if when == nil then
      fail(2, 'No matching when function call to %q', func)
    end
    return function(self2, ...)
      if not counter.check(#when.callRecords) then
        fail(2, 'Expected call %q to be called %s actually called: %d times.',
             func, tostring(counter), #when.callRecords)
      end
      local checkArgs = {}
      if self2 ~= nil or select('#', ...) > 0 then
        if self2 == self then
           self2 = mockObj
        end
        checkArgs = _argsToCheck(self2, ...)
      end
      when.verifyCount = when.verifyCount + 1
      local callRecord = when.callRecords[when.verifyCount]
      if not callRecord or not _matches(callRecord, checkArgs) then
        fail(2, 'Expected call: %s(%s) actual call: %s',
             func, _join(', ', checkArgs), tostring(callRecord))
      end
      _capture(callRecord, checkArgs)
      return self
    end
  end
}

local function _testAndCaptureCall(call, func, ...)
  local checkArgs = _argsToCheck(...)
  if not call or
      call.func ~= func or
      not _matches(call, checkArgs) then
    local actual = (call and tostring(call)) or 'Not Called!'
    fail(3, 'Expected: call: %s(%s) Actual: call: %s',
         func, _join(', ', checkArgs), actual)
  end
  _capture(call, checkArgs)
end

local inOrderMt = {
  __index = function(self, func)
    local mockObj = rawget(self, '_mockObj')
    local mockTable = mockTables[mockObj]
    return function(...)
      local verifyCount = rawget(self, '_verifyCount')
      verifyCount = verifyCount + 1
      rawset(self, '_verifyCount', verifyCount)
      local callRecord = mockTable.callRecords[verifyCount]
      _testAndCaptureCall(callRecord, func, ...)
      return self
    end
  end
}

local function when(mockObj)
  return setmetatable({_mockObj = mockObj}, whenMt)
end

local function _setTemplate(mockObj, template)
  local mockTable = mockTables[mockObj]
  while template ~= nil do
    if type(template) == 'userdata' then
      template = getmetatable(template).__index
    end
    if type(template) ~= 'table' then
      break
    end
    for func, value in pairs(template) do
      if type(value) == 'function' and mockTable.whens[func] == nil then
        mockTable.realMethods[func] = value
        when(mockObj)[func](capture.dotDotDot())
      end
    end
    template = getmetatable(template)
    if template ~= nil and type(template) == 'table' then
      template = template.__index
    end
  end
end

local function mock(template)
  local mockObj = {}
  if type(template) == 'table' then
    mockObj = template
  end
  local whens = {}
  local callRecords = {}
  local realMethods = {}
  local mockTable = {
    whens = whens,
    callRecords = callRecords,
    realMethods = realMethods,
    template = template or mockObj
  }
  mockTables[mockObj] = mockTable
  _setTemplate(mockObj, template)
  return mockObj
end

local function spy(template)
  local mockObj = mock(template)
  for _, w in pairs(mockTables[mockObj].whens) do
    w.matchers[1].thenCallRealMethod()
  end
end

local function verify(mockObj, counter)
  return setmetatable({
      _mockObj = mockObj,
      _counter = counter or args.gt(0),
  }, verifyMt)
end

local function inOrder(mockObj)
  local mockTable = mockTables[mockObj]
  local verifier = setmetatable({
      _mockObj = mockObj,
      _verifyCount = 0,
  }, inOrderMt)
  return {
    verify = function()
      return verifier
    end,
    verifyNoMoreInteractions = function()
      if verifier._verifyCount ~= #mockTable.callRecords then
        local err = 'missing calls:'
        for i = verifier._verifyCount + 1, #mockTable.callRecords do
          err = err .. '\n' .. tostring(mockTable.callRecords[i])
        end
        fail(2, err)
      end
    end,
  }
end

local function reset(mockObj)
  local mockTable = mockTables[mockObj]
  mockTable.callRecords = {}
  for _, when in pairs(mockTable.whens) do
    when.verifyCount = 0
    when.callRecords = {}
  end
end

local function verifyNoMoreInteractions(mockObj)
  local mockTable = mockTables[mockObj]
  local err = ''
  for func, when in pairs(mockTable.whens) do
    if when.verifyCount ~= #when.callRecords then
      err = err .. string.format(
          '\nFunction %q was called %d times but verified %d times',
          func, #when.callRecords, when.verifyCount)
    end
  end
  if err ~= '' then
    fail(2, 'More interactions found:%s', err)
  end
end

local function mockLibrary(library)
  local realLibrary = require(library)
  if type(realLibrary) == 'table' or type(realLibrary) == 'userdata' then
    local mockedLibrary = mock(realLibrary)
    package.loaded[library] = mockedLibrary
    return mockedLibrary
  end
  fail(2, '%q cannot be mocked because it does not return a table or userdata.',
       library)
end

local function spyLibrary(library)
  local spyLibrary = mockLibrary(library)
  for _, w in pairs(mockTables[spyLibrary].whens) do
    w.matchers[1].thenCallRealMethod()
  end
  return spyLibrary
end

return {
    mock = mock,
    spy = spy,
    when = when,
    reset = reset,
    verify = verify,
    capture = capture,
    inOrder = inOrder,
    mockLibrary = mockLibrary,
    spyLibrary = spyLibrary,
    verifyNoMoreInteractions = verifyNoMoreInteractions,
}
