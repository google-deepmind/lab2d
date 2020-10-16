# Grid World

```lua
local grid_world = require 'system.grid_world'
```

Library for defining and updating grid worlds.

Underlying C++ code is in `dmlab2d/system/grid_world/...`

## World

Contains data that does not change between episodes. It contains hits, states,
layers, groups, sprites, render_order and update_order.

```lua
local grid_world = require 'system.grid_world'

local world = grid_world.World{
    -- Layers are declared implicitly in states, render layers are the order
    -- that the sprites are placed when rendered. Sprites on layers other than
    -- these are not visible.
    renderOrder = {'layer0', 'layer1'},

    -- A fixed number of named update phases. Each updater is called in order
    -- during grid:update(...) and calls the equivalent function in onUpdate.
    updateOrder = {'update0', {name = 'update1', func = 'funcName1'}},

    -- Available beams for players to interact with other players at a distance.
    -- There maybe a sprite associated with them.
    hits = {
        hitName0 = {
            layer = 'hitLayer0',
            sprite = 'AspriteName',
        },
        hitName1 = {
            layer = 'hitLayer0',
            sprite = 'BspriteNameH1',
        },
    },
    -- List of piece states.
    states = {
        state0 = {
            -- Layer the sprite is rendered on if in updateOrder. No two pieces
            -- may be in the same location and layer.
            layer = 'layer0',
            -- Name of sprite to render.
            sprite = '0spriteName',
            -- Groups the state belongs to. All pieces belonging to a group can
            -- be accessed and updated together.
            groups = {'even', 'all'},
            -- When one piece moves to or from the same location as another
            -- piece (the pieces must be on different layers), that other piece
            -- state's onContact is called with contactName and enter/leave.
            contact = 'contactName0',
        },
        state1 = {
            layer = 'layer1',
            sprite = '1spriteName',
            groups = {'odd', 'all'},
            contact = 'contactName1',
        },
        state2 = {
            layer = 'layer2',
            sprite = '2spriteName',
            groups = {'even', 'all'},
            contact = 'contactName2',
        }
    },
    -- This field is for custom sprites that are not associated with a state.
    customSprites = {'customSprite1', 'customSprite2'},
    -- When rendering the level from a certain location there can be locations
    -- out of bounds of the described map. Render them with this sprite.
    outOfBoundsSprite = 'OutOfBounds',
    -- In certain rendering modes there are locations which are in the render
    -- window but out of view for the avatar. Render them with this sprite.
    outOfViewSprite = 'OutOfView',
}
```

### `world:createGrid{}` &rarr; `Grid`, `pieces`

