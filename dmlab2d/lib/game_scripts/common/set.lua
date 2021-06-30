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

-- A set is any table where all values are either true or nil. Never assign
-- false value to a set.
local strings = require 'common.strings'

local set = {}

local setMt = {}

-- Returns a ⊆ b.
function setMt.__le(a, b)
  for k in pairs(a) do
    if not b[k] then return false end
  end
  return true
end

-- Returns a ⊂ b.
function setMt.__lt(a, b) return a <= b and not (b <= a) end

-- Returns a == b.
function setMt.__eq(a, b) return a <= b and b <= a end

-- Returns a ∪ b.
function setMt.__add(a, b)
  local result = {}
  for k in pairs(a) do result[k] = true end
  for k in pairs(b) do result[k] = true end
  return setmetatable(result, setMt)
end

-- Returns a - b.
function setMt.__sub(a, b)
  local result = {}
  for k in pairs(a) do
    if not b[k] then result[k] = true end
  end
  return setmetatable(result, setMt)
end

-- Returns a string representation of set.
setMt.__tostring = function(self)
  local result = nil
  for key in pairs(self) do
    result = (result ~= nil and result .. ', ' or '') .. strings.quote(key)
  end
  return 'Set{' .. (result or '') .. '}'
end

-- Makes a set from a list.
function set.Set(list)
  local result = {}
  for _, k in ipairs(list) do
    result[k] = true
  end
  return setmetatable(result, setMt)
end

-- Makes a set from a table keys.
function set.SetFromKeys(tableSet)
  local result = {}
  for k in pairs(tableSet) do
    result[k] = true
  end
  return setmetatable(result, setMt)
end

-- Returns whether possibleSet is a set.
function set.isSet(possibleSet)
  return getmetatable(possibleSet) == setMt
end

-- Returns a copy of theSet.
function set.copy(theSet)
  assert(getmetatable(theSet) == setMt)
  local result = {}
  for k in pairs(theSet) do
    result[k] = true
  end
  return setmetatable(result, setMt)
end

-- Inserts all elements in list into theSet.
function set.insert(theSet, list)
  assert(getmetatable(theSet) == setMt)
  for _, k in ipairs(list) do
    theSet[k] = true
  end
end

-- Returns a sorted list of all elements in theSet.
function set.toSortedList(theSet)
  assert(getmetatable(theSet) == setMt)
  local list = {}
  for k in pairs(theSet) do
    table.insert(list, k)
  end
  table.sort(list)
  return list
end

-- Returns whether theSet is empty.
function set.isEmpty(theSet)
  assert(getmetatable(theSet) == setMt)
  return next(theSet) == nil
end

-- Returns whether a theSet is identical to otherSet.
function set.isSame(theSet, otherSet)
  assert(getmetatable(theSet) == setMt and getmetatable(otherSet) == setMt)
  return theSet == otherSet
end

-- Retuns whether two sets are disjoint.
function set.isDisjoint(theSet, otherSet)
  assert(getmetatable(theSet) == setMt and getmetatable(otherSet) == setMt)
  for k in pairs(otherSet) do
    if theSet[k] then
      return false
    end
  end
  return true
end

-- Returns theSet ⊆ otherSet.
function set.isSubsetOf(theSet, otherSet)
  assert(getmetatable(theSet) == setMt and getmetatable(otherSet) == setMt)
  return theSet <= otherSet
end

-- Returns theSet △ otherSet.
function set.symmetricDifference(theSet, otherSet)
  assert(getmetatable(theSet) == setMt and getmetatable(otherSet) == setMt)
  local result = {}
  for k in pairs(theSet) do
    if not otherSet[k] then
      result[k] = true
    end
  end
  for k in pairs(otherSet) do
    if not theSet[k] then
      result[k] = true
    end
  end
  return setmetatable(result, setMt)
end

-- Returns theSet - otherSet.
function set.difference(theSet, otherSet)
  assert(getmetatable(theSet) == setMt and getmetatable(otherSet) == setMt)
  return theSet - otherSet
end

-- Returns theSet ∩ otherSet.
function set.intersect(theSet, otherSet)
  assert(getmetatable(theSet) == setMt and getmetatable(otherSet) == setMt)
  local result = {}
  for k in pairs(otherSet) do
    if theSet[k] then
      result[k] = true
    end
  end
  return setmetatable(result, setMt)
end

-- Returns theSet ∪ otherSet.
function set.union(theSet, otherSet)
  assert(getmetatable(theSet) == setMt and getmetatable(otherSet) == setMt)
  return theSet + otherSet
end

-- Iterates all elements to return the number of elements in theSet.
-- This operation is slow. Use set.isEmpty(theSet) if possible.
function set.calculateNumberOfElements(theSet)
  assert(getmetatable(theSet) == setMt)
  local count = 0
  for _ in pairs(theSet) do
    count = count + 1
  end
  return count
end

return set
