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

local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'

local strings = require 'common.strings'

local tests = {}

function tests.convertFromBooleanWorks()
  asserts.EQ(strings.convertFrom('true'), true)
  asserts.EQ(strings.convertFrom('false'), false)
end

function tests.convertFromNilWorks()
  asserts.EQ(strings.convertFrom(nil), nil)
end

function tests.convertFromNumberWorks()
  asserts.EQ(strings.convertFrom("0"), 0)
  asserts.EQ(strings.convertFrom("1"), 1)
  asserts.EQ(strings.convertFrom("-1"), -1)
end

function tests.convertFromStringWorks()
  asserts.EQ(strings.convertFrom(""), "")
  asserts.EQ(strings.convertFrom("random string"), "random string")
  asserts.EQ(strings.convertFrom("5 6"), "5 6")
end

function tests.convertFromTableWorks()
  local random_table = {random_name = 1}
  asserts.EQ(strings.convertFrom(random_table), random_table)
end

function tests.joinWorks()
  asserts.EQ(strings.join(', ', {}), '')
  asserts.EQ(strings.join(', ', {'one'}), 'one')
  asserts.EQ(strings.join(', ', {'1', '2'}), '1, 2')
  asserts.EQ(strings.join(', ', {'1', '2', '3'}), '1, 2, 3')
  asserts.EQ(strings.join(', ', {1, 2, 3}), '1, 2, 3')
end

function tests.quoteWorks(str)
  asserts.EQ(strings.quote('a'), '"a"')
  asserts.EQ(strings.quote('a '), '"a "')
  asserts.EQ(strings.quote(5), '5')
  asserts.EQ(strings.quote(true), 'true')
end

return test_runner.run(tests)
