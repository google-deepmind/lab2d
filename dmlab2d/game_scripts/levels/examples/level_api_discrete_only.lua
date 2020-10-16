--[[ Copyright (C) 2019 The DMLab2D Authors.

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

local events = require 'system.events'
local tensor = require 'system.tensor'
local sys_random = require 'system.random'
local properties = require 'system.properties'

local api = {
    _steps = 10,
    _reward = 0,
    _observations = {
        tensor.ByteTensor{range = {1}},
        tensor.DoubleTensor{range = {2}},
        tensor.Int32Tensor{range = {3}},
    },
    _observation_seed = 0,
    _observation_step = 0,
    _observation_episode = 0,
}

--[[ Called once by framework before all other calls.

Arguments:

*   `kwargs` All settings forwarded from the EnvCApi apart from:
    -   'levelDirectory': Used to select the directory of the levels.
    -   'levelName': Used to select the level.
    -   'mixerSeed': Used to salt the seeds of random number generators.

In this example the only setting valid other than those consumed internally is
'steps = <number>'.
]]
function api:init(kwargs)
  local errors = ''
  for k, v in pairs(kwargs) do
    if k == 'steps' and tonumber(v) then
      self._steps = tonumber(v)
    else
      errors = errors .. 'Invalid setting ' .. k .. '=' .. v .. '\n'
    end
  end
  if errors ~= '' then
    return 2, errors
  end
end

--[[ Called by framework after once after init.

Returns:

*   Array observation spec.

Each observation spec must contain:

*   `name` - The name of the observation.
*   `type` - The type of the observation, Allowed values:
    -   'String'
    -   'tensor.DoubleTensor'
    -   'tensor.ByteTensor'
    -   'tensor.Int32Tensor'
    -   'tensor.Int64Tensor'
*   `shape` - Must be {0} if `type` is 'String' otherwise the shape of the
    tensor with 0 in place of dynamic dimensions.
]]
function api:observationSpec()
  return {
      {
          name = 'VIEW1',
          type = self._observations[1]:type(),
          shape = self._observations[1]:shape(),
      },
      {
          name = 'VIEW2',
          type = self._observations[2]:type(),
          shape = self._observations[2]:shape(),
      },
      {
          name = 'VIEW3',
          type = self._observations[3]:type(),
          shape = self._observations[3]:shape(),
      },
      {
          name = 'SEED',
          type = 'tensor.Int64Tensor',
          shape = {},
      },
      {
          name = 'EPISODE',
          type = 'tensor.Int32Tensor',
          shape = {},
      },
      {
          name = 'STEP',
          type = 'tensor.Int32Tensor',
          shape = {},
      }
  }
end

--[[ Called by framework and returns an observation.

Arguments:

*   `index` observation index corresponding the index of the spec returned in
    observationSpec.

Returns:
]]
function api:observation(index)
  if index < 4 then
    return self._observations[index]
  elseif index == 4 then
    return self._observation_seed
  elseif index == 5 then
    return self._observation_episode
  else
    return self._observation_step
  end
end

--[[ Called by framework after once after init and returns discreteActionSpec.

Returns:

*   Array of discrete action spec.
    Each action spec must contain:
    -   `name`: The name of the action.
    -   `min`: Inclusive minimum value of the action.
    -   `max`: Inclusive maximum value of the action.
]]
function api:discreteActionSpec()
  return {{name = 'REWARD_ACT', min = 0, max = 4}}
end

--[[ Called by framework and sets discrete actions before call to advance.

Arguments:

*   `actions` - An array of doubles matching continous action spec.
]]
function api:discreteActions(actions)
  self._reward = actions[1]
end

--[[ Called at the beginning of each episode.

The first observation may be observed after this call. The `episode` is
forwarded from the EnvCApi, and the `seed` is calculated from an internal random
number generator initialised with the seed from the EnvCApi and `mixerSeed`.
]]
function api:start(episode, seed)
  sys_random:seed(seed)
  self._observation_seed = seed
  self._observation_step = 0
  self._observation_episode = episode
  events:add('start', tensor.Int64Tensor{range = {3}})
end

--[[ Called at each advance of the environment.

Arguments:

*   `steps` - Total steps taken this episode including this one.

Returns:

*   bool - Whether the environment should continue.
*   number - Total reward gained this frame.
]]
function api:advance(steps)
  self._observation_step = steps
  return steps < self._steps, self._reward
end

return api
