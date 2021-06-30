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

local log = require 'common.log'
local strings = require 'common.strings'
local tables = require 'common.tables'
local properties = require 'common.properties'

--[[ Splits key by '.' separator and returns the last sub-key and sub-table.

If a sub-key is not found it returns the sub-key and sub-table where the find
failed and the number of valid characters.

Sub-keys are converted to numbers if possible.

Success Example:
```
nestedTable = {levelA = {levelB = {levelC = {10, 11, 12}}}}
key = 'levelA.levelB.levelC.2'
subTable, subKey = _findLastSubTableAndSubKey(nestedTable, key)
assert(subTable == nestedTable.levelA.levelB.levelC)
assert(subKey == 2)
assert(subTable[subKey] == 11)
```

Failure Example
```
nestedTable = {levelA = {levelB = {levelC = {10, 11, 12}}}}
key = 'levelA.levelB.levelN.2'
subTable, subKey = _findLastSubTableAndSubKey(nestedTable, key)
assert(subTable == nestedTable.levelA.levelB)
assert(subKey == 'levelN')
assert(subTable[subKey] == nil)
```
]]
local function _findLastSubTableAndSubKey(nestedTable, key)
  local subTable = nestedTable
  local prevSubTable = subTable
  local prefixCount = 0
  local lastKey = key
  -- Split string with dots.
  for subKey in key:gmatch('([^.]+)') do
    prevSubTable = subTable
    -- Convert key to integer if possible.
    lastKey = strings.convertFrom(subKey)
    subTable = prevSubTable[lastKey]
    if subTable == nil then
      break
    end
    prefixCount = prefixCount + #subKey + 1
  end
  return prevSubTable, lastKey, prefixCount - 1
end

local _settings = {}
local _kwargs

--[[ Updates `settings` with `kwargs`.

Arguments:

*   `kwargs` - table of settings passed from init. Special setting `logLevel`
    will set the log level of the environment and be removed from `kwargs`.
*   `settings` - Parameters that hold values to be overridden by the `kwargs.
*   `allowMissingSettings` - Whether to fail on an unrecognised setting.
]]
local function apply(kwargs, settings, allowMissingSettings)
  assert(kwargs ~= nil and settings ~= nil)
  _kwargs = kwargs
  _settings = settings
  if kwargs.logLevel then
    log.setLogLevel(kwargs.logLevel)
    kwargs.logLevel = nil
  end
  -- Override known kwargs.
  for k, v in tables.pairsByKeys(kwargs) do
    local subTable, subKey, prefixCount =
        _findLastSubTableAndSubKey(settings, k)
    if subTable[subKey] ~= nil then
      subTable[subKey] = strings.convertFrom(v)
    elseif not allowMissingSettings then
      error(string.format(
          'Invalid setting: %s = "%s", %s is a %s (default: %s)',
          k, v, k:sub(1, prefixCount), type(subTable),
          tables.tostringOneLine(subTable)))
    end
  end
  properties.addReadWrite('settings', settings)
end

--[[ Returns a table which lazily creates a deepcopy of values for missing keys.

```lua
> read_settings = require 'common.read_settings'
> setting = read_settings.default('Hello')
> =setting[1]
Hello
> setting[2] = 'World'
> =setting[1] .. ' ' .. setting[2] .. ' ' .. setting[3]
Hello World Hello
```
]]
local function default(values)
  return setmetatable({['%default'] = values}, {
      __index = function(table, key)
        local result = tables.deepCopy(table['%default'])
        table[key] = result
        return result
      end
  })
end

--[[ Any table of values.

Allows any tree of settings without raising an error.

```lua
  local settings = read_settings.any()
  local kwargs = {
      ['a.1'] = 1,
      ['b.1'] = 2,
      ['b.2'] = 3,
      ['c.cat.m'] = 4,
      ['c.cat.n'] = 5,
  }
  read_settings.apply(kwargs, settings)
  asserts.tablesEQ(settings, {a = {1}, b = {2, 3}, c = {cat = {m = 4, n = 5}}})
```
]]
local function any()
  return setmetatable({}, {
      __index = function(table, key)
        local result = any()
        table[key] = result
        return result
      end,
  })
end

local function settings()
  return _settings
end

local function kwargs()
  return _kwargs
end

return {
    any = any,
    apply = apply,
    settings = settings,
    default = default,
    kwargs = kwargs
}
