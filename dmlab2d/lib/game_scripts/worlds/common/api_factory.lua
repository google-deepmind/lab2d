--[[ Copyright (C) 2019 The DMLab2D Authors.

Licensed under the Apache License, Version 2.0 (the 'License');
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an 'AS IS' BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
]]

local grid_world = require 'system.grid_world'
local random = require 'system.random'
local tables = require 'common.tables'
local read_settings = require 'common.read_settings'
local properties = require 'common.properties'
local tile_set = require 'common.tile_set'


local function apiFactory(env)
  local api = {
      _world = {},
      _observations = {
          {
              name = 'GLOBAL.TEXT',
              type = 'String',
              shape = {0},
              func = function(grid) return tostring(grid) end
          },
      },
      _settings = {
          numPlayers = 1,
          spriteSize = 16,
          simulation = env.Simulation.defaultSettings(),
          avatars = env.AvatarList.defaultSettings(),
          episodeLengthFrames = 3600,
          topology = 'BOUNDED'
      }
  }

  function api:_createSprites(size)
    local tileSet = tile_set.TileSet(self._world, {width = size, height = size})
    self.simulation:addSprites(tileSet)
    self.avatars:addSprites(tileSet)
    return tileSet:set()
  end

  function api:init(kwargs)
    read_settings.apply(tables.flatten(env.settings), self._settings)
    read_settings.apply(kwargs, self._settings)
    self.simulation = env.Simulation{
        numPlayers = self._settings.numPlayers,
        settings = self._settings.simulation,
        avatarSettings = self._settings.avatars,
    }

    self.avatars = env.AvatarList{
        settings = self._settings.avatars,
        numPlayers = self._settings.numPlayers,
        simSettings = self._settings.simulation,
        simulation = self.simulation
    }
    local worldConfig = self.simulation:worldConfig()
    self.avatars:addConfigs(worldConfig)
    self._world = grid_world.World(worldConfig)
    local tileSet = self:_createSprites(self._settings.spriteSize)
    self.simulation:addObservations(tileSet, self._world, self._observations)
    self.avatars:addObservations(tileSet, self._world, self._observations)
  end

  function api:observationSpec()
    return self._observations
  end

  function api:observation(idx)
    return self._observations[idx].func(self._grid)
  end

  function api:discreteActionSpec()
    return self.avatars:discreteActionSpec()
  end

  function api:discreteActions(actions)
    self.avatars:discreteActions(self._grid, actions)
  end

  function api:start(episode, seed)
    if self._grid then
      self._grid:destroy()
    end
    random:seed(seed)
    local stateCallbacks = self.simulation:stateCallbacks(self.avatars)
    self.avatars:addPlayerCallbacks(stateCallbacks)
    local textMap = self.simulation:textMap()
    self._grid = self._world:createGrid{
        layout = textMap.layout,
        stateMap = textMap.stateMap,
        stateCallbacks = stateCallbacks,
        topology = grid_world.TOPOLOGY[self._settings.topology],
    }

    self.simulation:start(self._grid)
    self.avatars:start(self._grid)
    self._grid:update(random)
  end

  function api:advance(steps)
    self.avatars:update(self._grid)
    local simulationContinue = self.simulation:update(self._grid) ~= false
    -- Ensure all type changes are flushed for rendering this frame.
    self._grid:update(random)
    local withinFrameLimit = steps < self._settings.episodeLengthFrames
    local continue = simulationContinue and withinFrameLimit
    return continue, self.avatars:getReward(self._grid)
  end

  properties.decorate(api)
  return api
end

return {apiFactory = apiFactory}
