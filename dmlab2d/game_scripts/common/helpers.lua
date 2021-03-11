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

-- Common utilities.
local strings = require 'common.strings'
local paths = require 'common.paths'
local tables = require 'common.tables'

return {
    split = strings.split,
    quoteString = strings.quote,
    fromString = strings.convertFrom,
    pathJoin = paths.join,
    fileExists = paths.fileExists,
    dirname = paths.dirname,
    shallowCopy = tables.shallowCopy,
    deepCopy = tables.deepCopy,
    ['tostring'] = tables.tostring,
    tostringOneLine = tables.tostringOneLine,
    pairsByKeys = tables.pairsByKeys,
    flatten = tables.flatten,
}
