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

from typing import Collection, Sequence, Tuple, Union

import dm_env
import numpy as np
import dmlab2d.dmlab2d_pybind as dmlab2d_pybind

Lab2d = dmlab2d_pybind.Lab2d
EnvironmentStatus = dmlab2d_pybind.EnvironmentStatus
RUNNING = dmlab2d_pybind.RUNNING
TERMINATED = dmlab2d_pybind.TERMINATED
INTERRUPTED = dmlab2d_pybind.INTERRUPTED
PropertyAttribute = dmlab2d_pybind.PropertyAttribute


class Environment(dm_env.Environment):
  """Environment class for DeepMind Lab2D.

  This environment extends the `dm_env` interface with additional methods.
  For details, see https://github.com/deepmind/dm_env
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
    self._status = RUNNING

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
                              dtype=object)

  def reset(self):
    """See base class."""
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
        elif isinstance(action, str):
          self._act_text[self._act_text_map[spec.name]] = action.encode()
        elif isinstance(action, bytes):
          self._act_text[self._act_text_map[spec.name]] = action
        else:
          raise TypeError(f'Unexpected type {type(action)!r}')

  def step(self, action):
    """See base class."""
    if self._reset_next_step:
      return self.reset()

    self._read_action(self._action_spec, action)
    self._env.act_discrete(self._act_discrete)
    self._env.act_continuous(self._act_continuous)
    self._env.act_text(self._act_text)
    self._status, reward = self._env.advance()
    if self._status != RUNNING:
      self._reset_next_step = True
      return dm_env.termination(reward=reward, observation=self.observation())
    else:
      return dm_env.transition(reward=reward, observation=self.observation())

  def observation(self):
    """Returns the observation resulting from the last step or reset call."""
    return {
        name: np.asarray(
            self._env.observation(name), self._observation_spec[name].dtype)
        for name in self._obs_names
    }

  def observation_spec(self):
    """See base class."""
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
    """See base class."""
    return self._action_spec

  def events(self) -> Sequence[Tuple[str, Sequence[Union[np.ndarray, bytes]]]]:
    """Returns the events generated after last reset or step.

    Returns:
      (name, observations) pairs for all events generated after last step or
        reset.
    """
    return self._env.events()

  def list_property(
      self, key: str) -> Collection[Tuple[str, PropertyAttribute]]:
    """Returns a list of the properties under the specified key name.

    Args:
      key: prefix of property keys to return to search under. The empty string
        can be used as the root.

    Returns:
      (key, attribute) pairs of all properties under input key.

    Raises:
      KeyError: The property does not exist or is not listable.
    """
    return self._env.list_property(key)

  def write_property(self, key: str, value: str) -> None:
    """Writes a property.

    Args:
      key: the name to write to.
      value: the value to write.

    Raises:
      KeyError: The property does not exist or is not readable.
    """
    self._env.write_property(key, value)

  def read_property(self, key: str) -> str:
    """Returns the value of a given property (converted to a string).

    Args:
      key: The property to read.

    Raises:
      KeyError: The property does not exist or is not writable.
    """
    return self._env.read_property(key)
