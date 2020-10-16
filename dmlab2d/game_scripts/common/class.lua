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

--[[ Creates a class.

Call the created class to create insances arguments are forwarded to init.

Arguments:

*   Base - optional base class.

Example:

Create a Class.
```
  local MyClass = class.Class()__name__('MyClass')
  function MyClass:__init__(kwargs)
    self._a = kwargs.test
  end

  function MyClass:print(test)
    print(self._a)
  end
```

Use a class:
```
  myInstance = MyClass{test = 10}
  myInstance:print()
```

Base class init is not automatically called but can be via Base.

```
local MyClass = class.Class(MyBaseClass):__name__('MyClass')

function MyClass:__init__()
  MyClass.Base.__init__(self)
  ...
end
```

Also supports Mixins.

```
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

local object = MyClass(2, 3)

asserts.EQ(object:getArg0(), 2)
asserts.EQ(object:getArg1(), 3)
```

See also lua_tests/class_test.lua.
]]
local function Class(Base, ...)
  assert(select('#', ...) == 0, 'Only one base class allowed.')
  local clsMeta = {__index = Base}
  local cls = setmetatable({}, clsMeta)
  local meta = {__index = cls}
  clsMeta.__call = function(_, ...)
      local instance = setmetatable({}, meta)
      if cls.__init__ then cls.__init__(instance, ...) end
      return instance
  end
  cls.Base = Base

  function cls:__include__(mixin)
    for funcName, func in pairs(mixin) do
      cls[funcName] = func
    end
    return cls
  end

  function cls:__name__(name)
    clsMeta.__tostring = function(self)
        return '<class \'' .. name .. '\'>'
    end
    return cls
  end

  return cls
end


return {Class = Class}
