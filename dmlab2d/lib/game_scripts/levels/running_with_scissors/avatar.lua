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
local read_settings = require 'common.read_settings'
local random = require 'system.random'
local images = require 'images'


local _COMPASS = {'N', 'E', 'S', 'W'}

local _PLAYER_NAMES = {
    'blue',
    'mint',
    'carrot',
    'pink',
    'maroon',
    'beige',
    'brown',
    'coral',
    'cyan',
    'gold',
    'green',
    'grey',
    'lavender',
    'lime',
    'magenta',
    'navy',
    'olive',
    'purple',
    'red',
    'teal',
}

local _PLAYER_ACTION_ORDER = {
    'move',
    'turn',
    'fireZap'
}

local _PLAYER_ACTION_SPEC = {
    move = {default = 0, min = 0, max = #_COMPASS},
    turn = {default = 0, min = -1, max = 1},
    fireZap = {default = 0, min = 0, max = 1},
}

local _ITEMS = {
    'rock',
    'paper',
    'scissors',
}

local _ITEM_TO_INDEX = {}

for i, itemName in ipairs(_ITEMS) do
    _ITEM_TO_INDEX[itemName] = i
end


local Avatar = class.Class()

-- Class function returning default settings.
function Avatar.defaultSettings()
  return {
      minFramesBetweenZaps = 10,
      beamLength = 3,
      beamRadius = 1,
      -- Use `rewardOnCollectingSpecificItem` to train (mostly) pure strategies.
      -- To enable it, set it equal to one of {'rock', 'paper', or 'scissors'}.
      rewardOnCollectingSpecificItem = '',
      view = {
          left = 5,
          right = 5,
          forward = 9,
          backward = 1,
          centered = false,
          otherPlayersLookSame = true,
          thisPlayerLooksBlue = true,
      },
  }
end

function Avatar:__init__(kwargs)
  self._avatars = kwargs.avatars
  self._settings = kwargs.settings
  self._simSettings = kwargs.simSettings
  self._index = kwargs.index
  self._isBot = kwargs.isBot
  self._type = 'player.' .. kwargs.index
  self._waitState = 'playerWait.' .. kwargs.index
  self._playerState = {}
end

function Avatar:addConfigs(worldConfig)
  local id = self._index
  worldConfig.states[self._type] = {
      groups = {'players'},
      layer = 'pieces',
      sprite = 'Player.' .. id,
      contact = 'avatar',
  }
  worldConfig.states[self._waitState] = {
      groups = {'players', 'playersWait'},
  }
end

function Avatar:addSprites(tileSet)
  local id = self._index
  tileSet:addShape('Player.' .. id, images.playerShape(_PLAYER_NAMES[id]))
end

function Avatar:addReward(amount)
  self._playerState.reward = self._playerState.reward + amount
end

function Avatar:addItem(name, amount)
  if name == self._settings.rewardOnCollectingSpecificItem then
    self:addReward(1)
  end
  self._playerState.items(_ITEM_TO_INDEX[name]):add(1)
end

function Avatar:getItems()
  return self._playerState.items
end

function Avatar:_startFrame()
  self._playerState.reward = 0
end

function Avatar:getReward(id)
  return self._playerState.reward
end

function Avatar:discreteActionSpec(actSpec)
  self._actionSpecStartOffset = #actSpec
  local id = tostring(self._index)
  for a, actionName in ipairs(_PLAYER_ACTION_ORDER) do
    local action = _PLAYER_ACTION_SPEC[actionName]
    table.insert(actSpec, {
        name = id .. '.' .. actionName,
        min = action.min,
        max = action.max,
    })
  end
end

function Avatar:discreteActions(actions)
  local psActions = self._playerState.actions
  for a, actionName in ipairs(_PLAYER_ACTION_ORDER) do
     psActions[actionName] = actions[a + self._actionSpecStartOffset]
  end
end

function Avatar:addObservations(tileSet, world, observations, avatarCount)
  local settings = self._settings
  local id = self._index
  local stringId = tostring(id)
  local playerViewConfig = {
      left = settings.view.left,
      right = settings.view.right,
      forward = settings.view.forward,
      backward = settings.view.backward,
      centered = settings.view.centered,
      set = tileSet,
      spriteMap = {}
  }

  if settings.view.otherPlayersLookSame then
    for otherId = 1, avatarCount do
      if id == otherId then
        playerViewConfig.spriteMap['Player.' .. stringId] = 'Player.1'
      else
        playerViewConfig.spriteMap['Player.' .. otherId] = 'Player.2'
      end
    end
  elseif settings.view.thisPlayerLooksBlue then
    for otherId = 1, avatarCount do
      if id == otherId then
        playerViewConfig.spriteMap['Player.' .. stringId] = 'Player.1'
      elseif otherId == 1 then
        playerViewConfig.spriteMap['Player.' .. otherId] = 'Player.' .. id
      end
    end
  end

  observations[#observations + 1] = {
      name = stringId .. '.REWARD',
      type = 'Doubles',
      shape = {},
      func = function(grid)
        return self._playerState.reward
      end
  }

  observations[#observations + 1] = {
      name = stringId .. '.INVENTORY',
      type = 'tensor.DoubleTensor',
      shape = {3},
      func = function(grid)
        return self._playerState.items
      end
  }

  local playerLayerView = world:createView(playerViewConfig)
  local playerLayerViewSpec =
      playerLayerView:observationSpec(stringId .. '.LAYER')
  playerLayerViewSpec.func = function(grid)
    return playerLayerView:observation{
        grid = grid,
        piece = self._piece,
        orientation = 'N'
    }
  end
  observations[#observations + 1] = playerLayerViewSpec

  local playerView = tile.Scene{
      shape = playerLayerView:gridSize(),
      set = tileSet
  }

  local spec = {
      name = stringId .. '.RGB',
      type = 'tensor.ByteTensor',
      shape = playerView:shape(),
      func = function(grid)
        local layer_observation = playerLayerView:observation{
            grid = grid,
            piece = self._piece,
        }
        return playerView:render(layer_observation)
      end
  }
  observations[#observations + 1] = spec
end

function Avatar:update(grid)
  self:_startFrame()
  local psActions = self._playerState.actions
  if self._isBot then
    for _, actionName in ipairs(_PLAYER_ACTION_ORDER) do
      local action = _PLAYER_ACTION_SPEC[actionName]
      psActions[actionName] = random:uniformInt(action.min, action.max)
    end
  end
end

function Avatar:start(grid, locator)
  self:_startFrame()
  local actions = {}
  for a, actionName in ipairs(_PLAYER_ACTION_ORDER) do
    local action = _PLAYER_ACTION_SPEC[actionName]
    actions[actionName] = action.default
  end

  self._playerState = {
      reward = 0,
      coolFireAlt = 0,
      coolFire = 0,
      actions = actions,
      items = tensor.DoubleTensor(#_ITEMS):fill(1.0)
  }
  local targetTransform = grid:transform(locator)
  targetTransform.orientation = _COMPASS[random:uniformInt(1, #_COMPASS)]
  local piece = grid:createPiece(self._type, targetTransform)
  self._piece = piece
  return piece
end

function Avatar:addPlayerCallbacks(callbacks)
  local id = self._index
  local playerSetting = self._settings
  callbacks[self._type] = {
      -- Declare callbacks to be called by updaters.
      onUpdate = {
          ['move'] = function(grid, piece)
            local actions = self._playerState.actions
            if actions.turn ~= 0 then
              grid:turn(piece, actions.turn)
            end
            if actions.move ~= 0 then
              grid:moveRel(piece, _COMPASS[actions.move])
            end
          end,

          ['fire'] = function(grid, piece)
            local state = self._playerState
            local actions = state.actions
            grid:hitBeam(piece, 'direction', 1, 0)

            if playerSetting.minFramesBetweenZaps >= 0 then
              if state.coolFire > 0 then
                state.coolFire = state.coolFire - 1
              else
                if actions['fireZap'] == 1 then
                  state.coolFire = playerSetting.minFramesBetweenZaps
                  grid:hitBeam(piece,
                               'zap',
                               playerSetting.beamLength,
                               playerSetting.beamRadius)
                end
              end
            end
          end,
      },

      -- Declare hit callbacks to be called on the piece that was hit by a beam.
      onHit = {
          ['zap'] = function(grid, player, instigator)
            -- The zapper avatar is the row player.
            local zapperIndex = self._avatars.pieceToIndex[instigator]
            local zapperAvatar = self._avatars.avatarList[zapperIndex]
            local zapperResources = zapperAvatar:getItems():reshape{1, 3}
            local rowPlayerNumItemsCollected = zapperResources:sum()
            -- Get the row player's strategy profile.
            local rowProfile = zapperResources:reshape{1, #_ITEMS}:clone()
            rowProfile = rowProfile:div(rowPlayerNumItemsCollected)

            -- The struck avatar is the column player.
            local struckAvatarResources = self:getItems():reshape{1, 3}
            local colPlayerNumItemsCollected = struckAvatarResources:sum()
            -- Get the column player's strategy profile.
            local colProfile = struckAvatarResources:reshape{#_ITEMS, 1}:clone()
            colProfile = colProfile:div(colPlayerNumItemsCollected)

            local m = self._avatars.resolutionMatrix
            local mT = m:clone():transpose(1, 2)

            local rowReward = rowProfile:mmul(m):mmul(colProfile):reshape{
                }:val()
            local colReward = rowProfile:mmul(mT):mmul(colProfile):reshape{
                }:val()

            zapperAvatar:addReward(rowReward)
            self:addReward(colReward)

            -- End the episode on this frame.
            self._simSettings.continueEpisodeAfterThisFrame = false
            -- Beams do not pass through hit avatars.
            return true
          end,
      }
  }

  callbacks[self._waitState] = {
      respawnUpdate = function(grid, player, frames)
        grid:teleportToGroup(player, 'spawnPoints', self._type)
      end
  }
end

return {Avatar = Avatar}
