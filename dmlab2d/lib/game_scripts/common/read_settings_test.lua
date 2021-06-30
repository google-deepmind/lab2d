--[[ Copyright (C) 2019 The DMLab2D Authors.

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

local read_settings = require 'common.read_settings'

local tests = {}

function tests.anyWorks()
  local myAny = read_settings.any()
  myAny.house = 10
  asserts.tablesEQ(myAny, {house = 10})
  myAny = read_settings.any()
  myAny.house.cat = 10
  asserts.tablesEQ(myAny, {house = {cat = 10}})
  myAny = read_settings.any()
  myAny[1] = 10
  myAny[2] = 20
  myAny[3].cat = 30
  asserts.tablesEQ(myAny, {10, 20, {cat = 30}})
end

function tests.anyDoesntRaise()
  local settings = {
      a = read_settings.any(),
      b = read_settings.any(),
      c = read_settings.any(),
  }
  local kwargs = {
      ['a.1'] = 1,
      ['b.1'] = 2,
      ['b.2'] = 3,
      ['c.cat.m'] = 4,
      ['c.cat.n'] = 5,
  }
  read_settings.apply(kwargs, settings)
  asserts.tablesEQ(settings, {a = {1}, b = {2, 3}, c = {cat = {m = 4, n = 5}}})
end

function tests.defaultTableWorks()
  local myDefault = read_settings.default({
      a = 10,
      b = 20,
      c = {c1 = 30, c2 = 40},
  })

  asserts.tablesEQ(myDefault[1], {
      a = 10,
      b = 20,
      c = {c1 = 30, c2 = 40},
  })
  asserts.tablesEQ(myDefault[2], {
      a = 10,
      b = 20,
      c = {c1 = 30, c2 = 40},
  })
  myDefault[1].a = 100
  myDefault[1].b = 200
  myDefault[1].c.c1 = 300
  myDefault[1].c.c2 = 400
  asserts.tablesEQ(myDefault[1], {
      a = 100,
      b = 200,
      c = {c1 = 300, c2 = 400},
  })
  asserts.tablesEQ(myDefault[2], {
      a = 10,
      b = 20,
      c = {c1 = 30, c2 = 40},
  })
  asserts.tablesEQ(myDefault['anything'], {
      a = 10,
      b = 20,
      c = {c1 = 30, c2 = 40},
  })
end

function tests.defaultValueWorks()
  local myDefault = read_settings.default('hello')
  asserts.EQ(myDefault[1], 'hello')
  asserts.EQ(myDefault[2], 'hello')
  myDefault[1] = 'world'
  asserts.EQ(myDefault[1], 'world')
  asserts.EQ(myDefault[2], 'hello')
  asserts.EQ(myDefault['anything'], 'hello')
end

function tests.applyWorks()
  local settings = {
      a = 10,
      b = 20,
      c = {c1 = 30, c2 = 40}
  }
  local kwargs = {
      ['a'] = 100,
      ['b'] = 200,
      ['c.c1'] = 300,
      ['c.c2'] = 400,
  }
  read_settings.apply(kwargs, settings)
  asserts.tablesEQ(settings, {
      a = 100,
      b = 200,
      c = {c1 = 300, c2 = 400},
  })
end

function tests.applyWorksPartial()
  local settings = {
      a = 10,
      b = 20,
      c = {c1 = 30, c2 = 40}
  }
  local kwargs = {['c.c2'] = 400}
  read_settings.apply(kwargs, settings)
  asserts.tablesEQ(settings, {
      a = 10,
      b = 20,
      c = {c1 = 30, c2 = 400},
  })
end

function tests.missingKeyRaises()
    local settings = {
        a = 10,
        b = 20,
    c = {c1 = 30, c2 = 40}
  }
  local kwargs = {['badKey'] = 400}
  asserts.shouldFailWithMessage(
      function() read_settings.apply(kwargs, settings) end,
      {'badKey', '400'})
end

return test_runner.run(tests)
