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
      mudSpawnProbability = 0.5,

      -- Base probability of an apple respawning each step.
      appleRespawnProbability = 0.05,

      -- Waste density threshold at which the respawn probability of apples
      -- and lemons is set to 0.
      thresholdDepletion = 0.4,

      -- Waste density threshold below which apples respawn at their base
      -- probability.
      thresholdRestoration = 0.0,

      -- Number of frames after episode starts before dirt starts to accumulate.
      dirtGrowthStartTime = 50,
  }
end

function Simulation:__init__(kwargs)
  self._settings = kwargs.settings
end

function Simulation:addSprites(tileSet)
  tileSet:addColor('OutOfBounds', {0, 0, 0})
  tileSet:addColor('OutOfView', {80, 80, 80})
  tileSet:addSpritePath('Wall', images.path('environment/wall'), false)
  tileSet:addSpritePath('Apple', images.path('entities/apple'), false)
  tileSet:addSpritePath('Water', images.path('environment/water'), false)
  tileSet:addSpritePath('Mud', images.path('environment/mud'), false)
end

function Simulation:worldConfig()
  return {
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
          spawnPoint = {groups = {'spawnPoints'}},
          apple = {
              layer = 'logic',
              sprite = 'Apple',
          },
          ['apple.wait'] = {
              layer = 'logic',
              groups = {'apple.wait'},
          },
          ['river.water'] = {
              groups = {'river.water'},
              layer = 'logic',
              sprite = 'Water',
          },
          ['river.mud'] = {
              layer = 'logic',
              sprite = 'Mud',
          },
          stream = {
              layer = 'logic',
              sprite = 'Water',
          },
      }
  }
end

function Simulation:textMap()
  return maps[self._settings.mapName]
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

-- avatars require addContrib and addReward
function Simulation:stateCallbacks(avatars)
  self._mudCount = 0
  self._riverCount = 0
  return {
      wall = {
          onHit = true,
      },
      apple = {
          onContact = {
              avatar = {
                  enter = function(grid, applePiece, avatarPiece)
                    local avatarState = grid:userState(avatarPiece)
                    if avatarState.reward then
                      avatarState.reward = avatarState.reward + 1
                    end
                    grid:setState(applePiece, 'apple.wait')
                  end
              }
          }
      },
      ['apple.wait'] = {
          onUpdate = {
              fruit = function(grid, piece, framesOld)
                grid:setState(piece, 'apple')
              end
          }
      },
      ['river.mud'] = {
          onAdd = function(grid, mud)
            self._mudCount = self._mudCount + 1
          end,
          onRemove = function(grid, mud)
            self._mudCount = self._mudCount - 1
          end,
          onHit = {
              clean = function(grid, mud, avatarPiece)
                local avatarState = grid:userState(avatarPiece)
                if avatarState.contrib then
                  avatarState.contrib = avatarState.contrib + 1
                end
                -- Add a (pseudo)-reward for contribution, its value will be
                -- zero for most use cases. It can be used to train specialists.
                if avatarState.reward and avatarState.rewardForContribution then
                  avatarState.reward =
                      avatarState.reward + avatarState.rewardForContribution
                end
                grid:setState(mud, 'river.water')
                -- The cleaning beam does not pass through mud it contacts.
                return true
              end,
          }
      },
      ['river.water'] = {
          onAdd = function(grid, river)
            self._riverCount = self._riverCount + 1
          end,
          onRemove = function(grid, river)
            self._riverCount = self._riverCount - 1
          end,
      },
  }
end

function Simulation:start(grid)
  self._stepsSinceStarted = 0
end

function Simulation:update(grid)
  local dirtGrowthStartTime = self._settings.dirtGrowthStartTime

  local mudFraction = self._mudCount / (self._mudCount + self._riverCount)
  local depl = self._settings.thresholdDepletion
  local rest = self._settings.thresholdRestoration
  local interp = (mudFraction - depl) / (rest - depl)
  grid:setUpdater{update = 'fruit', group = 'apple.wait', probability =
        self._settings.appleRespawnProbability * interp}

  if self._riverCount > 0 then
    local addMud = random:uniformReal(0, 1) < self._settings.mudSpawnProbability
    if addMud and self._stepsSinceStarted > dirtGrowthStartTime then
      local randomMud = grid:groupRandom(random, 'river.water')
      if randomMud then
        grid:setState(randomMud, 'river.mud')
      end
    end
  end
  self._stepsSinceStarted = self._stepsSinceStarted + 1
end

return {Simulation = Simulation}
