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

local class = require 'common.class'
local tile = require 'system.tile'
local random = require 'system.random'
local images = require 'images'
local maps = require 'maps'

local Simulation = class.Class()

function Simulation.defaultSettings()
  return {
      mapName = 'default',
      -- Probability apple.possible becomes apple.
      initialAppleProbability = 0.75,
      -- Displays a color sprite associated with the probability of respawning.
      -- Green is highest probability red it zero probability.
      showRespawnProbability = false,
      -- Base probability of an apple respawning each step.
      appleRespawnProbabilities = setmetatable({0, 0.001, 0.005, 0.025}, {
        __index = function(table, key)
          local key = tonumber(key)
          if key == nil then
            return nil
          end
          table[key] = 0
          return 0
        end}),
      -- Seeding radius.
      seedRadius = 3
  }
end

function Simulation:__init__(kwargs)
  self._settings = kwargs.settings
end

local function interpColor(colors, fraction)
  local place = fraction * (#colors - 1) + 1
  local from = math.floor(place)
  local frac = place - from
  local to = math.min(#colors, from + 1)
  local result = {}
  for i = 1, # colors do
    result[i] = math.floor(colors[from][i] * (1 - frac) + colors[to][i] * frac)
  end
  return result
end

function Simulation:addSprites(tileSet)
  tileSet:addColor('OutOfBounds', {0, 0, 0})
  tileSet:addColor('OutOfView', {80, 80, 80})
  tileSet:addShape('Wall', images.wall())
  tileSet:addShape('Apple', images.apple())
  if self._settings.showRespawnProbability then
    -- Use probability to color respawn tile.
    local colors = {{255, 0, 0}, {255, 255, 0}, {0, 255, 0}}
    local count = #self._settings.appleRespawnProbabilities
    local maxP = 1e-3
    for i, p in ipairs(self._settings.appleRespawnProbabilities) do
      if p > maxP then
        maxP = p
      end
    end
    for i, p in ipairs(self._settings.appleRespawnProbabilities) do
      local color = interpColor(colors, math.sqrt(p) / math.sqrt(maxP))
      tileSet:addColor('apple.wait.' .. i, color)
    end
  end
end

function Simulation:worldConfig()
  local settings = self._settings
  local config = {
      outOfBoundsSprite = 'OutOfBounds',
      outOfViewSprite = 'OutOfView',
      updateOrder = {'fruit'},
      renderOrder = {'logic', 'pieces'},
      customSprites = {},
      hits = {},
      states = {
          wall = {
              layer = 'pieces',
              sprite = 'Wall',
          },
          ['spawn.any'] = {groups = {'spawn.any'}},
          apple = {
              layer = 'logic',
              sprite = 'Apple',
          },
          ['apple.wait'] = {layer = 'waitCalc'},
          ['apple.possible'] = {},
      }
  }
  local waitNames = {}
  for i, prob in ipairs(settings.appleRespawnProbabilities) do
    local name = 'apple.wait.' .. i
    local spriteName = settings.showRespawnProbability and name or nil
    waitNames[i] = name
    table.insert(config.updateOrder, {name = name, func = 'respawn'})
    local waiters = prob > 0 and 'waiter'
    config.states['apple.wait.' .. i] = {
        layer = 'wait',
        groups = {name},
        sprite = spriteName,
    }
  end
  self._waitNames = waitNames

  if settings.showRespawnProbability then
    table.insert(config.renderOrder, 1, 'wait')
  end
  return config
end

function Simulation:textMap()
  local map = maps[self._settings.mapName]
  if not map then
    error('missing map: ' .. self._settings.mapName)
  end
  return map
end

function Simulation:addObservations(tileSet, world, observations)
  local worldLayerView = world:createView{layout = self:textMap().layout}

  local worldView = tile.Scene{shape = worldLayerView:gridSize(), set = tileSet}
  local spec = {
      name = 'WORLD.RGB',
      type = 'tensor.ByteTensor',
      shape = worldView:shape(),
      func = function(grid)
        return worldView:render(worldLayerView:observation{grid = grid})
      end
  }
  observations[#observations + 1] = spec
end

local function _getNumLiveNeighbors(grid, pos, radius)
  local num = 0
  for _ in pairs(grid:queryDiamond('logic', pos, radius)) do
    num = num + 1
  end
  return num
end

-- avatars require reward in user state.
function Simulation:stateCallbacks(avatars)
  local settings = self._settings
  local radius = settings.seedRadius
  local stateCallbacks = {}
  stateCallbacks.wall = {onHit = true}
  local apple = {}
  function apple.onAdd(grid, apple)
    local pos = grid:position(apple)
    for piece in pairs(grid:queryDiamond('wait', pos, radius)) do
      grid:setState(piece, 'apple.wait')
    end
  end
  apple.onContact = {avatar = {}}
  function apple.onContact.avatar.enter(grid, applePiece, avatarPiece)
    local avatarState = grid:userState(avatarPiece)
    local pos = grid:position(applePiece)
    local numInRadius = _getNumLiveNeighbors(grid, pos, radius)
    local rewardAmountModifier = 0
    if numInRadius == 1 then
      rewardAmountModifier = avatarState.rewardForEatingLastAppleInRadius
    end
    local rewardAmount = 1 + rewardAmountModifier
    avatarState.reward = avatarState.reward + rewardAmount
    grid:setState(applePiece, 'apple.wait')
    for piece in pairs(grid:queryDiamond('wait', pos, radius)) do
      grid:setState(piece, 'apple.wait')
    end
  end

  local appleW = {}
  function appleW.onAdd(grid, appleWait)
      local pos = grid:position(appleWait)
      local waitNames = self._waitNames
      local count = 1
      for piece in pairs(grid:queryDiamond('logic', pos, radius)) do
        count = count + 1
        if count == #waitNames then
          break
        end
      end
      grid:setState(appleWait, waitNames[count])
  end

  local appleWaitCallback = {onUpdate = {}}
  appleWaitCallback.onUpdate.respawn = function(grid, piece)
    grid:setState(piece, 'apple')
  end

  local applePossible = {}
  local initialAppleProbability = settings.initialAppleProbability
  function applePossible.onAdd(grid, piece)
    if random:uniformReal(0, 1) <= initialAppleProbability then
      grid:setState(piece, 'apple')
    end
  end

  stateCallbacks.apple = apple
  stateCallbacks['apple.wait'] = appleW
  for i = 1, #self._settings.appleRespawnProbabilities do
    stateCallbacks['apple.wait.' .. i] = appleWaitCallback
  end
  stateCallbacks['apple.possible'] = applePossible
  return stateCallbacks
end

function Simulation:start(grid)
  for i, prob in ipairs(self._settings.appleRespawnProbabilities) do
    local name = 'apple.wait.' .. i
    grid:setUpdater{update = name, group = name, probability = prob}
  end
end

function Simulation:update(grid) end

return {Simulation = Simulation}
