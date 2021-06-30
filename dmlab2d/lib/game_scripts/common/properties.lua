--[[ Copyright (C) 2018-2019 The DMLab2D Authors.

Licensed under the Apache License, Version 2.0 (the 'License');
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an 'AS IS' BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
]]

local strings = require 'common.strings'
local system_properties = require 'system.properties'
local RESULT = system_properties

--[[ Returns the head and tail of keyList.

Examples:

> print(headTailSplit('aaa.bbb.ccc'))
'aaa'    'bbb.ccc'

> print(headTailSplit('aaa'))
'aaa'    nil

> print(headTailSplit('10.12'))
10    '12'

Arguments:

*   keyList - String containing dot (.) separated list of keys.

Returns:

*   Arg1 - First element of keyList (converted to a number if possible).
*   Arg2 - Tail of keyList still dot(.) separated. Returns nil if empty.
]]
local function headTailSplit(keyList)
  local dotIdx = keyList:find('[.]')
  if dotIdx then
    local key = keyList:sub(1, dotIdx - 1)
    key = tonumber(key) or key
    return key, keyList:sub(dotIdx + 1)
  end
  return strings.convertFrom(keyList), nil
end

--[[ Returns value of properties in keyList context.

See implmentation for details.

Arguments:

*   properties - table|function|string
*   keyList - String containing dot (.) separated list of keys.

Returns:

*   String if lookup is successful, RESULT.PERMISSION_DENIED if not not
    readable, otherwise RESULT.NOT_FOUND if not findable. If nil
    is returned it is equivalent to returning RESULT.NOT_FOUND.

Examples:

```lua
-- Look up in tables.
properties = {abc = {a = 1, b = 2, c = 3}}
assert(readProperty(properties, 'abc.b') == '2')
assert(readProperty(properties, 'abc.d') == RESULT.NOT_FOUND)

-- Look up in arrays.
properties = {abc = {'a', 'b', 'c'}}
assert(readProperty(properties, 'abc.2') == 'b')
assert(readProperty(properties, 'abc.0') == RESULT.NOT_FOUND)

-- Look up in functions.
properties = {abc = function(listKeys) return listKeys end}
assert(readProperty(properties, 'abc.hello_world'), 'hello_world')
```
]]
local function readProperty(properties, keyList)
  if properties == nil then
    return RESULT.NOT_FOUND
  end
  local propType = type(properties)
  if keyList == nil or keyList == '' then
    if propType == 'table' then
      return RESULT.PERMISSION_DENIED
    elseif propType == 'function' then
      return properties(keyList)
    else
      return tostring(properties)
    end
  end
  if propType == 'table' then
    local head, tail = headTailSplit(keyList)
    return readProperty(properties[head], tail)
  elseif propType == 'function' then
    return properties(keyList)
  else
    return RESULT.PERMISSION_DENIED
  end
end

--[[ Write value to writable in keyList context.

See implmentation for details.

Arguments:

*   properties - table|function
*   keyList - String containing dot (.) separated list of keys.

Returns:

*   RESULT.SUCCESS if write is successful, RESULT.INVALID_ARGUMENT if value is
    the wrong type, RESULT.PERMISSION_DENIED if the object is not writable or
    RESULT.NOT_FOUND if the property is not found. If nil is returned it is
    equivalent to returning RESULT.NOT_FOUND.

Examples:
```lua
-- Write number.
params = {abc = {a = 1, b = 2, c = 3}}
assert(writeProperty(params, 'abc.b', 22), RESULT.SUCCESS)
assert(writeProperty(params, 'abc.b', 'blah'), RESULT.INVALID_ARGUMENT)
assert(writeProperty(params, 'abc.d', 'blah'), RESULT.NOT_FOUND)

-- Write to array.
params = {abc = {'a', 'b', 'c'}}
assert(writeProperty(params, 'abc.2', 'bb'), RESULT.SUCCESS)
assert(writeProperty(params, 'abc.0'), RESULT.NOT_FOUND)
assert(writeProperty(params, 'abc', ''), RESULT.PERMISSION_DENIED)

-- Write to function
local params2 = {}
params2.abc = {
    add = function(key, value)
      params2.abc[key] = value
      return RESULT.SUCCESS
    end
}
assert(writeProperty(params2, 'abc.add.newKey', 'newValue'), RESULT.SUCCESS)
assert(params2.abc.newKey == 'newValue')
```
]]
local function writeProperty(properties, keyList, value)
  if properties == nil then
    return RESULT.NOT_FOUND
  end
  local t = type(properties)
  if t == 'table' then
    if keyList == nil then
      return RESULT.PERMISSION_DENIED
    end
    local head, tail = headTailSplit(keyList)
    if tail == nil then
      local val = properties[head]
      if val == nil then
        return RESULT.NOT_FOUND
      end
      local tval = type(val)
      if tval == 'function' then
        return pcall(val, value) and RESULT.SUCCESS or RESULT.INVALID_ARGUMENT
      elseif tval == 'string' then
        properties[head] = value
        return RESULT.SUCCESS
      elseif tval == 'number' then
        value = tonumber(value)
        if value ~= nil then
          properties[head] = value
          return RESULT.SUCCESS
        else
          return RESULT.INVALID_ARGUMENT
        end
      elseif tval == 'boolean' then
        if value == 'true' then
          properties[head] = true
          return RESULT.SUCCESS
        elseif value == 'false' then
          properties[head] = false
          return RESULT.SUCCESS
        else
          return RESULT.INVALID_ARGUMENT
        end
      else
        return RESULT.PERMISSION_DENIED
      end
    end
    return writeProperty(properties[head], tail, value)
  elseif t == 'function' then
    return properties(keyList, value) or RESULT.INVALID_ARGUMENT
  else
    return RESULT.PERMISSION_DENIED
  end
