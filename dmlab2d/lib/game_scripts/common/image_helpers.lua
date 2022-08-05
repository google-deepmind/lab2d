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

local tensor = require 'system.tensor'
local image = require 'system.image'
local unpack = _G.unpack or table.unpack  -- Handle different versions of Lua.


--[[ Creates a tensor containing 4 rotated sprites.

Arguments:

*   `sprite` - tensor.ByteTensor(h, w, c)

Returns:

*   tensor.ByteTensor(4, h, w, c) with all four rotations of `sprite`.
]]
local function generateRotations(sprite)
  local transpose = sprite:transpose(1, 2)
  local spriteShape = sprite:shape()
  if spriteShape[1] ~= spriteShape[2] then
    transpose = image.scale(transpose:clone(),
                            spriteShape[1], spriteShape[2], 'nearest')
  end
  local outSprite = tensor.ByteTensor(4, unpack(spriteShape))
  outSprite:select(1, 1):copy(sprite)
  outSprite:select(1, 2):copy(transpose:reverse(2))
  outSprite:select(1, 3):copy(sprite:reverse(1):reverse(2))
  outSprite:select(1, 4):copy(transpose:reverse(1))
  return outSprite
end

--[[ Creates a sprite with a given shape and color.

Arguments:

*   `shape` - Table containing integers `width` and `height` of sprites.
*   `color` - Lua array containing integers in the range [0-255] representing
    the color of the sprite.
]]
local function colorToSprite(shape, color)
  return tensor.ByteTensor(shape.height, shape.width, #color):fill(color)
end

local function _visitText(text, func)
  local stripNewLine = 1
  while text:sub(stripNewLine, stripNewLine) == '\n' do
    stripNewLine = stripNewLine + 1
  end
  local row = 1
  local col = 1
  for i = stripNewLine, #text do
    local c = text:sub(i, i)
    if c == '\n' then
      row = row + 1
      col = 1
    elseif c ~= ' ' then
      func(row, col, c)
      col = col + 1
    end
  end
end

--[[ Creates a sprite from text art and a palette.

Arguments:

*   `text` - New-line separated representation of sprite. Leading and
    trailing new-lines are ignored. Leading white-space is ignored. Values for
    each character are from read from the palette if there.

*   `palette` - Character to value array for each sprite. Each value array must
    be the same.

Returns:

*   ByteTensor - Shape matching text and first entry of palette.

```lua
local image_helpers = require 'common.image_helpers'
local text = "aaaaa\n..b..\nccccc\n....."
local palette = {a = {1, 0, 0}, b = {0, 0, 2}, c = {0, 3, 0}}
local sprite, shape = image_helpers.textToSprite(text, palette)
asserts.tablesEQ(sprite:shape(), {4, 5, 3})
asserts.tablesEQ(shape, {height = 4, width = 5})
asserts.EQ(sprite, tensor.ByteTensor{
    {{1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}, {1, 0, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 2}, {0, 0, 0}, {0, 0, 0}},
    {{0, 3, 0}, {0, 3, 0}, {0, 3, 0}, {0, 3, 0}, {0, 3, 0}},
    {{0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}, {0, 0, 0}}})
```
]]
local function textToSprite(text, palette)
  local colors = 0
  for char, p in pairs(palette) do
    if colors == 0 then
      colors = #p
    elseif #p ~= colors then
      error('All colors must have the same number of channels. ' ..
            'color \'' .. char .. '\' has ' .. #p ..
            ' channels but expected ' .. colors .. ' channels.')
    end
  end

  local width = 0
  local height = 0
  _visitText(text,
    function(row, col)
      width = width > col and width or col
      height = height > row and height or row
    end
  )

  local data = tensor.ByteTensor(height, width, colors)
  _visitText(text,
    function(row, col, char)
      local color = palette[char]
      if color and #color > 0 then
        data(row, col):fill(color)
      else
        error('color \'' .. char .. '\' not in palette')
      end
    end
  )
  return data, {height = height, width = width}
end

return {
    textToSprite = textToSprite,
    generateRotations = generateRotations,
    colorToSprite = colorToSprite,
}
