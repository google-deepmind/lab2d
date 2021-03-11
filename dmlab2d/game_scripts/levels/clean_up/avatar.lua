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
    'fireClean',
    'fireFine',
}

local _PLAYER_ACTION_SPEC = {
    move = {default = 0, min = 0, max = #_COMPASS},
    turn = {default = 0, min = -1, max = 1},
    fireClean = {default = 0, min = 0, max = 1},
    fireFine = {default = 0, min = 0, max = 1},
}


local Avatar = class.Class()

-- Class function returning default settings.
function Avatar.defaultSettings()
  return {
      -- Player able fine others
      fineWait = 10,
      cleanWait = 2,
      playerResetsAfterFined = false,
      rewardForFining = -1,
      rewardForBeingFined = -50,
      -- Set `rewardForContribution` to nonzero values in order to train
      -- (mostly) pure strategies: cooperators if positive and defectors if
      -- negative.
      rewardForContribution = 0,
      view = {
          left = 5,
          right = 5,
          forward = 9,
          backward = 1,
          centered = false,
          otherPlayersLookSame = false,
          followPlayer = true,
          thisPlayerLooksBlue = true,
      },
  }
end

function Avatar:__init__(kwargs)
  self._avatars = kwargs.avatars
  self._settings = kwargs.settings
  self._index = kwargs.index
  self._isBot = kwargs.isBot
  self._state = 'player.' .. kwargs.index
  self._waitState = 'player.' .. kwargs.index .. '.wait'
  self._playerState = {}
  self._simSetting = kwargs.simSettings
end

function Avatar:addConfigs(worldConfig)
  local id = self._index
  worldConfig.states[self._state] = {
      groups = {'players'},
      layer = 'pieces',
      sprite = 'Player.' .. id,
      contact = 'avatar',
  }
  worldConfig.states[self._waitState] = {
      groups = {'players', 'players.wait'},
  }
end

function Avatar:addSprites(tileSet)
  local id = self._index
  tileSet:addShape('Player.' .. id, images.playerShape(_PLAYER_NAMES[id]))
end

function Avatar:_startFrame(grid)
  grid:userState(self._piece).reward = 0
  grid:userState(self._piece).contrib = 0
end

function Avatar:getReward(grid)
  return grid:userState(self._piece).reward
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

function Avatar:discreteActions(grid, actions)
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
        return grid:userState(self._piece).reward
      end
  }

  observations[#observations + 1] = {
      name = stringId .. '.CONTRIB',
      type = 'Doubles',
      shape = {},
      func = function(grid)
        return grid:userState(self._piece).contrib
      end
  }

  observations[#observations + 1] = {
      name = stringId .. '.POSITION',
      type = 'Doubles',
      shape = {2},
      func = function(grid)
        return tensor.DoubleTensor(grid:position(self._piece))
      end
  }

  local playerLayerView = world:createView(playerViewConfig)
  local playerLayerViewSpec =
      playerLayerView:observationSpec(stringId .. '.LAYER')
  playerLayerViewSpec.func = function(grid)
    return playerLayerView:observation{
        grid = grid,
        piece = settings.view.followPlayer and self._piece or nil,
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
            piece = settings.view.followPlayer and self._piece or nil,
        }
        return playerView:render(layer_observation)
      end
  }
  observations[#observations + 1] = spec
end

function Avatar:update(grid)
  self:_startFrame(grid)
  local psActions = self._playerState.actions
  if self._isBot then
    for _, actionName in ipairs(_PLAYER_ACTION_ORDER) do
      local action = _PLAYER_ACTION_SPEC[actionName]
      psActions[actionName] = random:uniformInt(action.min, action.max)
    end
  end
end

function Avatar:start(grid, locator)
  local actions = {}
  for a, actionName in ipairs(_PLAYER_ACTION_ORDER) do
    local action = _PLAYER_ACTION_SPEC[actionName]
    actions[actionName] = action.default
  end
  self._playerState = {
      coolClean = 0,
      coolFine = 0,
      actions = actions
  }
  local targetTransform = grid:transform(locator)
  targetTransform.orientation = _COMPASS[random:uniformInt(1, #_COMPASS)]
  local piece = grid:createPiece(self._state, targetTransform)
  self._piece = piece
  grid:setUserState(piece, {
      reward = 0,
      contrib = 0,
      rewardForFining = self._settings.rewardForFining,
      rewardForBeingFined = self._settings.rewardForBeingFined,
      rewardForContribution = self._settings.rewardForContribution,
  })
  self:_startFrame(grid)
  return piece
end

function Avatar:addPlayerCallbacks(callbacks)
  local id = self._index
  local playerSetting = self._settings
  local playerResetsAfterFined = self._settings.playerResetsAfterFined
  callbacks[self._state] = {
      onUpdate = {
          move = function(grid, piece)
            local actions = self._playerState.actions
            if actions.turn ~= 0 then
              grid:turn(piece, actions.turn)
            end
            if actions.move ~= 0 then
              --grid:orientation(piece, _COMPASS[actions.move])
              grid:moveRel(piece, _COMPASS[actions.move])
            end
          end,
          fire = function(grid, piece)
            local state = self._playerState
            local actions = state.actions
            grid:hitBeam(piece, "direction", 1, 0)

            if playerSetting.fineWait >= 0 then
              if state.coolFine > 0 then
                state.coolFine = state.coolFine - 1
              else
                if actions.fireFine == 1 then
                  state.coolFine = playerSetting.fineWait
                  grid:hitBeam(piece, "fine", 5, 1)
                end
              end
            end
            if playerSetting.cleanWait >= 0 then
              if state.coolClean > 0 then
                state.coolClean = state.coolClean - 1
              else
                if actions.fireClean == 1 then
                  state.coolClean = playerSetting.cleanWait
                  grid:hitBeam(piece, "clean", 5, 1)
                end
              end
            end
          end,
      },
      onHit = {
          fine = function(grid, player, instigator)
            local zapperState = grid:userState(instigator)
            local playerState = grid:userState(player)
            zapperState.reward = (zapperState.reward +
                                  zapperState.rewardForFining)
            playerState.reward = (playerState.reward +
                                  playerState.rewardForBeingFined)
            if playerResetsAfterFined then
              grid:setState(player, self._waitState)
            end
            -- Beams do not pass through zapped players.
            return true
          end,
      }
  }

  callbacks[self._waitState] = {
      respawnUpdate = function(grid, player, frames)
        grid:teleportToGroup(player, 'spawnPoints', self._state)
      end
  }
end

return {Avatar = Avatar}
