# Generators Pushbox

```lua
local pushbox = require 'system.generators.pushbox'
```

Returns a level generator for pushbox levels.

Underlying C++ code is in `dmlab2d/system/generators/pushbox/pushbox.cc`

## `generate(kwargs)`

The `kwargs` will have the following values.

*   `seed = <number>` Seeds `roomSeed` `targetsSeed` and
    `actionSeed` if not provided. (Set to 0 if supplying other seeds.)
*   `width = <number>` The max-line-width in characters of the returned string.
*   `height = <number>` The number of lines in the returned string.
*   `numBoxes = <number>` The number of boxes.
*   `roomSteps = <number|nil>` The number of attempts to place rooms:
    higher will produce fewer wall characters.
*   `roomSeed = <number|nil> Seed used for room placement. If left unset one
    from `seed` is generated in its place.
*   `targetsSeed = <number|nil>` Seed used for goal placement. If left unset one
    from `seed` is generated in its place.
*   `actionsSeed = <number|nil>` Seed used for reverse move generation. The
     level is created in solution position and the actions are applied in
     reverse to generate the problem. (Steps are taken to ensure the level is of
     sufficient difficulty.)

```lua
> pushbox = require 'system.generators.pushbox'
> layout = pushbox.generate{
      seed = 10,
      width = 14,
      height = 11,
      numBoxes = 5,
  }
> print(layout)
**************
****** *******
**  X      B *
** XXBX * BPX*
** B**********
* *  *********
*    *********
* B  *********
*  ***********
**************
**************.
```
