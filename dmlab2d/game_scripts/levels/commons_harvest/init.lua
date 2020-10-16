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

local api_factory = require 'worlds.common.api_factory'
local simulation = require 'simulation'
local avatar_list = require 'avatar_list'

return function(mapName)
  if mapName == '' then
    mapName = 'default'
  end
  return api_factory.apiFactory{
      Simulation = simulation.Simulation,
      AvatarList = avatar_list.AvatarList,
      settings = {
          simulation = {mapName = mapName},
          episodeLengthFrames = 1000,
          spriteSize = 8,
      }
  }
end
