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

local maps = {}

maps.default = [[
****************************
*                          *
*      PP  P   PP P     P  *
*  AA       AAA       AAA  *
* PAAA     AAAAA     AAAA  *
* PAA       AAA       AAAP *
*  A         A         A   *
*                          *
*       A         A        *
*      AAA       AAA       *
*     AAAAA     AAAAA      *
*      AAA       AAA     P *
* P     A         A        *
*                          *
*  A         A         A P *
*  AA       AAA       AAA  *
*  AAA     AAAAA     AAAA  *
* PAA       AAA       AAA  *
*  A         A         A   *
*                          *
*       A         A      P *
*      AAA       AAA       *
*     AAAAA     AAAAA      *
*  P     P P        P    P *
*                          *
****************************]]

maps.probabilistic_apples = [[
****************************
*                          *
*      PP  P   PP P     P  *
*  aa       aaa       aaa  *
* Paaa     aaaaa     aaaa  *
* Paa       aaa       aaaP *
*  a         a         a   *
*                          *
*       a         a        *
*      aaa       aaa       *
*     aaaaa     aaaaa      *
*      aaa       aaa     P *
* P     a         a        *
*                          *
*  a         a         a P *
*  aa       aaa       aaa  *
*  aaa     aaaaa     aaaa  *
* Paa       aaa       aaa  *
*  a         a         a   *
*                          *
*       a         a      P *
*      aaa       aaa       *
*     aaaaa     aaaaa      *
*  P     P P        P    P *
*                          *
****************************]]

local _DEFAULT_STATE_MAP = {
    ['*'] = 'wall',
    ['P'] = 'spawn.any',
    ['A'] = 'apple',
    ['a'] = 'apple.possible',
}

local layouts = {}
for name, map in pairs(maps) do
  layouts[name] = {
      layout = map,
      stateMap = _DEFAULT_STATE_MAP
  }
end

return layouts
