# File System

```lua
local file_system = require 'system.file_system'
```

Functions for interacting with the file system and parameters passed in
DeepMindLab2DLaunchParams.

Underlying C++ code is in `dmlab2d/system/file_system/lua_file_system.cc`

## `runFiles()``

Returns the runfiles path where level scripts provided by
`DeepMindLab2DLaunchParams::runfiles_path`.

```lua
local file_system = require 'system.file_system'

local runfilesDir = file_system:runFiles()
```

## `loadFileToString(path)`

Returns a string: the contents of the file at `path`.

```lua
local file_system = require 'system.file_system'

FILE_NAME = ...
local content = file_system:loadFileToString(`FILE_NAME`)
```
