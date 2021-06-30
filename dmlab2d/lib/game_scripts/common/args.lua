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

--[[ Argument type checking.

Strict type checking of arguments passed to a function.

```lua
argsOut = args.parse(argsIn, {
    {'named1', [args.<default>,] [args.<constraint1>, args.<constraint2> ...]},
    {'named2', [args.<default>,] [args.<constraint1>, args.<constraint2> ...]},
    {'named3', [args.<default>,] [args.<constraint1>, args.<constraint2> ...]},
})
```

If all constraints are valid then argsOut contains values from args in and
defaults values for missing values.

Constraints fail if no default is provided and argsIn does not contain it or
when the value does that is provided does not match all constraints provided.

## Available defaults:

*   `args.default(val)` - Uses val if no value is provided (may be nil).
*   `args.lazy(func)` - Uses result of calling func if no value is provided.

## Available constraints:

### Value:

*   `args.ge(val)` - `arg >= val`
*   `args.le(val)` - `arg <= val`
*   `args.gt(val)` - `arg > val`
*   `args.lt(val)` - `arg < val`
*   `args.ne(val)` - `arg ~= val`
*   `args.positive` - `arg > 0`
*   `args.negative` - `arg < 0`
*   `args.nonNegative` - `arg >= 0`
*   `args.nonPositive` - `arg <= 0`
*   `args.nonZero` - `arg ~= 0`
*   `args.oneOf(...)` - `arg` must be one of `{...}`
*   `args.hasSubstr(substr)` - `arg` must be a string and contain `substr`.

### Type:

*   `args.stringType` - `type(arg) == 'string'`
*   `args.numberType` - `type(arg) == 'string'`
*   `args.booleanType` - `type(arg) == 'boolean'`
*   `args.tableType` - `type(arg) == 'table'`
*   `args.functionType` - `type(arg) == 'function'`
*   `args.userdataType` - `type(arg) == 'userdata'`
*   `args.classIs(cls)` - `arg` must be an instance of `cls`
*   `args.instance(cls)` - `arg` must ben an instance of `cls` or derrived class

### Meta:

*   `args.any(...)` - `arg` must fulfil at least one constraint.
*   `args.all(...)` - `arg` must fulfil all constraints.
*   `args.tableOf(...)` - each values of `arg` must fulfil all constraints.

### Custom:

*   `args.conditional(check, err)` - `check(arg)` must be `true`.

## Examples:

### For kwargs (keyword arguments):

```lua
local function myKwargsFunc(kwargs)
  kwargs = args.parse(kwargs, {
      {'a', args.numberType},
      {'b', args.numberType},
      {'c', args.default(10)},
  })
  -- Used validated kwargs.a, kwargs.b and kwargs.c.
end
```

### For positional arguments:

```lua
local function myPosFunc(...)
  local a, b, c = unpack(args.parse({...}, {
      {1, args.numberType},
      {2, args.numberType},
      {3, args.default(10)},
  }))
  -- Use validated positional arguments a, b and c.
end
```
]]

local tables = require 'common.tables'
local strings = require 'common.strings'

local args = {}

local function fail(message, depth)
  error(message, depth + 1)
end

local _ERROR_DEPTH = 2

local _isCheck = {
    __call = function(self, val)
      if self.check(val) then
        fail(self.err(val), _ERROR_DEPTH)
      end
    end,
    __tostring = function(self)
      return self.usage() or 'invalid tostring'
    end
}

local _isDefault = {
    __call = function(self)
      return self.val()
    end,
    __tostring = function(self)
      return self.usage()
    end
}


local tostringOneLine = tables.tostringOneLine
local repr = strings.quote

local function wrapUsage(usage)
  assert(usage ~= nil)
  local capture = usage
  if type(capture) ~= 'function' then
    return function()
      return capture
    end
  end
  return capture
end

local function makeCheck(func, err, usage)
  local capErr = err
  local errFunc = err
  if type(err) ~= 'function' then
    errFunc = function(val)
      return repr(val) .. ' ' .. capErr
    end
  end
  return setmetatable(
      {check = func, err = errFunc, usage = wrapUsage(usage)},
      _isCheck)
end

local function makeDefault(func, usage)
  return setmetatable(
      {val = func, usage = function()
        return 'default(' .. tostring(func()) .. ')' end},
      _isDefault)
end

