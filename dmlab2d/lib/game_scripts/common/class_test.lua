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

local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local class = require 'common.class'
local args = require 'common.args'

local tests = {}

function tests.createClassWithoutInit()
  local MyClass = class.Class()

  function MyClass:get2()
    return 2
  end

  local myInstance = MyClass()
  asserts.EQ(myInstance:get2(), 2)
end

function tests.initCalled()
  local MyClass = class.Class()
  function MyClass:__init__()
    self._arg = 3
  end

  function MyClass:getArg()
    return self._arg
  end

  local myInstance = MyClass()
  asserts.EQ(myInstance:getArg(), 3)
end

function tests.methodsCanHaveArgs()
  local MyClass = class.Class()

  function MyClass:addFive(value)
    return 5 + value
  end

  local myInstance = MyClass()
  asserts.EQ(myInstance:addFive(0), 5)
  asserts.EQ(myInstance:addFive(1), 6)
end

function tests.initCanCallMethods()
  local MyClass = class.Class()
  function MyClass:__init__()
    self._arg = 3 + self.getArg2()
  end

  function MyClass:getArg2()
    return 2
  end

  function MyClass:addFive(value)
    return self._arg + value
  end

  local myInstance = MyClass()
  asserts.EQ(myInstance:addFive(0), 5)
  asserts.EQ(myInstance:addFive(1), 6)
end

function tests.argsForwarded()
  local MyClass = class.Class()
  function MyClass:__init__(arg0, arg1)
    self._arg0 = arg0
    self._arg1 = arg1
  end

  function MyClass:getArg0()
    return self._arg0
  end

  function MyClass:getArg1()
    return self._arg1
  end

  local myInstance = MyClass(0, 1)
  asserts.EQ(myInstance:getArg0(), 0)
  asserts.EQ(myInstance:getArg1(), 1)
end

function tests.exceptionOnInit()
  local MyClass = class.Class()
  function MyClass:__init__(arg0)
    error(arg0)
  end

  asserts.shouldFailWithMessage(function()
      local myInstance = MyClass('hello')
    end, 'hello')
end

function tests.testBaseClassMethod()
  local MyBaseClass = class.Class()

  function MyBaseClass:addFive(arg)
    return arg + 5
  end

  local MyClass = class.Class(MyBaseClass)

  function MyClass:addSix(arg)
    return self:addFive(arg) + 1
  end


  local myInstance = MyClass()
  asserts.EQ(myInstance:addFive(3), 8)
  asserts.EQ(myInstance:addSix(2), 8)
end

function tests.testBaseClassCall()
  local MyBaseClass = class.Class()

  function MyBaseClass:__init__()
    self._base = 'base'
  end

  local MyClass = class.Class(MyBaseClass)

  function MyClass:__init__()
    MyClass.Base.__init__(self)
    self._class = 'class'
  end

  local myInstance = MyClass()

  asserts.EQ(myInstance._base, 'base')
  asserts.EQ(myInstance._class, 'class')
end


function tests.testBaseClassCall2()
  local MyClass0 = class.Class()

  function MyClass0:__init__()
    self._list = {'class0'}
  end

  local MyClass1 = class.Class(MyClass0)

  function MyClass1:__init__()
    MyClass1.Base.__init__(self)
    table.insert(self._list, 'class1')
  end

  local MyClass2 = class.Class(MyClass1)

  function MyClass2:__init__()
    MyClass2.Base.__init__(self)
    table.insert(self._list, 'class2')
  end

  local myInstance0 = MyClass0()
  asserts.tablesEQ(myInstance0._list, {'class0'})

  local myInstance1 = MyClass1()
  asserts.tablesEQ(myInstance1._list, {'class0', 'class1'})

  local myInstance2 = MyClass2()
  asserts.tablesEQ(myInstance2._list, {'class0', 'class1', 'class2'})
end

function tests.testBaseClassAddMethod()
  local MyBaseClass = class.Class()

  function MyBaseClass:addSome(arg)
    return arg + 5
  end

  local MyClass = class.Class(MyBaseClass)

  function MyClass:addMore(arg)
    return self:addSome(arg) + 1
  end

  local myInstance = MyClass()

  asserts.EQ(myInstance:addMore(4), 10)

  function MyBaseClass:addSome(arg)
    return arg + 4
  end
end

function tests.testMixins()
  local MyMixin = {
      mixinFunctionInit = function(self, arg0, arg1)
        self._arg0 = arg0
        self._arg1 = arg1
      end,
      getArg0 = function(self)
        return self._arg0
      end,
      getArg1 = function(self)
        return self._arg1
      end,
  }

  local MyClass = class.Class():__include__(MyMixin)

  function MyClass:__init__(arg0, arg1)
      self:mixinFunctionInit(arg0, arg1)
  end

  local myInstance = MyClass(2, 3)

  asserts.EQ(myInstance:getArg0(), 2)
  asserts.EQ(myInstance:getArg1(), 3)
end

function tests.testTostring()
  local MyClass = class.Class():__name__('MyClass')
  asserts.EQ(tostring(MyClass), '<class \'MyClass\'>')
end

function tests.testErrorOnCtor()
  local MyClass = class.Class():__name__('MyClass')
  function MyClass:__init__(kwargs)
    kwargs = args.parse(kwargs, {{'a', args.numberType}}, 'MyClass')
  end
  asserts.shouldFail(function() MyClass{a = 'Bad'} end, 'MyClass')
end

return test_runner.run(tests)
