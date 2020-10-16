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

local api_factory = require 'worlds.common.api_factory'
local simulation = require 'simulation'
local avatar_list = require 'avatar_list'

return api_factory.apiFactory{
    Simulation = simulation.Simulation,
    AvatarList = avatar_list.AvatarList,
    settings = {
        -- Scale each sprite to a square of size `spriteSize` X `spriteSize`.
        spriteSize = 16,
        -- End the episode after this many frames if not already ended.
        episodeLengthFrames = 1000,
        avatars = {
            bots = 0,
            -- Override default settings for each individual avatar.
            ['player.%default'] = {
                -- The cooldown time (frames) to wait between zap actions.
                minFramesBetweenZaps = 10,
                -- Length of the zapping beam.
                beamLength = 3,
                -- The zapping beam extends this many units to either side.
                beamRadius = 1,
                -- Configure the local view window to be used by all players.
                view = {
                    -- For the RGB view, these are in units of spriteSize.
                    left = 2,
                    right = 2,
                    forward = 3,
                    backward = 1,
                    otherPlayersLookSame = true,
                    thisPlayerLooksBlue = true,
                },
            },
        },
    }
}