--[[ Specifies default value for argument, thereby making the argument optional.

Arguments:

*   `val` : default value (can be nil).
]]
function args.default(val)
  return makeDefault(function() return val end)
end

--[[ Specifies default value for argument, thereby making the argument optional.
Instead of a value, a function is passed that is used to lazily generate the
value when it is required.

Arguments:

*   `valGenerator` (function): function returning default value.
]]
args.lazy = makeDefault

--[[ Constraint verifying that argument is one of a list of given values.

Arguments:

*   `...` : arbitrary number of admissible values for argument.
]]
function args.oneOf(...)
  local vals = {...}
  assert(select('#', ...) > 0,
         'args.oneOf needs to get at least one admissible argument value.')

  local err = function(val)
    return repr(val) .. ' does not match any of ' .. tostringOneLine(vals)
  end
  local usage = function()
    return 'oneof(' .. tostringOneLine(vals) .. ')'
  end
  return makeCheck(
      function(val)
        for _, t in ipairs(vals) do
          if t == val then
            return true
          end
        end
        return false
      end, err, usage)
end

local function typeConstraint(typeName)
  assert(typeName ~= nil)
  return makeCheck(
      function(val)
        return type(val) == typeName
      end,
      function(val)
        return repr(val) .. ' type expected: ' .. repr(typeName) ..
               ', actual: ' .. repr(type(val))
      end,
      typeName .. 'Type')
end

args.typeConstraint = typeConstraint
args.stringType = typeConstraint('string')
args.numberType = typeConstraint('number')
args.booleanType = typeConstraint('boolean')
args.tableType = typeConstraint('table')
args.functionType = typeConstraint('function')
args.userdataType = typeConstraint('userdata')

-- Equal constraint.
function args.eq(v)
  return makeCheck(
      function(val)
        return val == v
      end,
      'is not equal to ' .. repr(v),
      repr(v))
end

-- Greater-or-equal constraint.
function args.ge(v)
  return makeCheck(
      function(val)
        return val >= v
      end,
      'is not greater-equal to ' .. repr(v),
      'ge(' .. repr(v) .. ')')
end

-- Less-or-equal constraint.
function args.le(v)
  return makeCheck(
      function(val)
        return val <= v
      end,
      'is not less-equal to ' .. repr(v),
      'le(' .. repr(v) .. ')')
end

-- Greater-than constraint.
function args.gt(v)
  return makeCheck(
      function(val)
        return val > v
      end,
      'is not greater-than ' .. repr(v),
      'gt(' .. repr(v) .. ')')
end

-- Less-than constraint.
function args.lt(v)
  return makeCheck(
      function(val)
        return val < v
      end,
      'is not less-than ' .. repr(v),
      'lt(' .. repr(v) .. ')')
end

-- Not-equal constraint.
function args.ne(v)
  return makeCheck(
      function(val)
        return val ~= v
      end,
      'is equal to ' .. repr(v),
      'ne(' .. repr(v) .. ')')
end

function args.anyValue()
  return makeCheck(
      function(val)
        return true
      end,
      'Always Valid',
      'any()')
end

args.positive = args.gt(0)
args.negative = args.lt(0)
args.nonNegative = args.ge(0)
args.nonPositive = args.le(0)
args.nonZero = args.ne(0)
args._ = args.anyValue()


-- Has sub-string constraint.
function args.hasSubstr(substr)
  assert(type(substr) == 'string', '`substr` must be a string.')
  return makeCheck(
      function(val)
        return type(val) == 'string' and val:find(substr, nil, true) ~= nil
      end,
      'does not contain ' .. repr(substr),
      'hasSubstr(' .. repr(substr) .. ')')
end

--[[ Constraint that checks that the argument is a table of values which all
satisfy a given set of constraints.

Arguments:

*   `...` (variable number of args constraints) : component constraints which
    all have to be satisfied for each of the entries of the table-valued
    argument. Constraints are not allowed to make the argument optional (i.e.
    `args.default` and `args.oneOf(nil, ...)` cannot be used).
]]
function args.tableOf(...)
  local constraints = args.all(...)
  return makeCheck(
      function(val)
        for _, item in ipairs(val) do
          if not constraints.check(item) then
            return false
          end
        end
        return true
      end,
      function(val)
        for i, item in ipairs(val) do
          if not constraints.check(item) then
            return 'tableOf [' .. i .. ']: ' .. constraints.err(item)
          end
        end
      end,
      function()
        return 'tableOf(' .. constraints.usage() .. ')'
      end
  )
