# DeepMind Lab2D: Lua Levels API

On construction of the environment, the `levelName` setting specifies a game
script file name. By default, game script files live in the level directory
`game_scripts/levels` and end in `.lua`.

There is a setting allowing the user to specify their own level directories.
called `levelDirectory`.

The level script file name is searched for in the following order.

*   `<levelName>` (If levelName has .lua suffix.)
*   `<levelDirectory>/<levelName>.lua` if `levelDirectory` is specified.
*   `<levelDirectory>/<levelName>/init.lua` if `levelDirectory` is specified.
*   `game_scripts/levels/<levelName>.lua`
*   `game_scripts/levels/<levelName>/init.lua`

The level script must consist of Lua code that either returns a table, or a
function which when called returns a table. Where the table optionally
implementing the API functions below. An extra argument may be supplied to the
function call by specifying a suffix `:<argument>` to the levelName.

Require statements in scripts are processed in the following order.

*   `dirname(<levelName>)`.
*   `<levelDirectory>` if specified.
*   `game_scripts/levels`.

An example is described here:
[game_scripts/levels/examples/level_api.lua](../dmlab2d/game_scripts/levels/examples/level_api.lua)

The calling code that calls into these API functions can be found in:

*   [env_lua_api/env_lua_api.cc](../dmlab2d/env_lua_api/env_lua_api.cc).
*   [env_lua_api/actions.cc](../dmlab2d/env_lua_api/actions.cc).
*   [env_lua_api/episodes.cc](../dmlab2d/env_lua_api/episode.cc).
*   [env_lua_api/observations.cc](../dmlab2d/env_lua_api/observations.cc).
*   [env_lua_api/properties.cc](../dmlab2d/env_lua_api/properties.cc).

## Initialisation

Called at startup once.

*   `init(settings)` - Constructs the environment readying the specs.
*   `observationSpec()` - Returns the list of observations available.
*   `discreteActionSpec()` - Optional, returns the list of discrete actions.
*   `continuousActionSpec()` - Optional, returns the list of continuous actions.
*   `textActionSpec()` - Optional, returns the list of text actions.

## Episode Start

Called at any time after initialisation.

*   `start(episode, seed)` - Starts an episode ready for stepping.

## Frame Step

Called at any time after the first call to `start`.

*   `observation(idx)` - Returns observation at index matching spec, may be
    called for each observation the agent requires.

Called only while an episode is active i.e. after `start` or after a call to
`advance` where the episode has not been marked as ended.

*   `discreteActions(actions)` - Optional, called with a list of discrete
    actions.
*   `continuousActions(actions)` - Optional, called with a list of continuous
    actions.
*   `textActions(actions)` - Optional, called with a list of text actions.
*   `advance(frame)` - Advance the frame of the environment returning whether
    the episode is still active and the reward for that frame.

## Property API

Called at any time after initialisation.

*   `writeProperty(key, value)` - Optional, sets a property.
*   `readProperty(key)` - Optional, reads a property.
*   `listProperty(key, callback)` - Optional, lists properties.

## `init(settings)`

Called at game startup with the table *settings* representing a key-value store
for all settings passed at the start of the environment.

Example:

```lua
function api:init(settings)
  print('Script Settings')
  for k, v in pairs(settings) do
    print(k .. ' = ' .. v)
  end
  io.flush()
end
```

### `observationSpec()` &rarr; array

### `observation(index)` &rarr; MatchSpec

`observationSpec` is called after `init`. Returns an array of observation types.

Each entry must contain a `name`, `type` and `shape`.

*   `name` Name of the observation reported by the environment.
*   `type` Type of tensor returned by the environment. Must be one of:
    `'tensor.DoubleTensor'`, `'tensor.ByteTensor'`, `'tensor.Int32Tensor'`,
    `'tensor.Int64Tensor'`, `'String'`.
*   `shape` An array of integers denoting the shape of the tensor. If the shape
    of any dimension can change between calls, it must be set to `-1` here.
    Use an empty list to denote a scalar. Shape does not need to be specified
    for string types.

`observation(index)` is called after `start`. Returns an observation matching
the spec of the same index. This will be either a tensor or a string. (Scalar
values are auto converted.)

```lua
function api:observationSpec()
  local count = 2
  return {
    {name = 'HITS', type = 'tensor.Int32Tensor', shape = {count, count}},
    {name = 'PICK_UP_VALUES', type = 'doubles', shape = {-1}},
    {name = '1.REWARD', type = 'tensor.DoubleTensor', shape = {}},
    {name = '2.REWARD', type = 'tensor.DoubleTensor', shape = {}},
    {name = '1.MESSAGE_FROM2', type = 'String'},
    {name = '2.MESSAGE_FROM1', type = 'String'},
  }
end

local order = 'Find Apples!'
local observationTable = {
    tensor.Int32Tensor{2, 2}, -- 'HITS'
    tensor.DoubleTensor(4) -- 'PICK_UP_VALUES'
    0, -- 1.REWARD
    0, -- 2.REWARD
    '', -- 1.MESSAGE_FROM2
    '', -- 2.MESSAGE_FROM1
}

function api:observation(idx)
  return observationTable[idx]
end
```

