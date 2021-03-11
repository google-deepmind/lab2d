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
local tables = require 'common.tables'
local read_settings = require 'common.read_settings'
local properties = require 'common.properties'
local tile_set = require 'common.tile_set'
local set = require 'common.set'
local images = require 'images'
local maps = require 'maps'
local unpack = _G.unpack or table.unpack  -- Handle different versions of Lua.

local _COMPASS = {'N', 'E', 'S', 'W'}
local _DIRECTION = {
    N = {0, -1},
    E = {1, 0},
    S = {0, 1},
    W = {-1, 0},
}


local function _calculateQuad(dx, dy)
  local absx = math.abs(dx)
  local absy = math.abs(dy)
  local quad
  if absy >= absx then
    if dy <= 0 then
      quad = 'NN'
    else
      quad = 'SS'
    end
  else
    if dx <= 0 then
      quad = 'WW'
    else
      quad = 'EE'
    end
  end
  if absy < absx then
    if dy <= 0 then
      quad = quad .. 'N'
    else
      quad = quad .. 'S'
    end
  else
    if dx <= 0 then
      quad = quad .. 'W'
    else
      quad = quad .. 'E'
    end
  end
  return quad
end

local _TOWARDS_DIRECTION = {
    NNE = {'N', 'E', 'W', 'S'},
    NNW = {'N', 'W', 'E', 'S'},
    SSE = {'S', 'E', 'W', 'N'},
    SSW = {'S', 'W', 'E', 'N'},
    EEN = {'E', 'N', 'S', 'W'},
    EES = {'E', 'S', 'N', 'W'},
    WWN = {'W', 'N', 'S', 'E'},
    WWS = {'W', 'S', 'N', 'E'},
}

local _AWAY_DIRECTION = {
    NNE = {'S', 'W', 'E', 'N'},
    NNW = {'S', 'E', 'W', 'N'},
    SSE = {'N', 'W', 'E', 'S'},
    SSW = {'N', 'E', 'W', 'S'},
    EEN = {'W', 'S', 'N', 'E'},
    EES = {'W', 'N', 'S', 'E'},
    WWN = {'E', 'S', 'N', 'W'},
    WWS = {'E', 'N', 'S', 'W'},
}

local _180 = {
    N = 'S',
    E = 'W',
    S = 'N',
    W = 'E',
}

local function _isFoeAllowedToMove(grid, pos, compass)
  local hit, block = grid:rayCastDirection('pieces', pos, _DIRECTION[compass])
  if hit and block and grid:state(block) == 'wall' then
    return false
  end
  local hit = grid:rayCastDirection('foe', pos, _DIRECTION[compass])
  return not hit
end

local function _chaseRandom(grid, piece)
  local foe = grid:transform(piece)
  local banned = _180[foe.orientation]
  local picked = banned  -- Can get out of dead end.
  for _, compass in ipairs(random:shuffle(_COMPASS)) do
    if compass ~= banned and _isFoeAllowedToMove(grid, foe.pos, compass) then
      picked = compass
      break
    end
  end
  grid:setOrientation(piece, picked)
  grid:moveRel(piece, 'N')
end

local function _chase(grid, piece, position, direction, stochasticity)
  if random:uniformReal(0, 1) < stochasticity then
    _chaseRandom(grid, piece)
    return
  end

  local foe = grid:transform(piece)
  local tX, tY = unpack(position)
  local gX, gY = unpack(foe.pos)
  local dX = tX - gX
  local dY = tY - gY
  local banned = _180[foe.orientation]
  local picked = banned  -- Can get out of dead end.
  for _, compass in ipairs(direction[_calculateQuad(dX, dY)]) do
    if compass ~= banned and _isFoeAllowedToMove(grid, foe.pos, compass) then
      picked = compass
      break
    end
  end
  grid:setOrientation(piece, picked)
  grid:moveRel(piece, 'N')
end