end

--[[ Add property at location keyList with value value.

If keyList is split into key1, key2 ... keyN then
properties[key1][key2]...[keyN] is assinged to value.

If properties[key1][key2]...[keyN] already exists or any part is not a table
then the operation is not successful and the value is not assigned. Use
writeProperty instead. Returns whether the operation was successful.
]]
local function addProperty(properties, keyList, value)
  if type(properties) ~= 'table' then
    return false
  end
  local head, tail = headTailSplit(keyList)
  local property = properties[head]
  if tail == nil then
    if property == nil then
      properties[head] = value
      return true
    else
      return false
    end
  else
    if property == nil then
      property = {}
      properties[head] = property
    end
    return addProperty(property, tail, value)
  end
end

local function removeProperty(properties, keyList)
  if type(properties) ~= 'table' then
    return false
  end
  local head, tail = headTailSplit(keyList)
  local property = properties[head]
  if tail == nil then
    if property ~= nil then
      properties[head] = nil
      return true
    else
      return false
    end
  else
    if property == nil then
      return false
    end
    return removeProperty(property, tail)
  end
end

local _readWrite = {}
local _readOnly = {}
local _writeOnly = {}

local function addReadWrite(keyList, property)
  addProperty(_readWrite, keyList, property)
end

local function addReadOnly(keyList, property)
  addProperty(_readOnly, keyList, property)
end

local function addWriteOnly(keyList, property)
  addProperty(_writeOnly, keyList, property)
end

local function removeReadWrite(keyList)
  removeProperty(_readWrite, keyList)
end

local function removeReadOnly(keyList)
  removeProperty(_readOnly, keyList)
end

local function removeWriteOnly(keyList)
  removeProperty(_writeOnly, keyList)
end

local function apiWriteProperty(_, keyList, value)
  local resRW = writeProperty(_readWrite, keyList, value)
  if resRW == RESULT.SUCCESS then
    return resRW
  end
  local resW0 = writeProperty(_writeOnly, keyList, value)
  if resW0 == RESULT.SUCCESS then
    return resW0
  end

  -- If result is not a success return the most specific error message.
  return math.max(resRW or RESULT.NOT_FOUND,
                  resW0 or RESULT.NOT_FOUND)
end

local function apiReadProperty(_, keyList)
  local resRW = readProperty(_readWrite, keyList)
  if resRW ~= nil and type(resRW) ~= 'number' then
    return tostring(resRW)
  end
  local resRO = readProperty(_readOnly, keyList)
  if resRO ~= nil and type(resRO) ~= 'number' then
    return tostring(resRO)
  end

  -- If result is not a success return the most specific error message.
  return math.max(resRW or RESULT.NOT_FOUND,
                  resRO or RESULT.NOT_FOUND)
end

local function apiListProperty(_, keyList, callback)
  local localReadWrite = _readWrite
  local localReadOnly = _readOnly
  local localWriteOnly = _writeOnly
  local prefix = ''
  if keyList == '' then
    keyList = nil
  end
  -- Avoid calling any functions for listing.
  while keyList do
    local head, tail = headTailSplit(keyList)
    prefix = prefix .. head .. '.'
    keyList = tail
    localReadWrite = localReadWrite and
                     type(localReadWrite) == 'table' and
                     localReadWrite[head]
    localReadOnly = localReadOnly and
                     type(localReadOnly) == 'table' and
                     localReadOnly[head]
    localWriteOnly = localWriteOnly and
                     type(localWriteOnly) == 'table' and
                     localWriteOnly[head]
  end
  if localReadWrite == nil and
     localReadOnly == nil and
     localWriteOnly == nil then
     return RESULT.NOT_FOUND
  end
  local status = RESULT.NOT_FOUND
  if localReadWrite and type(localReadWrite) == 'table' then
    status = RESULT.SUCCESS
    for k, v in pairs(localReadWrite) do
      callback(prefix .. k, type(v) == 'table' and 'l' or 'rw')
    end
  end

  if localReadOnly and type(localReadOnly) == 'table' then
    status = RESULT.SUCCESS
    for k, v in pairs(localReadOnly) do
      callback(prefix .. k, type(v) == 'table' and 'l' or 'r')
    end
  end

  if localWriteOnly and type(localWriteOnly) == 'table' then
    status = RESULT.SUCCESS
    for k, v in pairs(localWriteOnly) do
      callback(prefix .. k, type(v) == 'table' and 'l' or 'w')
    end
  end
  return status
end


--[[ Adds writeProperty, readProperty and listProperty to an API.

Arguments:

*   `api` An environment API table without `writeProperty`, `readProperty` and
    `listProperty`. Exposes properties added via calls to `addReadWrite`,
    `addReadOnly` and `addWriteOnly`.
]]
local function decorate(api)
  assert(api.writeProperty == nil, 'Already has writeProperty')
  assert(api.readProperty == nil, 'Already has readProperty')
  assert(api.listProperty == nil, 'Already has listProperty')
  api.writeProperty = apiWriteProperty
  api.readProperty = apiReadProperty
  api.listProperty = apiListProperty
  return api
end

return {
    RESULT = RESULT,
    decorate = decorate,
    addReadWrite = addReadWrite,
    addReadOnly = addReadOnly,
    addWriteOnly = addWriteOnly,
    removeReadWrite = removeReadWrite,
    removeReadOnly = removeReadOnly,
    removeWriteOnly = removeWriteOnly,
}