### `discreteActionSpec()` &rarr; array

### `discreteActions(actions)`

`discreteActionSpec` is called after `init`. Returns a list of integer action
specs.

Each action must contain a `name`, `min` and `max`.

*   `name` Name of the action reported by the environment.
*   `min` Min value of actions.
*   `max` Max value of actions.

When specified `discreteActions` is called with an array of current actions for
that frame.

```lua

function api:actionSpec()
  return {
      {name = 'move', min = 0, max = 4},
      {name = 'turn', min = -1, max = 1},
      {name = 'fire', min = 0, max = 1},
  }
end

local discreteActions = {
    move = 0,
    turn = 0,
    fire = 0,
}

function api:discreteActions(actions)
  discreteActions.move = actions[1]
  discreteActions.turn = actions[2]
  discreteActions.fire = actions[3]
end
```

### `continuousActionSpec()` &rarr; array

### `continuousActions(actions)`

`continuousActionSpec` is called after `init`. Returns a list of floating point
action specs.

Each action must contain a `name`, `min` and `max`.

*   `name` Name of the action reported by the environment.
*   `min` Min value of actions.
*   `max` Max value of actions.

When specified `continuousActions` is called with an array of current actions
for that frame.

```lua
function api:actionSpec()
  -- Choose between ship speed and shields.
  return {{name = 'shieldSpeed', min = -1.0, max = 1.0}}
end

shieldSpeed = -0.5,

function api:continuousActions(actions)
  shieldSpeed = actions[1]
end
```

### `textActionSpec()` &rarr; array\[string\]

`textActionSpec` is called after `init`. Returns a list of names of text
actions.

When specified `textActions` is called with an array of current actions for that
frame.

```lua
function api:actionSpec()
  -- Choose between ship speed and shields.
  return {"talkToPlayer1", "talkToPlayer2"}
end

talkToPlayer1 = '',
talkToPlayer2 = '',

function api:textActions(actions)
  talkToPlayer1 = actions[1]
  talkToPlayer2 = actions[2]
end
```

### `start(episode, seed)`

The environment calls this function at the start of each episode, with:

*   A number *episode*, starting from 0.
*   A number *seed*, the random seed of the game engine. Supplying the same seed
    is intended to result in reproducible behaviour.

Example:

```lua
local random = require 'system.random'

function api:start(episode, seed)
  random:seed(seed)
  print('Entering episode no. ' .. episode .. ' with seed ' .. seed)
  io.flush()
end
```

### `advance(frameIdx)` &rarr; boolean, reward

Advance the simulation by one frame. Returns whether the episode should continue
and the total reward for this frame.

## Properties

`PROPERTY_RESULT` is one of:

```lua
local properties = require 'system.properties'
assert(properties.SUCCESS == 0)
assert(properties.NOT_FOUND == 1)
assert(properties.PERMISSION_DENIED == 2)
assert(properties.INVALID_ARGUMENT == 3)
```

### `writeProperty(key, value)` &rarr; `PROPERTY_RESULT`

Callback to support `RlCApi`'s `write_property`. Return `true` if
`property[key]` is assigned `value`. Return `false` if `property[key]` cannot
hold `value` and return `nil` if `property[key]` does not exist.

### `readProperty(key)` &rarr; string or `PROPERTY_RESULT`

Callback to support `RlCApi`'s `read_property`. Return value of `property[key]`
if it exists otherwise return `nil`.

### `listProperty(key, callback)` &rarr; `PROPERTY_RESULT`

Callback to support `RlCApi`'s `list_property`. Return true if `property[key]`
is a list and call `callback(key, type)` for each sub-property of
`property[key]` where key is the full length key to the sub-property and type a
string containing any combination of 'r', 'w' and 'l'.

If the type contains:

*   'l' - Listable and `listProperty(key)` maybe called.
*   'r' - Readable and `readProperty(key)` maybe called.
*   'w' - Writable and `writeProperty(key, value)` maybe called.

## Event API

Events are cleared at the start of each call to advance and can be created using
the `system.events` module, which can be loaded using `local events = require
'system.events'`. The module provides the following function.

### `add(name[, observation1[, observation2 ... ]])`

Adds events to be read by the user. Each event has a list of observations. Each
observation may be one of String, ByteTensor, DoubleTensor, Int32Tensor or
Int64Tensor.

Example script:

```lua
local events = require 'system.events'
local tensor = require 'system.tensor'

local api = {}
function api:start(episode, seed)
  events:add('Event Name', 'Text', tensor.ByteTensor{3}, tensor.DoubleTensor{7})
end
```
