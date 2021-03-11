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

local strings = require 'common.strings'

local tables = {}
local _tostring = tostring

--[[ Returns a shallow copy of a table.

That is, copies the first level of key value pairs, but does not recurse down
into the table.
]]
function tables.shallowCopy(input)
  if input == nil then return nil end
  assert(type(input) == 'table')
  local output = {}
  for k, v in pairs(input) do
    output[k] = v
  end
  setmetatable(output, getmetatable(input))
  return output
end

--[[ Returns a deep copy of a table.

Performs a deep copy of the key-value pairs of a table, recursing for each
value, depth-first, up to the specified depth, or indefinitely if that parameter
is nil.
]]
function tables.deepCopy(input, depth)
  if type(input) == 'table' then
    if depth == nil or depth > 0 then
      local output = {}
      local below = depth ~= nil and depth - 1 or nil
      for k, v in pairs(input) do
        output[k] = tables.deepCopy(v, below)
      end
      setmetatable(output, tables.deepCopy(getmetatable(input), below))
      return output
    end
  end
  return input
end



local KEYWORDS = {
    ['and'] = true,
    ['break'] = true,
    ['do'] = true,
    ['else'] = true,
    ['elseif'] = true,
    ['end'] = true,
    ['false'] = true,
    ['for'] = true,
    ['function'] = true,
    ['if'] = true,
    ['in'] = true,
    ['local'] = true,
    ['nil'] = true,
    ['not'] = true,
    ['or'] = true,
    ['repeat'] = true,
    ['return'] = true,
    ['then'] = true,
    ['true'] = true,
    ['until'] = true,
    ['while'] = true,
}

local function isKeyIdent(key)
 return not KEYWORDS[key] and string.find(key, "^[%a_][%w_]*$")
end

local function makeKey(key)
  if type(key) == 'string' then
    if isKeyIdent(key) then
      return key
    end
    return '[\'' .. key .. '\']'
  end
  return '[' .. _tostring(key) .. ']'
end

-- Naive recursive pretty-printer.
-- Prints the table hierarchically. Assumes all the keys are simple values.
function tables.tostring(input, spacing, limit)
  limit = limit or 5
  if limit < 0 then
    return ''
  end
  spacing = spacing or ''
  if type(input) == 'table' then
    local meta = getmetatable(input)
    if meta and meta.__tostring and meta.__tostring ~= tables.tostring then
      setmetatable(input, nil)
      local ret = meta.__tostring(input) or 'invalid tostring'
      setmetatable(input, meta)
      return ret
    end
    local res = '{\n'
    for k, v in pairs(input) do
      if type(k) ~= 'string' or string.sub(k, 1, 2) ~= '__' then
        res = res .. spacing .. '  [' .. strings.quote(k) .. '] = ' ..
            tables.tostring(v, spacing .. '  ', limit - 1) .. '\n'
      end
    end
    return res .. spacing .. '}'
  else
    return strings.quote(input)
  end
end


--[[ Naive recursive to string utility result is a single line.

This function does not add new-lines but meta-method __tostring of printed
entities may.

Arguments:

*   input: Value to be printed.
*   skipMetaTostring: Whether to avoid using builtin __tostring meta-method.
    Must be true if being called from input's __tostring meta-method.
*   limit: max recursion depth to print the table.

Returns: (string) one-line representation of the input.
]]
local function tostringOneLine(input, skipMetaTostring, limit)
  limit = limit or 5
  if limit < 0 then
    return '...'
  end
  if type(input) == 'table' then
    if not skipMetaTostring then
      local meta = getmetatable(input)
      if meta and meta.__tostring and meta.__tostring ~= tostringOneLine then
        setmetatable(input, nil)
        local ret = meta.__tostring(input) or 'invalid tostring'
        setmetatable(input, meta)
        return ret
      end
    end
    local count = #input
    local visited = {}
    local res = '{'
    for i, val in ipairs(input) do
      visited[i] = true
      if i > 1 then
        res = res .. ', '
      end
      res = res .. tostringOneLine(val, false, limit - 1)
    end
    for k, val in pairs(input) do
      if not visited[k] and (
          type(k) ~= 'string' or string.sub(k, 1, 2) ~= '__') then
        visited[k] = true
        if count > 0 then
          res = res .. ', '
        end
        count = count + 1
        res = res .. makeKey(k) .. ' = ' ..
            tostringOneLine(val, false, limit - 1)
      end
    end
    return res .. '}'
  end
  return strings.quote(input)
end

tables.tostringOneLine = tostringOneLine

--[[ Converts from a string to its most likely type.

Numbers are returned as numbers.
"true" and "false" are returned as booleans.
Everything else is unchanged.
]]
function tables.fromString(input)
  local number = tonumber(input)
  if number ~= nil then
    return number
  end
  if input == "true" then
    return true
  end
  if input == "false" then
    return false
  end
  return input
end

--[[ Iterates dictionary in sorted key order.

Arguments:

*   'dict' (table) Table to be iterated.
*   'sorter' (function) Optional sorting function.

Raises an error if keys are not comparable.
]]
function tables.pairsByKeys(dict, sorter)
  local a = {}
  for n in pairs(dict) do a[#a + 1] = n end
  table.sort(a, sorter)
  local i = 0
  return function()
    i = i + 1
    local k = a[i]
    if k then
      return k, dict[k]
    else
      return nil
    end
  end
end

local function _flattenWithPrefix(nestedTable, prefix, visited, outTable)
  for k, v in pairs(nestedTable) do
    local newKey = prefix .. tostring(k)
    local valType = type(v)
    if valType == 'table' then
      if not visited[v] then
        visited[v] = true
        _flattenWithPrefix(v, newKey .. '.', visited, outTable)
      end
    else
      outTable[newKey] = v
    end
  end
end

--[[ Flattens a nested table into '.' separated key - values pairs.

Arguments:

*   'nestedTable' (table) Table to be flattend.

Returns:

A flat map for string key to value. E.g.:

```lua
{
    sub00 = {
        a = 'hello',
        b = 10,
    },
    sub01 = {1, 2, 3},
}

Will become:
{
    ['sub00.a'] = 'hello',
    ['sub00.b'] = 10,
    ['sub01.1'] = 1,
    ['sub01.2'] = 2,
    ['sub01.3'] = 3,
}
```
]]
function tables.flatten(nestedTable)
  local flatTable = {}
  _flattenWithPrefix(nestedTable, '', {[nestedTable] = true}, flatTable)
  return flatTable
end

local function _unpackRecursive(values, count, i)
  if i < count then
    return values[i], _unpackRecursive(values, count, i + 1)
  end
  return values[i]
end

-- [[ Unpacks a table with an optional count.
function tables.unpack(values, count, startIndex)
  startIndex = startIndex or 1
  count = count or #values
  if count >= startIndex then
    return _unpackRecursive(values, count, startIndex)
  end
end

return tables
