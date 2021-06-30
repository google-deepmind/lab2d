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
local tensor = require 'system.tensor'
local random = require 'system.random'
local read_settings = require 'common.read_settings'

local avatar = require 'avatar'
local images = require 'images'

local AvatarList = class.Class()

function AvatarList.defaultSettings()
  return {
      bots = 0,
      player = read_settings.default(avatar.Avatar.defaultSettings()),
      resolutionMatrix = {
      --   R,  P,  S
          {0, -1, 1},  -- R
          {1, 0, -1},  -- P
          {-1, 1, 0},  -- S
      },
  }
end

function AvatarList:__init__(kwargs)
  local numAvatars = kwargs.settings.bots + kwargs.numPlayers
  self._settings = kwargs.settings
  self._simSettings = kwargs.simSettings
  self._numPlayers = kwargs.numPlayers
  self._numAvatars = numAvatars
  self.avatarList = {}
  for i = 1, numAvatars do
    self.avatarList[i] = avatar.Avatar{
        avatars = self,
        index = i,
        settings = kwargs.settings.player[i],
        isBot = i > kwargs.numPlayers,
        simSettings = kwargs.simSettings,
    }
  end
  self.pieceToIndex = {}
  self.indexToPiece = {}
end

function AvatarList:settings()
  return self._settings
end

function AvatarList:addConfigs(worldConfig)
  table.insert(worldConfig.updateOrder, 1, 'respawn')
  table.insert(worldConfig.updateOrder, 2, 'move')
  table.insert(worldConfig.updateOrder, 3, 'fire')

  worldConfig.hits['direction'] = {
      layer = 'direction',
      sprite = 'Direction',
  }
  worldConfig.hits['zap'] = {
      layer = 'beamFine',
      sprite = 'BeamFine',
  }

  table.insert(worldConfig.renderOrder, 'beamFine')
  table.insert(worldConfig.renderOrder, 'direction')

  for _, av in ipairs(self.avatarList) do
    av:addConfigs(worldConfig)
  end
end

function AvatarList:addSprites(tileSet)
  tileSet:addColor('Direction', {100, 100, 100, 200})
  tileSet:addColor('BeamFine', {252, 252, 106})
  for _, av in ipairs(self.avatarList) do
    av:addSprites(tileSet)
  end
end

function AvatarList:addReward(piece, amount)
  self.avatarList[self.pieceToIndex[piece]]:addReward(amount)
end

function AvatarList:addItem(piece, item, amount)
  self.avatarList[self.pieceToIndex[piece]]:addItem(item, amount)
end

function AvatarList:getReward(grid)
  local reward = 0
  for _, av in ipairs(self.avatarList) do
    reward = reward + av:getReward(grid)
  end
  return reward
end

function AvatarList:discreteActionSpec()
  local act = {}
  for _, av in ipairs(self.avatarList) do
    av:discreteActionSpec(act)
  end
  return act
end


function AvatarList:discreteActions(grid, actions)
  for _, av in ipairs(self.avatarList) do
    av:discreteActions(actions)
  end
end

function AvatarList:addObservations(tileSet, world, observations)
  local avatarCount = self._numAvatars
  for _, av in ipairs(self.avatarList) do
    av:addObservations(tileSet, world, observations, avatarCount)
  end
end

function AvatarList:update(grid)
  for _, av in ipairs(self.avatarList) do
    av:update(grid)
  end
end

function AvatarList:start(grid)
  self.resolutionMatrix = tensor.DoubleTensor(self._settings.resolutionMatrix)
  grid:setUpdater{update = 'fire', group = 'players'}
  grid:setUpdater{update = 'move', group = 'players'}
  grid:setUpdater{update = 'respawn', group = 'playersWait', startFrame = 10}
  local points = grid:groupShuffledWithCount(
      random, 'spawnPoints', self._numAvatars)

  assert(#points == self._numAvatars, 'Insufficient spawn points!')
  for i, point in ipairs(points) do
    local avatarPiece = self.avatarList[i]:start(grid, point)
    self.pieceToIndex[avatarPiece] = i
    self.indexToPiece[i] = avatarPiece
  end
end

function AvatarList:addPlayerCallbacks(callbacks)
  for _, av in ipairs(self.avatarList) do
    av:addPlayerCallbacks(callbacks)
  end
end

return {AvatarList = AvatarList}
