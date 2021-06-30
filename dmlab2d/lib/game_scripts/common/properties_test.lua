--[[ Copyright (C) 2019 The DMLab2D Authors.

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

local properties = require 'common.properties'
local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'

local tests = {}

local function _propertyListKey(api, key)
  local list = {}
  local function gather(key, mode)
    list[key] = mode
  end
  asserts.EQ(properties.RESULT.SUCCESS, api:listProperty(key, gather))
  return list
end

function tests.addReadWriteTableWorks()
  local api = {}
  properties.decorate(api)
  local settings = {
      num = 10,
      text = 'hello',
      subTable = {
          subNum = 5,
          subText = 'world'
      }
  }
  properties.addReadWrite('settingsRw', settings)
  local settingsList = _propertyListKey(api, 'settingsRw')
  asserts.tablesEQ(settingsList, {['settingsRw.num'] = 'rw',
                                  ['settingsRw.text'] = 'rw',
                                  ['settingsRw.subTable'] = 'l'})

  local settingsList = _propertyListKey(api, 'settingsRw.subTable')
  asserts.tablesEQ(settingsList, {['settingsRw.subTable.subNum'] = 'rw',
                                  ['settingsRw.subTable.subText'] = 'rw'})

  asserts.EQ(api:readProperty('settingsRw.num'), '10')
  asserts.EQ(api:readProperty('settingsRw.text'), 'hello')
  asserts.EQ(api:readProperty('settingsRw.subTable.subNum'), '5')
  asserts.EQ(api:readProperty('settingsRw.subTable.subText'), 'world')

  asserts.EQ(api:writeProperty('settingsRw.num', '100'),
             properties.RESULT.SUCCESS)
  asserts.EQ(api:writeProperty('settingsRw.text', 'Hello'),
             properties.RESULT.SUCCESS)
  asserts.EQ(api:writeProperty('settingsRw.subTable.subNum', '50'),
             properties.RESULT.SUCCESS)
  asserts.EQ(api:writeProperty('settingsRw.subTable.subText', 'World'),
             properties.RESULT.SUCCESS)

  asserts.tablesEQ(settings, {
          num = 100,
          text = 'Hello',
          subTable = {
              subNum = 50,
              subText = 'World'
          }
      })
  properties.removeReadWrite('settingsRw')
  asserts.EQ(api:writeProperty('settingsRw.num', '100'),
             properties.RESULT.NOT_FOUND)
end

function tests.addWriteOnlyTableWorks()
  local api = {}
  properties.decorate(api)
  local settings = {
      num = 10,
      text = 'hello',
      subTable = {
          subNum = 5,
          subText = 'world'
      }
  }
  properties.addWriteOnly('settingsW', settings)
  local settingsList = _propertyListKey(api, 'settingsW')
  asserts.tablesEQ(settingsList, {['settingsW.num'] = 'w',
                                  ['settingsW.text'] = 'w',
                                  ['settingsW.subTable'] = 'l'})

  local settingsList = _propertyListKey(api, 'settingsW.subTable')
  asserts.tablesEQ(settingsList, {['settingsW.subTable.subNum'] = 'w',
                                  ['settingsW.subTable.subText'] = 'w'})

  asserts.EQ(api:readProperty('settingsW.num'),
             properties.RESULT.NOT_FOUND)
  asserts.EQ(api:readProperty('settingsW.text'),
             properties.RESULT.NOT_FOUND)
  asserts.EQ(api:readProperty('settingsW.subTable.subNum'),
             properties.RESULT.NOT_FOUND)
  asserts.EQ(api:readProperty('settingsW.subTable.subText'),
             properties.RESULT.NOT_FOUND)

  asserts.EQ(api:writeProperty('settingsW.num', '100'),
             properties.RESULT.SUCCESS)
  asserts.EQ(api:writeProperty('settingsW.text', 'Hello'),
             properties.RESULT.SUCCESS)
  asserts.EQ(api:writeProperty('settingsW.subTable.subNum', '50'),
             properties.RESULT.SUCCESS)
  asserts.EQ(api:writeProperty('settingsW.subTable.subText', 'World'),
             properties.RESULT.SUCCESS)

  asserts.tablesEQ(settings, {
          num = 100,
          text = 'Hello',
          subTable = {
              subNum = 50,
              subText = 'World'
          }
      })

  properties.removeWriteOnly('settingsW')
  asserts.EQ(api:writeProperty('settingsW.num', '100'),
             properties.RESULT.NOT_FOUND)
end


function tests.addReadTableWorks()
  local api = {}
  properties.decorate(api)
  local settings = {
      num = 10,
      text = 'hello',
      subTable = {
          subNum = 5,
          subText = 'world'
      }
  }
  properties.addReadOnly('settingsR', settings)
  local settingsList = _propertyListKey(api, 'settingsR')
  asserts.tablesEQ(settingsList, {['settingsR.num'] = 'r',
                                  ['settingsR.text'] = 'r',
                                  ['settingsR.subTable'] = 'l'})

  local settingsList = _propertyListKey(api, 'settingsR.subTable')
  asserts.tablesEQ(settingsList, {['settingsR.subTable.subNum'] = 'r',
                                  ['settingsR.subTable.subText'] = 'r'})

  asserts.EQ(api:readProperty('settingsR.num'), '10')
  asserts.EQ(api:readProperty('settingsR.text'), 'hello')
  asserts.EQ(api:readProperty('settingsR.subTable.subNum'), '5')
  asserts.EQ(api:readProperty('settingsR.subTable.subText'), 'world')

  asserts.EQ(api:writeProperty('settingsR.num', '100'),
             properties.RESULT.NOT_FOUND)
  asserts.EQ(api:writeProperty('settingsR.text', 'Hello'),
             properties.RESULT.NOT_FOUND)
  asserts.EQ(api:writeProperty('settingsR.subTable.subNum', '50'),
             properties.RESULT.NOT_FOUND)
  asserts.EQ(api:writeProperty('settingsR.subTable.subText', 'World'),
             properties.RESULT.NOT_FOUND)

  asserts.tablesEQ(settings, {
          num = 10,
          text = 'hello',
          subTable = {
              subNum = 5,
              subText = 'world'
          }
      })
  properties.removeReadOnly('settingsR')
  asserts.EQ(api:readProperty('settingsR.num'), properties.RESULT.NOT_FOUND)
end

function tests.addWriteFunctionWorks()
  local api = {}
  properties.decorate(api)
  local param0
  local param1

  local function myFuncWrite(inParam0, inParam1)
    param0 = inParam0
    param1 = inParam1
    return properties.RESULT.SUCCESS
  end

  properties.addWriteOnly('myFuncWrite', myFuncWrite)

  asserts.EQ(api:writeProperty('myFuncWrite', '100'),
             properties.RESULT.SUCCESS)
  asserts.EQ(param0, '100')

  api:writeProperty('myFuncWrite.param.other', '100')
  asserts.EQ(param0, 'param.other')
  asserts.EQ(param1, '100')
end

function tests.addReadFunctionWorks()
  local api = {}
  properties.decorate(api)
  local param
  local function myFuncRead(inParam)
    param = inParam
    return 'hello'
  end

  properties.addReadOnly('myFuncRead', myFuncRead)
  asserts.EQ(api:readProperty('myFuncRead.10'), 'hello')
  asserts.EQ(param, '10')
  asserts.EQ(api:readProperty('myFuncRead'), 'hello')
  asserts.EQ(param, nil)
end

return test_runner.run(tests)
