# Copyright 2020 The DMLab2D Authors.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS-IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
"""A simple human player for testing the `clean_up` level.

Use `WASD` keys to move the character around.
Use `Q and E` to turn the character.
Use `SPACE` to fire clean.
Use `LEFT_CTRL` to fire fine.
Use `TAB` to switch between players.
Use `[]` to switch between levels.
Use `R` to restart a level.
Use `ESCAPE` to quit.
"""

import argparse
import collections
import json
from typing import Mapping

from dmlab2d.python import ui_renderer

_ACTION_MAP = {
    'move': ui_renderer.get_direction_pressed,
    'turn': ui_renderer.get_turn_pressed,
    'fireClean': ui_renderer.get_space_key_pressed,
    'fireFine': ui_renderer.get_left_control_pressed
}

_FRAMES_PER_SECOND = 8


def _run(rgb_observation: str, config: Mapping[str, str]):
  """Run multiplayer environment, with per player rendering and actions."""
  player_count = int(config.get('numPlayers', '1'))

  score = collections.defaultdict(float)
  total_contrib = collections.defaultdict(float)

  prefixes = [str(i + 1) + '.' for i in range(player_count)]

  ui = ui_renderer.Renderer(
      config=config,
      action_map=_ACTION_MAP,
      rgb_observation=rgb_observation,
      player_prefixes=[str(i + 1) + '.' for i in range(player_count)],
      frames_per_second=_FRAMES_PER_SECOND)

  def player_printer(idx: int):
    print(f'Player({idx}) contrib({total_contrib[idx]}) score({score[idx]})')

  for step in ui.run():
    if step.type == ui_renderer.StepType.FIRST:
      print(f'=== Start episode {step.episode} ===')
    print_player = False
    for idx, prefix in enumerate(prefixes):
      reward = step.env.observation(prefix + 'REWARD')
      score[idx] += reward
      contrib = step.env.observation(prefix + 'CONTRIB')
      total_contrib[idx] += contrib

      if step.player == idx and (reward != 0 or contrib != 0):
        print_player = True

    if print_player:
      player_printer(step.player)

    if step.type == ui_renderer.StepType.LAST:
      print(f'=== End episode {step.episode} ===')
      for idx in range(player_count):
        player_printer(idx)
      print('======')

  print('=== Exiting ===')
  for idx in range(player_count):
    player_printer(idx)


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument(
      '--observation', type=str, default='RGB', help='Observation to render')
  parser.add_argument(
      '--settings', type=json.loads, default={}, help='Settings as JSON string')
  parser.add_argument(
      '--players', type=int, default=4, help='Number of players.')

  args = parser.parse_args()
  if 'levelName' not in args.settings:
    args.settings['levelName'] = 'clean_up'
  if 'numPlayers' not in args.settings:
    args.settings['numPlayers'] = args.players
  for k in args.settings:
    args.settings[k] = str(args.settings[k])
  _run(args.observation, args.settings)


if __name__ == '__main__':
  main()