Returns a new grid with specified parameters. The second value `pieces` will
contain the pieces created during grid creation if a `layout` is specified. See
[`grid:createLayout`](#gridcreatelayout-pieces).

```lua
local grid, peices = world:createGrid{
  -- Either:
  stateMap = stateMap,
  layout = layout,
  -- Or:
  size = size,
  --
  stateCallbacks = stateCallbacks
  topology = topology,  -- Optional.
}
```

#### `stateMap` and `layout`

See [`grid:createLayout`](#gridcreatelayout-pieces).

#### `size`

A table in the form `{width = <inputWidth>, height = <inputHeight>}`. An empty
grid is created with `<inputWidth>` by `<inputHeight>`.

#### `topology`

The topology of the grid. Must be one of:

*   `grid_world.TOPOLOGY.BOUNDED` (default) - Pieces may not move outside the
    bounds of the grid and rendered views will render `outOfBoundsSprite`s
    outside of the playable area.
*   `grid_world.TOPOLOGY.TORUS` - The left-right and top-bottom are joined
    together and the rendered view will loop too.

#### `stateCallbacks`

```lua
local stateCallbacks = {
      state0 = {
          onHit = true,  -- Absorbs all beam weapons.
      },
      state1 = {
          onAdd = function(grid, selfPiece) end,
          onRemove = function(grid, selfPiece) end,
          onBlocked = function(grid, selfPiece, blocker) end,
          onContact = {
              contactName1 = {
                  enter = function(grid, selfPiece, otherPiece) end,
                  leave = function(grid, selfPiece, otherPiece) end,
              },
              contactName2 = {
                  enter = function(grid, selfPiece, otherPiece) end,
                  leave = function(grid, selfPiece, otherPiece) end,
              },
          },
          onUpdate = {
              update0 = function(grid, selfPiece, framesOld) end,
              funcName1 = function(grid, selfPiece, framesOld) end,
          },
          onHit = {
              hitName0 = function(grid, selfPiece, hitterPiece)
                return true  --Returns whether the piece absorbs beam.
              end,
              hitName1 = function(grid, selfPiece, hitterPiece)
                return true  --Returns whether the piece absorbs beam.
              end,
          }
      },
  }
```

### `world:spriteNames()` &rarr; `array<string>`

Returns a stable list of sprite names used when rendering grids.

### `world:createView(kwargs)` &rarr; LayerView

Returns an object used for rendering sections of a Grid as layer observations
around a selected cell. It can be used with system.tile_set to create RGB observations.

kwargs:

* `left` (optional int) - number of cells left of selected cell to render.
* `right` (optional int) - number of cells right of selected cell to render.
* `forward`(optional int) - number of cells forward of selected cell to render.
* `backward`(optional int) - number of cells right of selected cell to render.
* `layout` (optional string) - Used instead of `left`, `right`, `forward` and
  `backward`. Calculates right and backwards such that the whole layout would be
  visible if the selected cell is (0, 0).
* `centered` - Whether to center observation such that all rotations do not move
   the selected cell visually.
* `spriteMap` map[string][string] (Must be valid sprite names.) Remaps sprites
  for this layer viewer.

## LayerView

LayerViews are designed to work with system.tile.Scene.

```Lua
  local tile = require 'system.tile'
  local tile_set = require 'common.tile_set'
  -- ...
  local layerView = world:CreateView{left=5, right=5, forward=5, backward=5}
  local set = tile_set(word, {width=8, height=8})
  set:addShape('name', shapeData)
  -- ...
  local rgbScene= tile.Scene{
      shape = layerView:gridSize(),
      set = set,
  }
  -- ...
  -- Scene view observation spec.
  local rgbSceneSpec = {
    name = 'RGB',
    type = 'tensor.ByteTensor',
    shape = rgbScene:shape(),
  }
  -- Render scene using layer view.
  local rgbObservation = rgbScene:render(layerView:observation{grid = grid})
```

### `LayerView::observationSpec(name)`

Returns the observation spec of the Layer view with a given `name`.
The returned value will be an int64 tensor with shape {H, W, L}. Where L is the
number of render layers specified in the world.

### `LayerView::gridSize()`  &rarr; {width=width, height=height}

Returns the size of the grid rendered by the layer view.

### `LayerView::observation(kwargs)`

Returns the rendered view of the grid from specified cell and orientation.

kwargs:

 * `grid` The grid to be rendered. (Must be created from the same world object.)
 * `transform` optional. The selected cell and orientation to render the grid
   from.
 * `piece` optional. The piece handle to read the orientation and position from.
   (If the cell is off grid then the OutOfBounds sprite is rendered everywhere
   instead.)
 * `orientation` optional. Overides player orientation to render from a fixed
   orientation.

If no position or orientation is provided then it is assumed to render from:
(0,0) North.

## Grid

Terms:

*   `position` refers to a grid position of the form {x, y} starting at {0, 0}
    for the left-top and {gridWidth-1, gridHeight-1} for the right-bottom.
*   `orientation` refers to one of `['N', 'E', 'S', 'W']`.
    -   `N` - Piece relative forward or grid relative negative y.
    -   `E` - Piece relative right or grid relative positive x.
    -   `S` - Piece relative backwards or grid relative positive y.
    -   `W` - Piece relative left or grid relative negative x.
*   `turn` refers to one of `[0, 1, 2, 3]` which represent the delta between two
    orientations.
    -   `0` - Turn 0 degrees.
    -   `1` - Turn 90 degrees counterclockwise.
    -   `2` - Turn 180 degrees.
    -   `3` - Turn 90 clockwise.
*   `transform` refers to a piece position and orientation in the form:
    `{pos=position, orientation=orientation}`
*   `piece` refers to a piece created via createLayout or createPiece.
*   `random` random number generator returned by `require 'system.random'``
    seeded at start of episode.

### Grid removal

#### `grid:destroy()`

Forces the destruction of the grid and any references held by it.

```lua
if self._grid then
  self._grid:destroy()
end
self._grid = self._world:createGrid{...}
```

### Piece creation/removal.

#### `grid:createLayout{}` &rarr; `pieces`

```lua
local piecesCreated = grid:createLayout {
  stateMap = stateMap,
  layout = layout,
  offset = offset,
}
```

*   `stateMap`

This a table of a character to `state`. Used by layout for building a grid.

```lua
local stateMap = {
  ['0'] = 'state0',
  ['1'] = 'state1',
  ['2'] = 'state2',
}
```

*   `layout`

A new-line separated string describing the grid layout and shape. New lines and
leading spaces are ignored. Characters not existing in `stateMap` are not
converted to pieces.

```lua
local layout = [[
..0.0.0.0..
.1.1.1.1.1.
2.2.2.2.2.2
]]
```

Assuming room for the pieces to be placed, it will have 4 pieces of state
`state0`, 5 of state `state1` and 6 of state `state2` and `#piecesCreated` will
be 15.

#### `grid:createPiece(state, transform)` &rarr; `piece`

Creates a piece at `transform` if the layer and location are available. Returns
a handle to that piece. Triggers the `onAdd` callback for `state`.

#### `grid:removePiece(pieceHandle)`

Removes a piece. `pieceHandle` is recycled and may appear in future calls to
[`createPiece()`](#gridcreatepiecestate-transform-piece).

### User-State

#### `grid:setUserState(piece, any)`

Stores a reference to `any`, any Lua value/function, with `piece`.

```lua
-- Set table value
local userState = {1, 2, 3}
grid:setUserState(piece, userState)
assert(userState == grid:userState(piece))

-- Set string value
grid:setUserState(piece, "hello")
assert("hello" == grid:userState(piece))

-- Set number value
grid:setUserState(piece, 10)
assert(10 == grid:userState(piece))

-- Clear
grid:setUserState(piece, nil)
assert(nil == grid:userState(piece))
```

#### `grid:userState(piece)` &rarr; `any`

Retrieves user-state associated with `piece` if previously set, otherwise `nil`.

See [`grid:setUserState(piece, any)`](#gridsetuserstatepiece-any).

### Querying

#### `grid:__tostring()` &rarr; `string`

Debug utility for rendering a grid as a string using the first character of the
sprite name for the highest-layer piece state at each location.

#### `grid:transform(piece)` &rarr; `transform`

Returns the transform of a piece.

#### `grid:position(piece)` &rarr; {x, y}

Returns the position of a piece. (Useful if the orientation is not needed.)

#### `grid:state(piece)` &rarr; `string`

Returns the current state of a piece.

#### `grid:layer(piece)` &rarr; `string|nil`

Returns the current layer of a piece or nil if it does not have one.

#### `grid:frames(piece)` &rarr; `Number`

Returns the number of frames a piece has been in its current state.

#### `grid:rayCast(layer, positionStart, positionEnd)` &rarr; hit, piece, position

Returns whether there is a piece on the line between `positionStart` and
`positionEnd` on a given `layer`, not including the start position. If `hit` is
true, the `piece` and `position` return values are the first piece found. In
torus topology `positionEnd` is reached by the shortest route available, using
the negative direction if two routes are equal. The resulting position may not
be normalised.

#### `grid:rayCastDirection(layer, positionStart, direction)` &rarr; hit, piece, offset

Returns whether there is a piece on a line between `positionStart` and
`positionStart + direction` on a given `layer`, not including the start
position, or whether the line is out of bounds. If a piece is found, the `piece`
and `offset` return values are the first piece found and its offset. In torus
topology `rayCastDirection` does not change direction, but the offset is not
normalised.

#### `grid:queryPosition(layer, position)` &rarr; piece or nil

Returns piece at given `layer` or nil.

#### `grid:queryRectangle(layer, positionCorner1, positionCorner2)` &rarr; table\[piece, position\]

Returns a table of all pieces to positions in rectangle on given `layer` between
`positionCorner1` and `positionCorner2` inclusive. In torus topology the
positions returned are not normalised.

#### `grid:queryDiamond(layer, position, radius)` &rarr; table\[piece, position\]

Returns a table of all pieces with an L1 distance to `position` less than or
equal to `radius`. In torus topology the positions returned are not normalised.

#### `grid:queryDisc(layer, position, radius)` &rarr; table\[piece, position\]

Returns a table of all pieces with an L2 distance to `position` less than or
equal to `radius`. In torus topology the positions returned are not normalised.

#### `grid:groupCount(group)` &rarr; Number

Returns the number of pieces belonging to a certain group.

#### `grid:groupRandom(random, group)` &rarr; piece

Returns a random piece belonging to a given group.

#### `grid:groupShuffled(random, group)` &rarr; array\[piece\]

Returns pieces belonging to a certain group in a random order.

#### `grid:groupShuffledWithCount(random, group, count)` &rarr; array\[piece\]

Returns `count` random pieces belonging to a certain group in a random order.

#### `grid:groupShuffledWithProbability(random, group, probability)` &rarr; array\[piece\]

Returns pieces belonging to a certain group in a random order, where each piece
has the given probability of being returned.

### Updating

All updates are queued when called, then processed during `grid:update(random,
flushCount = 128)`. Callbacks may introduce new updates on the queue. These will
be flushed up to `flushCount` (128) times.

#### `grid:setUpdater{update=update, group=group, probability=1.0, startFrame=0}`

Sets the group of pieces to be updated during `grid:update(random)`. The update
will trigger the callback `onUpdate.update(grid, piece)`

#### `grid:update(random, flushCount = 128)`

Updates the grid, processing all actions queued. If new actions are queued
during the update via callbacks they are flushed up to `flushCount` (128) times.

#### `grid:moveAbs(piece, orientation)`

Pushes a piece in the direction specified by `orientation`. World relative: the
piece's orientation is not taken into account. If there is a piece in the target
location at the time of the move then the piece stays where it is and
`onBlocked` is called.

Triggers `onContact.contactName.leave` callbacks for pieces at current location
and `onContact.contactName.enter` for pieces at the location moved to. Both
callbacks are triggered even if the move is not possible, in which case `piece`
effectively leaves and enters the same cell.

#### `grid:moveRel(piece, orientation)`

Pushes a piece in specified direction relative to the direction the piece is
facing. If there is a piece in the target location at the time of the move then
the piece stays where it is and `onBlocked` is called. Triggers
`onContact.contactName.leave` and `onContact.contactName.enter` callbacks in the
same way as `grid:moveAbs(piece, orientation)`

#### `grid:teleport(piece, position)`

Teleports the player to a given position. Triggers `onContact.contactName.leave`
and `onContact.contactName.enter` in the same way as `grid:moveAbs(piece,
orientation)`.

#### `grid:turn(piece, turn)`

Turns a piece by given value:

*   `0` -> No Op.
*   `1` -> 90 degrees clockwise.
*   `2` -> 180 degrees.
*   `3` -> 90 degrees counterclockwise.

#### `grid:setOrientation(piece, orientation)`

Sets the orientation of a piece (the direction it is facing in grid space):

*   `'N'` -> North (decreasing y).
*   `'E'` -> East (increasing x).
*   `'S'` -> South (increasing y.
*   `'W'` -> West (decreasing x).

#### `grid:hitBeam(piece, hitName, width, radius)`

Fires a beam with a given `hitName` with a given `width` and `radius`. Triggers
`onHit.hitName(pieceHit, hitter)` for all pieces hit by the beam.

#### `grid:setState(piece, state)`

Sets the state of a piece. The transition to the new state will happen during an
update as soon as a layer becomes available. Triggers callback `onRemove(piece)`
for the previous state and `onAdd(piece)` for the new state. Also triggers
`onContact.contactName.leave` and `onContact.contactName1.enter` for pieces at
that location.

#### `grid:teleportToGroup(piece, group, state[, orienationFlag])`

Sets the position of a piece to an unoccupied layer and position in group
`group`. Calls the same callbacks as `grid::setState()`,

`orienationFlag` can be one of the following:

*   `grid_world.TELEPORT_ORIENTATION.PICK_RANDOM`: **Default** - Picks a random
    orientation.
*   `grid_world.TELEPORT_ORIENTATION.KEEP_ORIGINAL`: Keeps piece's original
    orientation.
*   `grid_world.TELEPORT_ORIENTATION.MATCH_TARGET`: Matches target piece's
    orientation.

To pick own orientation:

```Lua
local KEEP_ORIENTATION = grid_world.TELEPORT_ORIENTATION.KEEP_ORIGINAL
grid:setOrientation(piece, orientation)
grid:teleportToGroup(piece, group, nil, KEEP_ORIENTATION)
```

### Connecting

#### `grid:connect(piece1, piece2)`

Connects two piece together in such a way that if one piece is pushed they are
all pushed.

#### `grid:disconnect(piece)`

Disconnect a previously connected piece.

#### `grid:disconnectAll(piece)`

Disconnects a piece and all transitively connected pieces.
