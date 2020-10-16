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

local tile = require 'system.tile'
local class = require 'common.class'
local tensor = require 'system.tensor'
local read_settings = require 'common.read_settings'
local random = require 'system.random'
local images = require 'images'
local avatar = require 'avatar'

local AvatarList = class.Class()

function AvatarList.defaultSettings()
  return {
      framesTillRespawn = 25,
      player = read_settings.default(avatar.Avatar.defaultSettings()),
  }
end

function AvatarList:__init__(kwargs)
  local numAvatars = kwargs.numPlayers
  self._framesTillRespawn = kwargs.settings.framesTillRespawn
  self._simSettings = kwargs.simSettings
  self._numPlayers = kwargs.numPlayers
  self._numAvatars = numAvatars
  self._playerZapMatrix = tensor.Int32Tensor(numAvatars, numAvatars)
  self._avatarList = {}
  for i = 1, numAvatars do
    self._avatarList[i] = avatar.Avatar{
        index = i,
        settings = kwargs.settings.player[i],
        simSettings = kwargs.simSettings,
    }
  end
end

function AvatarList:addConfigs(worldConfig)
  table.insert(worldConfig.updateOrder, 1, 'respawn')
  table.insert(worldConfig.updateOrder, 2, 'move')
  table.insert(worldConfig.updateOrder, 3, 'zap')
  worldConfig.hits.direction = {
      layer = 'direction',
      sprite = 'Direction',
  }
  worldConfig.hits.zapHit = {
      layer = 'zap',
      sprite = 'BeamZap',
  }

  table.insert(worldConfig.renderOrder, 'zap')
  table.insert(worldConfig.renderOrder, 'direction')

  for _, av in ipairs(self._avatarList) do
    av:addConfigs(worldConfig)
  end
end

function AvatarList:addSprites(tileSet)
  tileSet:addColor('Direction', {100, 100, 100, 200})
  tileSet:addColor('BeamZap', {252, 252, 106})
  for _, av in ipairs(self._avatarList) do
    av:addSprites(tileSet)
  end
end

function AvatarList:_startFrame()
  self._playerZapMatrix:fill(0)
end

function AvatarList:getReward(grid)
  local reward = 0
  for _, av in ipairs(self._avatarList) do
    reward = reward + av:getReward(grid)
  end
  return reward
end

function AvatarList:discreteActionSpec()
  local act = {}
  for _, av in ipairs(self._avatarList) do
    av:discreteActionSpec(act)
  end
  return act
end


function AvatarList:discreteActions(grid, actions)
  for _, av in ipairs(self._avatarList) do
    av:discreteActions(grid, actions)
  end
end

function AvatarList:addObservations(tileSet, world, observations)
  local numAvatars = self._numAvatars
  observations[#observations + 1] = {
      name = 'WORLD.ZAP_COUNT',
      type = 'Int32s',
      shape = {numAvatars, numAvatars},
      func = function(grid)
        return self._playerZapMatrix
      end
  }

  for _, av in ipairs(self._avatarList) do
    av:addObservations(tileSet, world, observations, numAvatars)
  end
end

function AvatarList:update(grid)
  self:_startFrame()
  for _, av in ipairs(self._avatarList) do
    av:update(grid)
  end
end

function AvatarList:start(grid)
  grid:setUpdater{update = 'zap', group = 'players'}
  grid:setUpdater{update = 'move', group = 'players'}
  grid:setUpdater{
      update = 'respawn',
      group = 'players.wait',
      startFrame = self._framesTillRespawn,
  }
  local points = grid:groupShuffledWithCount(
      random, 'spawn.any', self._numAvatars)

  assert(#points == self._numAvatars, "Insufficient spawn points!")
  local _playerZapMatrix = self._playerZapMatrix
  for i, point in ipairs(points) do
    local avatarPiece = self._avatarList[i]:start(
        grid, point, self._playerZapMatrix(i))
  end
  self:_startFrame()
end

function AvatarList:addPlayerCallbacks(callbacks)
  for _, av in ipairs(self._avatarList) do
    av:addPlayerCallbacks(callbacks)
  end
end

return {AvatarList = AvatarList}
