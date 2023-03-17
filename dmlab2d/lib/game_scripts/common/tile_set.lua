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

local class = require 'common.class'
local image_helpers = require 'common.image_helpers'
local image = require 'system.image'
local tile = require 'system.tile'

--[[ A wrapper around 'system.tile' allowing for more ways of adding sprites.
]]

local TileSet = class.Class()

--[[ Initialises a TileSet.

Arguments:

*   world - system.World.
*   size - KWArgs {width=%width%, height=%height%}.
]]
function TileSet:__init__(world, size)
  self._tileSet = tile.Set{names = world:spriteNames(), shape = size}
  self._size = size
end

--[[ Adds a single colored sprite of the correct size to the tile set.

Arguments:

*   name - Name of sprite to set.
*   color - {R, G, B[, A=255]} Color of sprite where each component is from
    the range [0, 255].
]]
function TileSet:addColor(name, color)
  return self._tileSet:setSprite{
      name = name,
      image = image_helpers.colorToSprite(self._size, color)
  }
end

--[[ Adds a sprite scaling and rotating where needed to the tile set

Arguments:

*   name - Name of sprite to set.
*   sprite - ByteTensor with 3 dims.
*   noRatate - Whether to use the same sprite for each rotation or to generate
    rotations.
]]
function TileSet:setSprite(name, sprite, noRotate)
  local spriteShape = sprite:shape()
  local size = {height = spriteShape[1], width = spriteShape[2]}
  if self._size.height ~= size.height or self._size.width ~= size.width then
    sprite = image.scale(sprite, self._size.height, self._size.width, 'nearest')
  end
  return self._tileSet:setSprite{
      name = name,
      image = noRotate and sprite or image_helpers.generateRotations(sprite)
  }
end

--[[ Adds a sprite scaling and rotating where needed to the tile set

Arguments:

*   name - Name of sprite to set.
*   spritePath - Path to an image file.
*   noRatate - Whether to use the same sprite for each rotation or to generate
    rotations.
]]
function TileSet:addSpritePath(name, spritePath, noRotate)
  self:setSprite(name, image.load(tostring(spritePath)), noRotate)
end

--[[ Adds a sprite to the tile set by using `image_helpers.textToSprite`.

Arguments:

*   name - Name of sprite to set.
*   shape - table containing {text = '', palette = {['*'] = .. }}.
*   paletteOverride - optional palette to override the one given in shape.
]]
function TileSet:addShape(name, shape, paletteOverride)
  local palette = shape.palette
  if paletteOverride then
    palette = {}
    for k, v in pairs(shape.palette) do
      palette[k] = v
    end
    for k, v in pairs(paletteOverride) do
      palette[k] = v
    end
  end

  -- Call `rawget` to prevent creating an entry when inspecting it.
  local star = rawget(palette, '*')
  if self._size.width == 1 and self._size.height == 1 and star then
    return self:addColor(name, star)
  end
  local sprite, size = image_helpers.textToSprite(shape.text, palette)
  self:setSprite(name, sprite, shape.noRotate)
end

function TileSet:set()
  return self._tileSet
end


return {TileSet = TileSet}
