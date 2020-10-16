# Tile

```lua
local tile = require 'system.tile'
```

Library for rendering grid-world sprites.

Underlying C++ code is in `dmlab2d/system/tile/...`

## Set

A tile set is an ordered list of sprite names, to be used by the renderer.
A sprite's id is its (0-based) index in this list.

### `tile.Set{names=<list[string]>, shape={width=<number>, height=<number>}}`

To create a tile set the names and shape need to be provided.

```lua
local set = tile.Set{
  names = {'Empty', 'Sprite.N', 'Sprite.E', 'Sprite.S', 'Sprite.W'},
  shape = {width = 8, height = 8},
}
```

### `set:setSprite{name=<string>, image=<ByteTensor>}`

Afterwards, an image may be set for each sprite name.
The folowing sets `'Empty'` to solid green.
```lua
set:setSprite{
    name = 'Empty',
    image = tensor.ByteTensor(width, height, 4):fill{0, 255, 0, 255},
}
```

Multiple sprites with the same prefix may be set at the same time. The following
will set `'Sprite.N'`, `'Sprite.E'`, `'Sprite.S'`, `'Sprite.W'` with solid red.

```lua
set:setSprite{
    name = 'Sprite',
    image = tensor.ByteTensor(width, height, 4):fill{255, 0, 0, 255},
}
```lua

Multiple sprites may also be set with an animation tensor. (frames, h, w, 4).

```lua
set:setSprite{
    name = 'Sprite',
    image = tensor.ByteTensor(4, width, height, 4):fill{255, 0, 0, 255},
}
```

## Scene

A Scene converts a tensor of sprite ids into a texture.


### `tile.Scene{shape={width=<number>, height=<number>}, set=<tile.Set>}`

Creates an RGB image tensor whose size is the `shape` provided multiplied by
the sprite size for sprites in the `set`.

```lua
local set = tile.Set{
  names = {'Empty', 'Sprite.N', 'Sprite.E', 'Sprite.S', 'Sprite.W'},
  shape = {width = 8, height = 8},
}

local scene = tile.Scene{shape = {width=10, height=12}, set=set}
```

The returned scene can be used to render a tensor of sprite ids with
shape `Int32Tensor(12, 10, <numLayers>)`.

### scene:shape()

Returns the shape of the ByteTensor that will be returned by `render`.

```lua
local set = tile.Set{
  names = {'Empty', 'Sprite.N', 'Sprite.E', 'Sprite.S', 'Sprite.W'},
  shape = {width = 8, height = 8},
}

local scene = tile.Scene{shape = {width=10, height=12}, set=set}

local height, width, numChannels = unpack(scene:shape())
assert(height == 8 * 12)
assert(width == 8 * 10)
assert(numChannels == 3)
```

### `scene:render(grid=<Int32Tensor>)`

The `grid` must have the same height and width as the scene, with any
number of layers. All ids must be in the range `[0, #set.names)`.
The layers are converted to sprite textures and blended in order to produce
the final render.

```lua
local tensor = require 'system.tensor'
local tile = require 'system.tile'
local set = tile.Set{names = {'red', 'green', 'blue'},
                     shape = {height = 8, width = 5}}

local image = tensor.ByteTensor(8, 8, 3)
set:setSprite{name = 'red', image = image:fill{255, 0, 0}}
set:setSprite{name = 'green', image = image:fill{0, 255, 0}}
set:setSprite{name = 'blue', image = image:fill{0, 0, 255}}
local scene = tile.Scene{shape = {width = 3, height = 1}, set = set}
local grid = tensor.Int32Tensor{range = {0, 2}}:reshape{1, 3}
local renderedScened = scene:render(grid)
```
