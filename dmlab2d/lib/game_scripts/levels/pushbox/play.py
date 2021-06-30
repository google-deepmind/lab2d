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
Use `[]` to switch between episodes.
Use `R` to restart a level.
Use `ESCAPE` to quit.
"""

import argparse
import json
from typing import Mapping

from dmlab2d import ui_renderer

_ACTION_MAP = {'MOVE': ui_renderer.get_direction_pressed}


def _run(config: Mapping[str, str]):
  """Run multiplayer environment, with per player rendering and actions."""

  ui = ui_renderer.Renderer(
      config=config, action_map=_ACTION_MAP, rgb_observation='WORLD.RGB')

  scores = dict()
  for step in ui.run():
    if step.type == ui_renderer.StepType.FIRST:
      print(f'=== Start episode {step.episode} ===')
    else:
      scores[step.episode] = scores.get(step.episode, 0) + step.reward
      print(f'Episode({step.episode}), Score ({scores[step.episode]})')

    if step.type == ui_renderer.StepType.LAST:
      print(f'=== End episode {step.episode} ===')

  print('=== Exiting ===')
  for episode in sorted(scores):
    print(f'Episode({episode}), Score ({scores[episode]})')


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument(
      '--settings', type=json.loads, default={}, help='Settings as JSON string')
  args = parser.parse_args()
  args.settings['levelName'] = 'pushbox'
  _run(args.settings)


if __name__ == '__main__':
  main()
