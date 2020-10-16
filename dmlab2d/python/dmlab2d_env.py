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

"""DeepMind Lab2D environment."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import dm_env
import numpy as np
import six
import dmlab2d.python.dmlab2d as dmlab2d


class Environment(dm_env.Environment):
  """Environment class for DeepMind Lab2D.

  This environment uses the `dm_env` interface. For details, see
  https://github.com/deepmind/dm_env
  """

  def __init__(self, env, observation_names, seed=None):
    """DeepMind Lab2D environment.

    Args:
      env: dmlab2d.Lab2d -
      observation_names: List<string>
      seed: int - seed the environment.
    """
    self._env = env
    self._obs_names = observation_names
    self._rng = np.random.RandomState(seed=seed)
    self._next_episode = 0
    self._reset_next_step = True
    self._status = dmlab2d.RUNNING

    action_discrete_names = self._env.action_discrete_names()
    action_continuous_names = self._env.action_continuous_names()
    action_text_names = self._env.action_text_names()
    self._observation_spec = self._make_observation_spec()
    self._action_spec = self._make_action_spec(action_discrete_names,
                                               action_continuous_names,
                                               action_text_names)
    self._act_discrete_map = {
        name: i for i, name in enumerate(action_discrete_names)
    }
    self._act_continuous_map = {
        name: i for i, name in enumerate(action_continuous_names)
    }
    self._act_text_map = {name: i for i, name in enumerate(action_text_names)}
    self._act_discrete = np.zeros(len(action_discrete_names), dtype='int32')
    self._act_continuous = np.zeros(
        len(action_continuous_names), dtype='float64')
    self._act_text = np.array([b'' for _ in range(len(action_text_names))],
                              dtype=np.object)

  def reset(self):
    """Implements dm_env.reset()."""
    self._reset_next_step = False
    self._env.start(self._next_episode, seed=self._rng.randint(0, 2**31))
    self._next_episode += 1
    return dm_env.restart(self.observation())

  def _read_action(self, spec, action):
    if isinstance(spec, list):
      for spec_i, act_i in zip(spec, action):
        self._read_action(spec_i, act_i)
    elif isinstance(spec, dict):
      for spec_key in spec:
        if spec_key in action:
          self._read_action(spec[spec_key], action[spec_key])
    else:
      if spec.dtype == np.dtype('int32'):
        self._act_discrete[self._act_discrete_map[spec.name]] = action
      elif spec.dtype == np.dtype('float64'):
        self._act_continuous[self._act_continuous_map[spec.name]] = action
      elif spec.dtype == np.dtype('S'):
        if isinstance(action, np.ndarray):
          self._act_text[self._act_text_map[spec.name]] = action.tobytes()
        else:
          self._act_text[self._act_text_map[spec.name]] = six.ensure_binary(
              action)

  def step(self, action):
    """Step the environment with an action."""
    if self._reset_next_step:
      return self.reset()

    self._read_action(self._action_spec, action)
    self._env.act_discrete(self._act_discrete)
    self._env.act_continuous(self._act_continuous)
    self._env.act_text(self._act_text)
    self._status, reward = self._env.advance()
    if self._status != dmlab2d.RUNNING:
      self._reset_next_step = True
      return dm_env.termination(reward=reward, observation=self.observation())
    else:
      return dm_env.transition(reward=reward, observation=self.observation())

  def observation(self):
    """Implements dm_env.observation()."""
    return {
        name: np.asarray(
            self._env.observation(name), self._observation_spec[name].dtype)
        for name in self._obs_names
    }

  def observation_spec(self):
    """Implements dm_env.observation_spec()."""
    return self._observation_spec

  def _make_observation_spec(self):
    observations = {}
    for name in self._obs_names:
      spec = self._env.observation_spec(name)
      observations[name] = dm_env.specs.Array(
          shape=spec['shape'], dtype=spec['dtype'], name=name)
    return observations

  def _make_action_spec(self, action_discrete_names, action_continuous_names,
                        action_text_names):
    action_spec = {}
    for name in action_discrete_names:
      spec = self._env.action_discrete_spec(name)
      action_spec[name] = dm_env.specs.BoundedArray(
          dtype=np.dtype('int32'),
          shape=(),
          name=name,
          minimum=spec['min'],
          maximum=spec['max'])

    for name in action_continuous_names:
      spec = self._env.action_continuous_spec(name)
      action_spec[name] = dm_env.specs.BoundedArray(
          dtype=np.dtype('float64'),
          shape=(),
          name=name,
          minimum=spec['min'],
          maximum=spec['max'])

    for name in action_text_names:
      action_spec[name] = dm_env.specs.Array(
          dtype=np.dtype(object), shape=(), name=name)

    return action_spec

  def action_spec(self):
    """Implements dm_env.action_spec()."""
    return self._action_spec

  def __getattr__(self, attr):
    """Delegate calls on this dm_env to its wrapped raw environment.

    Args:
      attr: class attibute.

    Returns:
      Attribute for wrapped environment.
    """
    return getattr(self._env, attr)
