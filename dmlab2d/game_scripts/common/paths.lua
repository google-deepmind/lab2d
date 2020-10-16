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

local io = require 'io'

local paths = {}

local function _join(base, path)
  if path:sub(1, 1) == '/' then
    return path
  elseif base:sub(#base) == '/' then
    return base .. path
  else
    return base .. '/' .. path
  end
end

--[[ Joins a path with a base directory.
(Honours leading slash to ignore base directory)

*   paths.join('base', 'path') => 'base/path'
*   paths.join('base/', 'path') => 'base/path'
*   paths.join('base', '/path') => '/path'
*   paths.join('base/', '/path') => '/path'
*   paths.join('a', 'b', 'c', 'd') => 'a/b/c/d'
]]
function paths.join(base, ...)
  for i = 1, select('#', ...) do
    base = _join(base, select(i, ...))
  end
  return base
end

--[[ Returns directory name of path including trailing slash.

*   paths.dirname('file') => ''
*   paths.dirname('/path2') => '/'
*   paths.dirname('path1/path2') => 'path1/'
*   paths.dirname('path1/path2/') => 'path1/path2/'
*   paths.dirname('/path1/path2') => '/path1/'
*   paths.dirname('/path1/path2/') => '/path1/path2/'
*   paths.dirname('~/path1/path2/') => '~/path1/path2/'
*   paths.dirname('~/path1/path2') => '~/path1/'
]]
function paths.dirname(path)
  return path:match(".-/.-") and path:gsub("(.*/)(.*)", "%1") or ''
end

-- Returns whether a file exists. (Must be readable.)
function paths.fileExists(path)
  local file = io.open(path, "r")
  if file ~= nil then
    io.close(file)
    return true
  else
    return false
  end
end

return paths
