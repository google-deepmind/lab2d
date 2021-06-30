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

local LAYOUTS = {}

LAYOUTS.default = [[
***********************
*P r  r  a   a  p  p P*
*                     *
*P r  r  a   a  p  p P*
*                     *
*P r  r  a   a  p  p P*
*                     *
*     P    P    P     *
*                     *
*P s  s  a   a  a  a P*
*                     *
*P s  s  a   a  a  a P*
*                     *
*P s  s  a   a  a  a P*
***********************]]

LAYOUTS.no_rock = [[
***********************
*P       a   a  p  p P*
*                     *
*P       a   a  p  p P*
*                     *
*P       a   a  p  p P*
*                     *
*     P    P    P     *
*                     *
*P s  s  a   a  a  a P*
*                     *
*P s  s  a   a  a  a P*
*                     *
*P s  s  a   a  a  a P*
***********************]]

LAYOUTS.no_paper = [[
***********************
*P r  r  a   a       P*
*                     *
*P r  r  a   a       P*
*                     *
*P r  r  a   a       P*
*                     *
*     P    P    P     *
*                     *
*P s  s  a   a  a  a P*
*                     *
*P s  s  a   a  a  a P*
*                     *
*P s  s  a   a  a  a P*
***********************]]

LAYOUTS.no_scissors = [[
***********************
*P r  r  a   a  p  p P*
*                     *
*P r  r  a   a  p  p P*
*                     *
*P r  r  a   a  p  p P*
*                     *
*     P    P    P     *
*                     *
*P       a   a  a  a P*
*                     *
*P       a   a  a  a P*
*                     *
*P       a   a  a  a P*
***********************]]

LAYOUTS.debug = [[
******
*P  P*
*    *
******]]

local STATE_MAP = {
    ['*'] = 'wall',
    ['P'] = 'spawnPoint',
    ['r'] = 'rock',
    ['p'] = 'paper',
    ['s'] = 'scissors',
    ['a'] = 'random',
}

local maps = {}
for key, layout in pairs(LAYOUTS) do
  maps[key] = {
      layout = layout,
      stateMap = STATE_MAP
  }
end

return maps
