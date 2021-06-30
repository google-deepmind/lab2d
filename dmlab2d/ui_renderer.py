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
"""Human testing for DMLab2d levels.

Use `[/]` to switch between players.
Use `TAB, SHIFT-TAB` to switch between levels.
Use `R` to restart level.
"""
import enum
from typing import Callable, Generator, List, Mapping, Optional
import dataclasses
import numpy as np
import pygame

import dmlab2d
from dmlab2d import runfiles_helper

MAX_SCREEN_WIDTH = 960
MAX_SCREEN_HEIGHT = 640


def get_direction_pressed() -> int:
  """Gets direction pressed."""
  key_pressed = pygame.key.get_pressed()
  if key_pressed[pygame.K_UP] or key_pressed[pygame.K_w]:
    return 1
  if key_pressed[pygame.K_RIGHT] or key_pressed[pygame.K_d]:
    return 2
  if key_pressed[pygame.K_DOWN] or key_pressed[pygame.K_s]:
    return 3
  if key_pressed[pygame.K_LEFT] or key_pressed[pygame.K_a]:
    return 4
  return 0


def get_turn_pressed() -> int:
  """Calculates turn amount."""
  key_pressed = pygame.key.get_pressed()
  if key_pressed[pygame.K_DELETE] or key_pressed[pygame.K_q]:
    return -1
  if key_pressed[pygame.K_PAGEDOWN] or key_pressed[pygame.K_e]:
    return 1
  return 0


def get_left_control_pressed() -> int:
  return 1 if pygame.key.get_pressed()[pygame.K_LCTRL] else 0


def get_space_key_pressed() -> int:
  return 1 if pygame.key.get_pressed()[pygame.K_SPACE] else 0


class StepType(enum.Enum):
  FIRST = 0
  MID = 1
  LAST = 2


@dataclasses.dataclass
class Step:
  env: object
  reward: Optional[float]
  type: StepType
  player: int
  episode: int


class Renderer:
  """Creates a pygame window for playing an environment."""

  def __init__(self,
               config: Mapping[str, str],
               action_map: Mapping[str, Callable[[], int]],
               rgb_observation: str = 'RGB',
               player_prefixes: Optional[List[str]] = None,
               frames_per_second: Optional[int] = None):
    env = dmlab2d.Lab2d(runfiles_helper.find(), config)
    self._player = 0
    self._env = env
    self._player_prefixes = player_prefixes if player_prefixes else ['']
    self._rgb_observation = rgb_observation
    self._action_map = action_map
    self._frames_per_second = frames_per_second
    self._action_names = env.action_discrete_names()
    self._actions = np.zeros([len(self._action_names)], dtype=np.intc)
    self._observation_names = set(self._env.observation_names())

  def run(self) -> Generator[Step, None, None]:
    """Run the environment."""
    self._init_pygame()
    episode = 0
    while episode is not None:
      episode = yield from self._play_episode(episode)

  def _play_episode(self, episode) -> Generator[Step, None, Optional[int]]:
    """Plays the environment for a single episode."""
    self._env.start(episode, episode)
    yield Step(
        env=self._env,
        reward=None,
        type=StepType.FIRST,
        player=self._player,
        episode=episode)
    num_players = len(self._player_prefixes)
    while True:
      key_pressed = False
      for event in pygame.event.get():
        if event.type == pygame.QUIT:
          return None

        if event.type == pygame.KEYDOWN:
          if event.key == pygame.K_TAB:
            if event.mod & pygame.KMOD_SHIFT:
              self._player = (self._player + num_players - 1) % num_players
            else:
              self._player = (self._player + 1) % num_players
          elif event.key == pygame.K_ESCAPE:
            return None
          elif event.key == pygame.K_r:
            return episode
          elif event.key == pygame.K_LEFTBRACKET:
            return episode - 1
          elif event.key == pygame.K_RIGHTBRACKET:
            return episode + 1
          else:
            key_pressed = True

      self._update_actions()
      if self._frames_per_second or key_pressed:
        self._env.act_discrete(self._actions)
        status, reward = self._env.advance()

        if status != dmlab2d.RUNNING:
          yield Step(
              env=self._env,
              reward=reward,
              type=StepType.LAST,
              player=self._player,
              episode=episode)
          return episode + 1

        yield Step(
            env=self._env,
            reward=reward,
            type=StepType.MID,
            player=self._player,
            episode=episode)

      self._update_screen()
      if self._frames_per_second:
        self._clock.tick(self._frames_per_second)
      else:
        self._clock.tick(60)
    return None

  def _init_pygame(self):
    """Constructs pygame window based on first player's observation spec."""
    pygame.init()
    pygame.display.set_caption(self._env.name())
    scale = 1
    prefix = self._player_prefixes[self._player]

    if prefix + self._rgb_observation in self._observation_names:
      obs_spec = self._env.observation_spec(prefix + self._rgb_observation)
    elif self._rgb_observation in self._observation_names:
      obs_spec = self._env.observation_spec(self._rgb_observation)
    else:
      raise ValueError(f'Cannot find observation {self._rgb_observation}')
    observation_shape = obs_spec['shape']
    observation_height = observation_shape[0]
    observation_width = observation_shape[1]
    scale = min(MAX_SCREEN_HEIGHT // observation_height,
                MAX_SCREEN_WIDTH // observation_width)
    self._game_display = pygame.display.set_mode(
        (observation_width * scale, observation_height * scale))
    self._scale = scale
    self._clock = pygame.time.Clock()

  def _update_actions(self):
    """Reads action map and applies to current player."""
    prefix = self._player_prefixes[self._player]
    for i, name in enumerate(self._action_names):
      if not name.startswith(prefix):
        continue
      action = name[len(prefix):]
      if action in self._action_map:
        self._actions[i] = self._action_map[action]()

  def _player_observation(self):
    """Return observation of current player."""
    prefix = self._player_prefixes[self._player]
    if prefix + self._rgb_observation in self._observation_names:
      return self._env.observation(prefix + self._rgb_observation)
    elif self._rgb_observation in self._observation_names:
      return self._env.observation(self._rgb_observation)
    raise ValueError(
        f'Cannot find observation {prefix + self._rgb_observation}')

  def _update_screen(self):
    # PyGame is column major!
    obs = np.transpose(self._player_observation(), (1, 0, 2))
    surface = pygame.surfarray.make_surface(obs)
    rect = surface.get_rect()
    surf = pygame.transform.scale(
        surface, (rect[2] * self._scale, rect[3] * self._scale))
    self._game_display.blit(surf, dest=(0, 0))
    pygame.display.update()
