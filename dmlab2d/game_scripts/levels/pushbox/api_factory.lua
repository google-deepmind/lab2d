--[[ Copyright (C) 2020 The DMLab2D Authors.

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
local tile = require 'system.tile'
local pushbox = require 'system.generators.pushbox'
local tables = require 'common.tables'
local read_settings = require 'common.read_settings'
local properties = require 'common.properties'
local tile_set = require 'common.tile_set'
local images = require 'images'

local _COMPASS = {'N', 'E', 'S', 'W'}
local _DIRECTION = {
    N = {0, -1},
    E = {1, 0},
    S = {0, 1},
    W = {-1, 0},
}

local api_factory = {}

function api_factory.apiFactory(env)
  local api = {
      _world = {},
      _observations = {},
      _settings = {
          spriteSize = 8,
          episodeLengthFrames = 120,
          gridShape = {width = 10, height = 10},
          topology = 'BOUNDED',
          stepCost = 0.1,
          numBoxes = 4,
          roomSteps = 30,
      },
      _move = 0,
      _steps = 0,
  }

  function api:_createSprites(size)
    local tileSet = tile_set.TileSet(self._world, {width = size, height = size})
    tileSet:addShape('*Wall', images.wall())
    tileSet:addShape('Box', images.box())
    tileSet:addShape('XGoal', images.goal())
    tileSet:addShape('&BoxGoal', images.goal())
    tileSet:addShape('Player', images.player('green'))
    return tileSet:set()
  end

  function api:_addObservations(specs, world, set)
    local stepSpec = {
        name = 'WORLD.STEPS',
        type = 'Int32s',
        shape = {},
        func = function() return self._steps end
    }
    table.insert(specs, stepSpec)

    local textSpec = {
        name = 'WORLD.TEXT',
        type = 'String',
        shape = {0},
        func = function(grid) return tostring(grid) end
    }
    table.insert(specs, textSpec)

    local worldLayerView = world:createView{
        left = 0,
        forward = 0,
        right = self._settings.gridShape.width - 1,
        backward = self._settings.gridShape.height - 1,
    }

    local worldLayerSpec = worldLayerView:observationSpec('WORLD.LAYER')
    worldLayerSpec.func = function(grid)
      return worldLayerView:observation{grid = grid}
    end

    table.insert(specs, worldLayerSpec)
    local tileScene = tile.Scene{shape = worldLayerView:gridSize(), set = set}
    local worldRgbSpec = {
        name = 'WORLD.RGB',
        type = 'tensor.ByteTensor',
        shape = tileScene:shape(),
        func = function(grid)
          return tileScene:render(worldLayerView:observation{grid = grid})
        end
    }
    table.insert(specs, worldRgbSpec)
    return specs
  end

  function api:_createWorldConfig()
    return {
        renderOrder = {'pieces', 'logic'},
        updateOrder = {'move'},
        states = {
            wall = {
                layer = 'pieces',
                sprite = '*Wall',
            },
            spawnPoint = {
                groups = {'spawnPoints'},
                layer = 'invisible',
            },
            player = {
                groups = {'movers'},
                layer = 'pieces',
                sprite = 'Player',
            },
            box = {
                layer = 'pieces',
                sprite = 'Box',
            },
            boxGoal = {
                layer = 'logic',
                sprite = '&BoxGoal',
                contact = 'goal',
            },
            goal = {
                groups = {'empty_goal'},
                layer = 'logic',
                sprite = 'XGoal',
                contact = 'goal',
            },
        }
    }
  end

  function api:init(kwargs)
    read_settings.apply(tables.flatten(env.settings or {}), self._settings)
    read_settings.apply(kwargs, self._settings)
    local worldConfig = self:_createWorldConfig()
    self._world = grid_world.World(worldConfig)
    local tileSet = self:_createSprites(self._settings.spriteSize)
    self:_addObservations(self._observations, self._world, tileSet)
  end

  function api:observationSpec()
    return self._observations
  end

  function api:observation(idx)
    return self._observations[idx].func(self._grid)
  end

  function api:discreteActionSpec()
    return {{name = 'MOVE', min = 0, max = #_COMPASS}}
  end

  function api:discreteActions(actions)
    self._move = actions[1]
  end

  function api:stateCallbacks()
    local states = {wall = {onHit = true}}
    states.box = {
        onContact = {
            goal = {
                enter = function(grid, boxPiece, goalPiece)
                  grid:setState(goalPiece, 'boxGoal')
                end,
                leave = function(grid, boxPiece, goalPiece)
                  grid:setState(goalPiece, 'goal')
                end,
            }
        }
    }

    states.player = {
        onUpdate = {
            move = function(grid, piece)
              if self._move ~= 0 then
                local compass = _COMPASS[self._move]
                local hit, block = grid:rayCastDirection(
                    'pieces', grid:position(piece), _DIRECTION[compass])
                local connect = hit and grid:state(block) ~= 'wall'
                if connect then
                  grid:connect(piece, block)
                end
                grid:moveAbs(piece, compass)
                if connect then
                  grid:disconnect(piece)
                end
              end
            end
        }
    }
    return states
  end

  function api:start(episode, seed)
    if self._grid then
      self._grid:destroy()
    end
    random:seed(seed)
    local settings = self._settings
    local layout = pushbox.generate{
        seed = random:uniformInt(1, 2 ^ 32 - 1),
        width = settings.gridShape.width,
        height = settings.gridShape.height,
        numBoxes = settings.numBoxes,
        roomSteps = settings.roomSteps,
    }

    self._move = 0
    local grid = self._world:createGrid{
        stateCallbacks = self:stateCallbacks(),
        topology = grid_world.TOPOLOGY[settings.topology],
        size = settings.gridShape,
    }
    self._grid = grid
    local piecesCreated = grid:createLayout{
        stateMap = {
            P = 'spawnPoint',
            B = 'box',
            X = 'goal',
            ['&'] = 'boxGoal',
            ['*'] = 'wall',
        },
        layout = layout,
    }
    local piecesCreated = grid:createLayout{
        stateMap = {['&'] = 'box'},
        layout = layout,
    }
    local spawnHere = grid:groupRandom(random, 'spawnPoints')
    self._avatar = grid:createPiece('player', grid:transform(spawnHere))
    grid:setUpdater{update = 'move', group = 'movers'}
    grid:update(random)
  end

  function api:advance(steps)
    local countBefore = self._grid:groupCount('empty_goal')
    self._grid:update(random)
    --
    local countAfter = self._grid:groupCount('empty_goal')
    local continue = (steps < self._settings.episodeLengthFrames and
                      countAfter ~= 0)
    local reward = countBefore - countAfter - self._settings.stepCost
    self._steps = steps
    if countAfter == 0 then
      reward = reward + 10
    end
    return continue, reward
  end

  properties.decorate(api)
  return api
end

return api_factory
