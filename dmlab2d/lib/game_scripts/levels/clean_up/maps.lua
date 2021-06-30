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

local _DEFAULT_LAYOUT = [[
******************************
*HFFFHFFHFHFHFHFHFHFHHFHFFFHF*
*HFHFHFFHFHFHFHFHFHFHHFHFFFHF*
*HFFHFFHHFHFHFHFHFHFHHFHFFFHF*
*HFHFHFFHFHFHFHFHFHFHHFHFFFHF*
*HFFFFFFHFHFHFHFHFHFHHFHFFFHF*
*               HFHHHHHH     *
*   P    P          SSS      *
*     P     P   P   SS   P   *
*             P   PPSS       *
*   P    P          SS    P  *
*               P   SS P     *
*     P           P SS       *
*           P       SS  P    *
*  P             P PSS       *
* B B B B B B B B B SSB B B B*
*BBBBBBBBBBBBBBBBBBBBBBBBBBBB*
*BBBBBBBBBBBBBBBBBBBBBBBBBBBB*
*BBBBBBBBBBBBBBBBBBBBBBBBBBBB*
*BBBBBBBBBBBBBBBBBBBBBBBBBBBB*
******************************
]]

local _DEFAULT_STATE_MAP = {
    ['*'] = 'wall',
    ['P'] = 'spawnPoint',
    ['A'] = 'apple',
    ['B'] = 'apple.wait',
    ['S'] = 'stream',
    ['H'] = 'river.water',
    ['F'] = 'river.mud',
}

return {
    default = {
        layout = _DEFAULT_LAYOUT,
        stateMap = _DEFAULT_STATE_MAP,
    }
}