local function apiFactory(env)
  local api = {
      _world = {},
      _observations = {},
      _settings = {
          spriteSize = 16,
          episodeLengthFrames = 3000,
          mapName = 'DEFAULT',
          topology = 'BOUNDED',
          food = {reward = 1},
          pill = {
              num = 2,
              reward = 2,
              duration = 20,
          },
          foe = {
              -- Starting foe count.
              numInit = 1,
              -- Number of foes math.floor((level-1) * numIncrease + numInit.
              numIncrease = 0.5,

              -- Starting foe speed (probability it will move.)
              -- Speed (level-1) * speedIncrease + speedInit.
              speedInit = 0.5,
              speedIncrease = 0.1,

              -- When food slow down by half.
              foodSlowFactor = 0.5,

              -- Probability to choose a random direction.
              stochasticity = 0.05,

              -- Minimum L1 distance to spawn foe.
              safeDistance = 5,

              -- Reward for collecting foe.
              reward = 5,
          },
      },
      _move = 0,
      _steps = 0,
  }

  function api:textMap()
    return maps[self._settings.mapName]
  end

  function api:_createSprites(size)
    local tileSet = tile_set.TileSet(self._world, {width = size, height = size})
    tileSet:addShape('#Wall', images.wall())
    tileSet:addShape('Player', images.player(1))
    tileSet:addShape('+Pill', images.pill())
    tileSet:addShape('.Food', images.food())
    tileSet:addShape('FoeFood', images.foeFood())
    tileSet:addShape('Foe', images.foe())
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
        layout = self:textMap()
    }
    local worldView = tile.Scene{
        shape = worldLayerView:gridSize(),
        set = set,
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
    local config = {
        renderOrder = {'pills', 'pieces', 'foe'},
        updateOrder = {'move', 'chase'},
        states = {
            wall = {
                layer = 'pieces',
                sprite = '#Wall',
            },
            spawn = {
                groups = {'spawn'},
                layer = 'spawn',
            },
            player = {
                layer = 'pieces',
                groups = {'movers'},
                sprite = 'Player',
                contact = 'avatar',
            },
            ['player.wait'] = {
                layer = 'pieces',
                sprite = 'Player',
            },
            food = {
                layer = 'pills',
                sprite = '.Food',
                groups = {'food'},
            },
            pill = {
                layer = 'pills',
                sprite = '+Pill',
                groups = {'food'},
            },
            foe = {
                layer = 'foe',
                sprite = 'Foe',
                groups = {'foes', 'allFoes'},
            },
            ['foe.food'] = {
                layer = 'foe',
                sprite = 'FoeFood',
                groups = {'foes.food', 'allFoes'},
            },
        }
    }
    return config
  end

  function api:init(kwargs)
    read_settings.apply(tables.flatten(env.settings), self._settings)
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
    if actions[1] ~= 0 then
      self._move = actions[1]
    end
  end


  function api:stateCallbacks()
    local states = {wall = {onHit = true}}
    states.player = {
        onUpdate = {
            move = function(grid, piece)
              if self._move ~= 0 then
                local compass = _COMPASS[self._move]
                grid:setOrientation(piece, compass)
                grid:moveRel(piece, 'N')
              end
            end
        }
    }
    states.food = {
        onContact = {
            avatar = {
                enter = function(grid, food, avatar)
                  self._reward = self._reward + self._settings.food.reward
                  grid:removePiece(food)
                end
            }
        }
    }
    states.pill = {
        onContact = {
            avatar = {
                enter = function(grid, piece, avatar)
                  local pill = self._settings.pill
                  self._reward = self._reward + pill.reward
                  grid:removePiece(piece)
                  self:_enablePowerUp()
                  self._endPill = self._steps + pill.duration
                end
            }
        }
    }
    states.foe = {
        onContact = {
            avatar = {
                enter = function(grid, foe, avatar)
                  grid:setState(avatar, 'player.wait')
                end
            }
        },
        onUpdate = {
            chase = function(grid, piece)
              _chase(grid, piece, grid:position(self._avatar),
                     _TOWARDS_DIRECTION, self._settings.foe.stochasticity)
            end
        }
    }
    states['foe.food'] = {
        onContact = {
            avatar = {
                enter = function(grid, piece, avatar)
                  self._reward = self._reward + self._settings.foe.reward
                  grid:removePiece(piece)
                end
            }
        },
        onUpdate = {
            chase = function(grid, piece)
              _chase(grid, piece, grid:position(self._avatar),
                  _AWAY_DIRECTION, self._settings.foe.stochasticity)
            end
        },
    }
    return states
  end

  function api:_foeSpeed(hasPower)
    local foe = self._settings.foe
    local result = foe.speedInit + (self._level - 1) * foe.speedIncrease
    return hasPower and foe.foodSlowFactor * result or result
  end

  function api:startLevel(level)
    if self._grid then
      self._grid:destroy()
    end
    self._level = level
    local settings = self._settings
    self._move = 0
    local grid = self._world:createGrid{
        stateCallbacks = self:stateCallbacks(),
        topology = grid_world.TOPOLOGY[settings.topology],
        layout = self:textMap(),
        stateMap = {
            ['*'] = 'wall',
            ['#'] = 'wall',
            ['.'] = 'spawn',
        },
    }
    self._grid = grid
    local spawnPoints = grid:groupShuffled(random, 'spawn')
    local playerSpawn = spawnPoints[1]
    local playerTransform = grid:transform(playerSpawn)
    self._avatar = grid:createPiece('player', grid:transform(playerSpawn))
    grid:setUpdater{update = 'move', group = 'movers'}

    local foe = self._settings.foe
    grid:setUpdater{
        update = 'chase',
        group = 'allFoes',
        self:_foeSpeed(false)
    }
    grid:update(random)
    -- skip first spawn point.
    local disallowedSpawnPoints =
        grid:queryDiamond(
            'spawn',
            playerTransform.pos,
            self._settings.foe.safeDistance)
    local numFoes = math.floor(
      foe.numInit + (level - 1) * foe.numIncrease)
    local pill = self._settings.pill
    local numPills = pill.num
    for i, spawnPoint in next, spawnPoints, 1 do
      if numFoes > 0 and not disallowedSpawnPoints[spawnPoint] then
        local piece = grid:createPiece('foe', grid:transform(spawnPoint))
        numFoes = numFoes - 1
        grid:createPiece('food', grid:transform(spawnPoint))
      elseif numPills > 0 then
        grid:createPiece('pill', grid:transform(spawnPoint))
        numPills = numPills - 1
      else
        grid:createPiece('food', grid:transform(spawnPoint))
      end
    end
    self._reward = 0
    self._steps = 0
    self._endPill = 0
  end

  function api:start(episode, seed)
    random:seed(seed)
    self:startLevel(1)
  end

  function api:_enablePowerUp()
    local grid = self._grid
    grid:setUpdater{
        update = 'chase',
        group = 'allFoes',
        self:_foeSpeed(true),
        probability = self:_foeSpeed(true),
    }
    for _, piece in ipairs(grid:groupShuffled(random, 'foes')) do
      grid:setState(piece, "foe.food")
      local t = grid:transform(piece)
      grid:setOrientation(piece, _180[t.orientation])
    end
  end

  function api:_disablePowerUp()
    local grid = self._grid
    grid:setUpdater{
        update = 'chase',
        group = 'allFoes',
        probability = self:_foeSpeed(false),
    }
    for _, piece in ipairs(grid:groupShuffled(random, 'foes.food')) do
      grid:setState(piece, "foe")
      local t = grid:transform(piece)
      grid:setOrientation(piece, _180[t.orientation])
    end
  end

  function api:advance(steps)
    local grid = self._grid
    local settings = self._settings
    self._steps = steps
    local countBefore = grid:groupCount('food')
    if self._endPill == steps then
      self:_disablePowerUp()
    end
    grid:update(random)
    local reward = self._reward
    local countAfter = grid:groupCount('food')
    if countAfter == 0 then
      self:startLevel(self._level + 1)
      grid = self._grid
    end
    local continue = (grid:state(self._avatar) == 'player'
                      and steps < settings.episodeLengthFrames)
    self._reward = 0
    return continue, reward
  end

  properties.decorate(api)
  return api
end

return {apiFactory = apiFactory}
