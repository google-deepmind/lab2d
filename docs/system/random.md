# Random

Class holding a random number generator. It is the user's responsibility to seed
the random number generator. This should be done at the beginning of the `start`
function. I.e.:

```lua
local random = require 'system.random'
api = {}
function api:start(episode, seed)
  random:seed(seed)
  ...
end
return api
```

## `seed(seed)`

Seeds the random number generator. The seed may be a number or string.

*   `seed` Number or string.

Warning: The sequences returned may be different depending on which version of
the standard library the code was compiled with.

```lua
> random = require 'system.random'
> random:seed(4)  -- chosen by fair dice roll.
> =random:uniformInt(1, 100)  -- Example output.
72
> =random:uniformInt(1, 100)
77
> =random:uniformInt(1, 100)
27
> random:seed(4)
> =random:uniformInt(1, 100)
72
> =random:uniformInt(1, 100)
77
> =random:uniformInt(1, 100)
27
```

## `uniformInt(min, max)`

Returns random integer values, uniformly distributed on the closed interval
`[min, max]`.

```lua
> random = require 'system.random'
> random:seed(4)  -- chosen by fair dice roll.
> =random:uniformInt(1, 4)  -- Example output.
4
> =random:uniformInt(1, 4)
1
> =random:uniformInt(1, 4)
3
> =random:uniformInt(1, 4)
3
```

## `uniformReal(min, max)`

Returns random floating point values, uniformly distributed on the interval
`[min, max)`.

```lua
> random = require 'system.random'
> random:seed(4)  -- chosen by fair dice roll.
> =random:uniformReal(0, 1)  -- Example output.
0.78554828992887
> =random:uniformReal(0, 1)
0.45382974451972
> =random:uniformReal(0, 1)
0.59425062967119
> =random:uniformReal(0, 1)
0.062309866433035
```

## `discreteDistribution(weights)`

Returns random integer values on the interval `[1, #weights]`, where the
probability of each individual `i` is given by `weights[i]/sum(weights)`.

*   `weights` - Non empty list of positive numbers.

```lua
> random = require 'system.random'
> fruit = {'Apple', 'Pear', 'Lemon'}
> random:seed(4)  -- chosen by fair dice roll.
> -- 'Apple' will, on average, be chosen twice as often.
> =fruit[random:discreteDistribution{2, 1, 1}]
Lemon
> =random:discreteDistribution{2, 1, 1}
Apple
> =random:discreteDistribution{2, 1, 1}
Pear
> =random:discreteDistribution{2, 1, 1}
Apple
```

## `normalDistribution(mean, stddev)`

Returns a real number sampled from a normal distribution centered around `mean`
with standard deviation `stddev`.

```lua
> random:seed(
> -- Return values around 10.
> =random:normalDistribution(10, 1)
11.46061550983
> =random:normalDistribution(10, 1)
10.139912107345
> =random:normalDistribution(10, 1)
10.072564595487
> =random:normalDistribution(10, 1)
10.727950782062
> =random:normalDistribution(10, 1)
11.923987687169
> =random:normalDistribution(10, 1)
9.4964397131262
```

## `poissonDistribution(mean)`

Returns the discrete probability according to the Poisson Distribution. See also
https://en.cppreference.com/w/cpp/numeric/random/poisson_distribution.

```lua
> random:seed(4)
> -- Return values around 4.
> =random:poissonDistribution(4)
3
> =random:poissonDistribution(4)
3
> =random:poissonDistribution(4)
8
> =random:poissonDistribution(4)
6
> =random:poissonDistribution(4)
4
> =random:poissonDistribution(4)
2
```

## `choice(list)`

Returns a random element from a list, if the list is not empty. Otherwise
returns `nil`.

```lua
> random = require 'system.random'
> random:seed(4)
> =random:choice{1, 2, 3, 4}
4
> =random:choice{1, 2, 3, 4}
1
> =random:choice{1, 2, 3, 4}
3
> =random:choice{}
nil
```

## `shuffle(list)`

Returns a copy of the list with the elements shuffled.

```lua
> tables = require 'common.tables'
> random = require 'system.random'
> random:seed(4)
> =tables.tostringOneLine(random:shuffle{1, 2, 3, 4})
{4, 2, 3, 1}
```

## `shuffleInPlace(list)`

Shuffles the elements of `list` in-place.

```lua
> tables = require 'common.tables'
> random = require 'system.random'
> random:seed(4)
> list = {1, 2, 3, 4}
> random:shuffleInPlace(list)
> return tables.tostringOneLine(list)
{4, 2, 3, 1}
```