end

function args.instance(cls)
  assert(type(cls) == 'table', 'cls must be a table')
  return makeCheck(
      function(val)
        local meta = getmetatable(val)
        if not meta then
          return false
        end
        local valCls = meta.__index
        for i = 1, 100 do
          if valCls == nil then
            return false
          elseif valCls == cls then
            return true
          end
          valCls = valCls.Base
        end
        return false
      end,
      'instance not derrived from class.',
      'instance(' .. repr(cls) .. ')')
end

function args.classIs(cls)
  assert(type(cls) == 'table', 'cls must be a table')
  return makeCheck(
      function(val)
        local meta = getmetatable(val)
        return meta and meta.__index == cls
      end,
      'classIs incorrect instance class.',
      'classIs(' .. repr(cls) .. ')')
end

--[[ Constraint generator turning boolean function into argument constraint.

Arguments:

*   `fn` (boolean-returning function) : to be used for verifying constraint
    validity. If the function returns a string alongside `false` in case of
    failure, this will be appended to the error message.
*   `errmsg` (optional string or string-returning function, default `''`) :
    error message to be displayed when checked argument fails the constraint.

Returns: args constraint to be used for argument specification
]]
function args.conditional(fn, errmsg, usage)
  errmsg = errmsg or 'Error custom function'
  usage = usage or 'Custom Func'
  assert(type(fn) == 'function',
         'First argument to args.conditional has to be a (boolean) function.')
  assert(type(errmsg) == 'string' or type(errmsg) == 'function',
         'Second argument to args.conditional has to be a string or function.')
  return makeCheck(function(val)
    return fn(val)
  end, errmsg, usage)
end

--[[ Meta-constraint that checks that at least one of its component constraints
is satisfied.

Arguments:

*   `...` (variable number of args constraints) : component constraints (at
    least) one of which has to be satisfied.

Returns: args constraint representing the logical disjunction ('or') of its
component constraints.
]]
function args.any(...)
  assert(select('#', ...) > 0,
         'args.oneOf needs to get at least one admissible argument value.')
  local constraints = {...}
  return makeCheck(
      function(val)
        for _, t in ipairs(constraints) do
          if getmetatable(t) == _isCheck then
            if t.check(val) then
              return true
            end
          end
        end
        return false
      end,
      function(val)
          local message = ''
          if #constraints > 1 then
            message = 'Invalid any constraint:\n  '
          end
          for i, t in ipairs(constraints) do
            if getmetatable(t) == _isCheck then
              if i > 1 then
                message = message .. '\n  '
              end
              message = message .. t.err(val)
            end
          end
          return message
      end,
      function()
        if #constraints ~= 1 then
          return 'any(' .. tostringOneLine(constraints) .. ')'
        else
          return tostringOneLine(constraints[1])
        end
      end)
end

--[[ Meta-constraint that checks that all of its component constraints
are satisfied.

Arguments:

*   `...` (variable number of args constraints) : component constraints all of
    which have to be satisfied.

Returns: args constraint representing the logical conjunction ('and') of its
component constraints.
]]
function args.all(...)
  assert(select('#', ...) > 0,
         'args.all expects at least one args constraint.')
  local constraints = {...}
  return makeCheck(
      function(val)
        for _, t in ipairs(constraints) do
          if getmetatable(t) == _isCheck and not t.check(val) then
            return false
          end
        end
        return true
      end,
      function(val)
        for _, t in ipairs(constraints) do
          if getmetatable(t) == _isCheck and not t.check(val) then
            return t.err(val)
          end
        end
        fail('Internal error: mismatch in check and error message.')
      end,
      function()
        if #constraints ~= 1 then
          return 'all(' .. tostringOneLine(constraints) .. ')'
        else
          return tostringOneLine(constraints[1])
        end
      end)
end

-- Returns has default value, value.
local function hasDefaultAndValue(arg)
  local hasDefault = false
  for _, t in ipairs(arg) do
    if getmetatable(t) == _isDefault then
      hasDefault = true
      local val = t.val()
      if val ~= nil then return true, val end
    end
  end
  return hasDefault, nil
end

