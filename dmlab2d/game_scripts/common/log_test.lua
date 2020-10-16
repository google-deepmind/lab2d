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
local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'

local function mockWriter(expectedText)
  local writer = {
    text = '',
    expectedText = false,
    messageWritten = false,
  }

  function writer:write(text)
    if text == expectedText then
      self.expectedText = true
    end
    self.text = self.text .. text
  end

  function writer:flush()
    self.messageWritten = true
  end
  return writer
end

local tests = {}

function tests.info()
  local writer = mockWriter('Hello')
  log.setOutput(writer)
  log.setLogLevel(log.NEVER)
  log.info('Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.ERROR)
  log.info('Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.WARN)
  log.info('Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.INFO)
  log.info('Hello')
  assert(writer.messageWritten and writer.expectedText)

  local writer2 = mockWriter('Hello2')
  log.setOutput(writer2)
  log.setLogLevel(1)
  log.info('Hello2')
  assert(writer2.messageWritten and writer2.expectedText)
end

function tests.warn()
  local writer = mockWriter('Hello')
  log.setOutput(writer)
  log.setLogLevel(log.NEVER)
  log.warn('Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.ERROR)
  log.warn('Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.WARN)
  log.warn('Hello')
  assert(writer.messageWritten and writer.expectedText)

  local writer2 = mockWriter('Hello2')
  log.setOutput(writer2)
  log.setLogLevel(log.INFO)
  log.warn('Hello2')
  assert(writer2.messageWritten and writer2.expectedText)
end

function tests.error()
  local writer = mockWriter('Hello')
  log.setOutput(writer)
  log.setLogLevel(log.NEVER)
  log.error('Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.ERROR)
  log.error('Hello')
  assert(writer.messageWritten and writer.expectedText)

  local writer2 = mockWriter('Hello2')
  log.setOutput(writer2)
  log.setLogLevel(log.WARN)
  log.error('Hello2')
  assert(writer2.messageWritten and writer2.expectedText)
end

function tests.v()
  local writer = mockWriter('Hello')
  log.setOutput(writer)
  log.setLogLevel(log.NEVER)
  log.v(2, 'Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.ERROR)
  log.v(2, 'Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.WARN)
  log.v(2, 'Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(log.INFO)
  log.v(2, 'Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(1)
  log.v(2, 'Hello')
  assert(not writer.messageWritten and not writer.expectedText)

  log.setLogLevel(2)
  log.v(2, 'Hello')
  assert(writer.messageWritten and writer.expectedText)
end

function tests.infoToString()
  local writer = mockWriter('25')
  log.setOutput(writer)
  log.setLogLevel(log.INFO)
  log.info(25)
  assert(writer.messageWritten and writer.expectedText)
end

return test_runner.run(tests)
