# Copyright 2019 The DMLab2D Authors.
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

"""Random agent for running against DM Lab2D environments."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import argparse
import json

import numpy as np
import pygame

from dmlab2d.python import dmlab2d
from dmlab2d.python import dmlab2d_env
from dmlab2d.python import runfiles_helper


def _make_int32_distribution(random, minimum, maximum):

  def function():
    return random.randint(minimum, maximum + 1)

  return function


def _make_float64_distribution(random, minimum, maximum):

  def function():
    return random.uniform(minimum, maximum)

  return function


class PyGameRandomAgent(object):
  """Random agent works with int32 or float64 bounded actions."""

  def __init__(self, action_spec, observation_name, observation_spec, seed,
               scale):
    """Create a PyGame agent.

    Args:
      action_spec: Environment action spec used to generate random actions.
      observation_name: Name of observation to render each frame.
      observation_spec: Environment observation spec for creating PyGame window.
      seed: Agent seed used for generating random actions.
      scale: Scales screen.
    """
    self._observation_name = observation_name
    random = np.random.RandomState(seed)
    self._actions = []
    self._scores = []
    self._scale = scale
    for name, spec in action_spec.items():
      if spec.dtype == np.dtype('int32'):
        self._actions.append(
            (name, _make_int32_distribution(random, spec.minimum,
                                            spec.maximum)))
      elif spec.dtype == np.dtype('float64'):
        self._actions.append(
            (name, _make_float64_distribution(random, spec.minimum,
                                              spec.maximum)))
      else:
        print("Warning '{}' is not supported".format(spec))
    obs_spec = observation_spec[observation_name]
    self._setup_py_game(obs_spec.shape)

  def _setup_py_game(self, shape):
    pygame.init()
    pygame.display.set_caption('DM Lab2d')
    self._game_display = pygame.display.set_mode(
        (int(shape[1] * self._scale), int(shape[0] * self._scale)))

  def _render_observation(self, observation):
    obs = np.transpose(observation, (1, 0, 2))
    surface = pygame.surfarray.make_surface(obs)
    rect = surface.get_rect()
    surf = pygame.transform.scale(
        surface, (int(rect[2] * self._scale), int(rect[3] * self._scale)))

    self._game_display.blit(surf, dest=(0, 0))
    pygame.display.update()

  def step(self, timestep):
    """Renders timestep and returns random actions according to spec."""
    self._render_observation(timestep.observation[self._observation_name])
    display_score_dirty = False
    if timestep.reward is not None:
      if timestep.reward != 0:
        self._scores[-1] += timestep.reward
        display_score_dirty = True
    else:
      self._scores.append(0)
      display_score_dirty = True

    if display_score_dirty:
      pygame.display.set_caption('%d score' % self._scores[-1])
    return {name: gen() for name, gen in self._actions}

  def print_stats(self):
    print('Scores: ' + ', '.join(str(score) for score in self._scores))


def _create_environment(args):
  """Creates an environment.

  Args:
    args: See `main()` for description of args.

  Returns:
    dmlab2d_env.Environment with one observation.
  """
  args.settings['levelName'] = args.level_name
  lab2d = dmlab2d.Lab2d(runfiles_helper.find(), args.settings)
  return dmlab2d_env.Environment(lab2d, [args.observation], args.env_seed)


def _run(args):
  """Runs a random agent against an environment rendering the results.

  Args:
    args: See `main()` for description of args.
  """
  env = _create_environment(args)
  agent = PyGameRandomAgent(env.action_spec(), args.observation,
                            env.observation_spec(), args.agent_seed, args.scale)
  for _ in range(args.num_episodes):
    timestep = env.reset()
    # Run single episode.
    while True:
      # Query PyGame for early termination.
      if any(event.type == pygame.QUIT for event in pygame.event.get()):
        print('Exit early last score may be truncated:')
        agent.print_stats()
        return
      action = agent.step(timestep)
      timestep = env.step(action)
      if timestep.last():
        # Observe last frame of episode.
        agent.step(timestep)
        break

  # All episodes completed, report per episode.
  agent.print_stats()


def main():
  parser = argparse.ArgumentParser(description=__doc__)
  parser.add_argument(
      '--level_name', type=str, default='clean_up', help='Level name to load')
  parser.add_argument(
      '--observation',
      type=str,
      default='WORLD.RGB',
      help='Observation to render')
  parser.add_argument(
      '--settings', type=json.loads, default={}, help='Settings as JSON string')
  parser.add_argument(
      '--env_seed', type=int, default=0, help='Environment seed')
  parser.add_argument('--agent_seed', type=int, default=0, help='Agent seed')
  parser.add_argument(
      '--num_episodes', type=int, default=1, help='Number of episodes')
  parser.add_argument(
      '--scale', type=float, default=1, help='Scale to render screen')

  args = parser.parse_args()
  _run(args)


if __name__ == '__main__':
  main()