-- Helper function to return usage of table of arguments.
local function usageString(arguments, optional)
  local usage
  if optional then
    usage = {optional .. ' valid arguments:'}
  else
    usage = {'Valid arguments:'}
  end
  for _, arg in ipairs(arguments) do
    local name = arg[1]
    local hasDefault = hasDefaultAndValue(arg)
    local infostr = hasDefault and '' or 'required: '
    for idx = 2, #arg do
      if idx > 2 then
        infostr = infostr .. ', '
      end
      infostr = infostr .. tostringOneLine(arg[idx])
    end
    table.insert(usage, string.format('   %s (%s)', tostring(name), infostr))
  end
  return table.concat(usage, '\n')
end

local function failUsage(message, arguments, optional, depth)
  local err = string.format('%s\n%s', message, usageString(arguments, optional))
  error(err, depth + 1)
end

local function ofNameType(x)
  return type(x) == 'string' or type(x) == 'number'
end

local function makeLookup(constraintLists, optional, errorDepth)
  local lookup = {}
  for i, constraintList in ipairs(constraintLists) do
    local name = constraintList[1]
    if not ofNameType(name) then
      failUsage(string.format(
          'Arg %d - %s is not a string or number.', i, repr(name)),
          constraintLists, optional, errorDepth)
    end
    if lookup[name] ~= nil then
      failUsage('Duplicate name found: "' .. name .. '"',
          constraintLists, optional, errorDepth)
    end
    lookup[name] = i
  end
  return lookup
end

local function parseInto(result, opts, constraintLists, optional, errorDepth)
  local named = {}
  local visited = {}
  errorDepth = errorDepth or _ERROR_DEPTH
  local lookup = makeLookup(constraintLists, optional, errorDepth + 1)
  -- Read args in.
  for name, v in pairs(opts or {}) do
    local constraintIdx = lookup[name]
    if constraintIdx == nil then
      failUsage(string.format("Unsupported argument '%s'", tostring(name)),
          constraintLists, optional, errorDepth)
    end
    visited[name] = true
    local constraintList = constraintLists[constraintIdx]

    for i = 2, #constraintList do
      local constraint = constraintList[i]
      local meta = getmetatable(constraint)
      if meta == _isCheck then
        local success = constraint.check(v)
        if not success then
          failUsage('Error parsing ' .. repr(name) .. ': ' .. constraint.err(v),
              constraintLists, optional, errorDepth)
        end
      end
    end
    result[name] = v
  end
  -- Read defaults.
  for i, constraintList in ipairs(constraintLists) do
    local name = constraintList[1]
    if not visited[name] then
      local hasDefault, value = hasDefaultAndValue(constraintList)
      if not hasDefault then
        failUsage(
            string.format("Required argument '%s' missing", tostring(name)),
            constraintLists, optional, errorDepth)
      end
      result[name] = value
    end
  end
  return result
end

--[[ Strict parsing of named argument table (error for unknown arguments).

Verifies that all required arguments are present, fills optional
arguments with default values, and ensures all value constraints are satisfied.
The constraints of each argument are applied in order.
Error if there are any unknown arguments.

Arguments:

*   `opts` : table of (named) arguments
*   `constraintLists` : argument description tables
*   `name` : optional name to include with errors.

Returns: copy of table of named arguments with filled-in default values
]]
function args.parse(opts, constraintLists, name)
  return parseInto({}, opts, constraintLists, name, _ERROR_DEPTH + 1)
end

--[[ Strict parsing of named argument table (error for unknown arguments).

Verifies that all required arguments are present, fills optional arguments with
default values, and ensures all value constraints are satisfied. The constraints
of each argument are applied in order. Error if there are any unknown arguments.

Arguments:

*   `result` : table of arguments to write into.
*   `opts` : table of (named) arguments
*   `constraintLists` : argument description tables
*   `name` : optional name to include with errors.

Returns: copy of table of named arguments with filled-in default values
]]
function args.parseInto(result, opts, constraintLists, name)
  return parseInto(result, opts, constraintLists, name, _ERROR_DEPTH + 1)
end

function args.makeCheck(val)
  if getmetatable(val) == _isCheck then
    return val
  end
  return args.eq(val)
end

return setmetatable(args, {
            __index = function(args, key)
              fail('args - invalid constraint "' .. tostring(key) .. '"', 2)
            end,
            __newindex = function(args, key, value)
              fail('args - is readonly "' .. tostring(key) .. '"', 2)
            end})
