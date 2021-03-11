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

local helpers = require 'common.helpers'
local log = require 'common.log'
local class = require 'common.class'
local tile = require 'system.tile'
local random = require 'system.random'

local images = require 'images'
local maps = require 'maps'

local Simulation = class.Class()

function Simulation.defaultSettings()
  return {mapName = 'default'}
end

function Simulation:__init__(kwargs)
  self._settings = kwargs.settings

  return self
end

function Simulation:settings()
  return self._settings
end

function Simulation:addSprites(tileSet)
  tileSet:addColor('OutOfBounds', {0, 0, 0})
  tileSet:addColor('OutOfView', {80, 80, 80})
  tileSet:addShape('Wall', images.wall)
  tileSet:addShape('Rock', images.rock)
  tileSet:addShape('Paper', images.paper)
  tileSet:addShape('Scissors', images.scissors)
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
          spawnPoint = {
              groups = {'spawnPoints'},
              layer = 'invisible',
              sprite = 'StartPoints',
          },
          random = {
              layer = 'logic',
          },
          rock = {
              layer = 'logic',
              sprite = 'Rock',
          },
          paper = {
              layer = 'logic',
              sprite = 'Paper',
          },
          scissors = {
              layer = 'logic',
              sprite = 'Scissors',
          },
          pickedUp = {},
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
  return {
      ['wall'] = {
          onHit = true,
      },
      ['random'] = {
          onAdd = function(grid, applePiece)
            local choice = {'rock', 'paper', 'scissors'}
            grid:setState(applePiece, choice[random:uniformInt(1, 3)])
          end
      },
      ['rock'] = {
          onContact = {
              ['avatar'] = {
                  enter = function(grid, piece, avatarPiece)
                    avatars:addItem(avatarPiece, 'rock', 1)
                    grid:setState(piece, 'pickedUp')
                  end,
              }
          }
      },
      ['paper'] = {
          onContact = {
              ['avatar'] = {
                  enter = function(grid, piece, avatarPiece)
                    avatars:addItem(avatarPiece, 'paper', 1)
                    grid:setState(piece, 'pickedUp')
                  end,
              }
          }
      },
      ['scissors'] = {
          onContact = {
              ['avatar'] = {
                  enter = function(grid, piece, avatarPiece)
                    avatars:addItem(avatarPiece, 'scissors', 1)
                    grid:setState(piece, 'pickedUp')
                  end,
              }
          }
      },
  }
end

function Simulation:start(grid)
  self._settings.continueEpisodeAfterThisFrame = true
end

function Simulation:update(grid)
  -- Return value `true` continues the episode, `false` ends it immediately.
  return self:_continue()
end

function Simulation:_continue()
  return self._settings.continueEpisodeAfterThisFrame
end

return {Simulation = Simulation}
