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

local strings = {}
local _tostring = tostring

-- Returns an array of strings split according to single character separator.
-- Skips empty fields.
function strings.split(str, sep)
  local words = {}
  for word in string.gmatch(str, '([^' .. sep .. ']+)') do
    words[#words + 1] = word
  end
  return words
end

function strings.join(sep, seq)
  local result = ''
  -- Ensure nils are printed.
  for i = 1, #seq do
    local v = _tostring(seq[i])
    if i > 1 then
      result = result .. sep .. v
    else
      result = v
    end
  end
  return result
end

function strings.quote(str)
  if type(str) == 'string' then
    return '"' .. str .. '"'
  else
    return _tostring(str)
  end
end

--[[ Converts from a string to its most likely type.

Numbers are returned as numbers.
"true" and "false" are returned as booleans.
Everything else is unchanged.
]]
function strings.convertFrom(str)
  local number = tonumber(str)
  if number ~= nil then
    return number
  end
  if str == "true" then
    return true
  end
  if str == "false" then
    return false
  end
  return str
end

return strings
