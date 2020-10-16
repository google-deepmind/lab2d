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

"""Tests for dmlab2d.python.dmlab2d."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from absl.testing import absltest
import numpy as np
from dmlab2d.python import dmlab2d
from dmlab2d.python import runfiles_helper


class Dmlab2DTest(absltest.TestCase):

  def _create_env(self, extra_settings=None):
    settings = extra_settings.copy() if extra_settings else {}
    settings['levelName'] = 'examples/level_api'
    return dmlab2d.Lab2d(runfiles_helper.find(), settings)

  def test_environment_name(self):
    self.assertEqual(self._create_env().name(), 'dmlab2d')

  def test_observation_names(self):
    env = self._create_env()
    self.assertEqual(env.observation_names(),
                     ['VIEW' + str(i) for i in range(1, 6)])

  def test_observation_spec(self):
    env = self._create_env()

    self.assertEqual(
        env.observation_spec('VIEW1'), {
            'dtype': np.dtype('uint8'),
            'shape': (1,)
        })

    self.assertEqual(
        env.observation_spec('VIEW2'), {
            'dtype': np.dtype('double'),
            'shape': (2,)
        })

    self.assertEqual(
        env.observation_spec('VIEW3'), {
            'dtype': np.dtype('int32'),
            'shape': (3,)
        })

    self.assertEqual(
        env.observation_spec('VIEW4'), {
            'dtype': np.dtype('int64'),
            'shape': (4,)
        })

    # Text is stored in objects.
    self.assertEqual(
        env.observation_spec('VIEW5'), {
            'dtype': np.dtype('O'),
            'shape': ()
        })

  def test_action_spec(self):
    env = self._create_env()
    self.assertEqual(env.action_discrete_names(), ['REWARD_ACT'])
    self.assertEqual(
        env.action_discrete_spec('REWARD_ACT'), {
            'min': 0,
            'max': 4
        })
    self.assertEqual(env.action_continuous_names(), ['OBSERVATION_ACT'])
    self.assertEqual(
        env.action_continuous_spec('OBSERVATION_ACT'), {
            'min': -5,
            'max': 5
        })
    self.assertEqual(env.action_text_names(), ['LOG_EVENT'])

  def test_start_environment(self):
    env = self._create_env()
    env.start(episode=0, seed=0)

  def test_events_start(self):
    env = self._create_env()
    env.start(episode=0, seed=0)
    events = env.events()
    self.assertLen(events, 1)
    event_name, observations = events[0]
    self.assertEqual(event_name, 'start')
    self.assertLen(observations, 1)
    np.testing.assert_array_equal(observations[0], [1, 2, 3])

  def test_events_cleared_after_advance_not_read(self):
    env = self._create_env()
    env.start(episode=0, seed=0)
    self.assertLen(env.events(), 1)
    self.assertLen(env.events(), 1)
    env.advance()
    self.assertEmpty(env.events())

  def test_observe(self):
    env = self._create_env()
    env.start(episode=0, seed=0)
    np.testing.assert_array_equal(env.observation('VIEW1'), [1])
    np.testing.assert_array_equal(env.observation('VIEW2'), [1, 2])
    np.testing.assert_array_equal(env.observation('VIEW3'), [1, 2, 3])
    np.testing.assert_array_equal(env.observation('VIEW4'), [1, 2, 3, 4])
    self.assertEqual(env.observation('VIEW5'), b'')

  def test_ten_steps_terminate_environment(self):
    env = self._create_env()
    env.start(episode=0, seed=0)
    for _ in range(9):
      self.assertEqual(env.advance()[0], dmlab2d.RUNNING)
    self.assertEqual(env.advance()[0], dmlab2d.TERMINATED)

  def test_settings_environment(self):
    env = self._create_env({'steps': '5'})
    env.start(episode=0, seed=0)
    for _ in range(4):
      self.assertEqual(env.advance()[0], dmlab2d.RUNNING)
    self.assertEqual(env.advance()[0], dmlab2d.TERMINATED)

  def test_properties_environment(self):
    env = self._create_env({'steps': '5'})
    properties = env.list_property('')
    self.assertLen(properties, 1)
    self.assertEqual(properties[0],
                     ('steps', dmlab2d.PropertyAttribute.READABLE_WRITABLE))
    self.assertEqual(env.read_property('steps'), '5')
    env.write_property('steps', '3')
    self.assertEqual(env.read_property('steps'), '3')
    env.start(episode=0, seed=0)
    for _ in range(2):
      self.assertEqual(env.advance()[0], dmlab2d.RUNNING)
    self.assertEqual(env.advance()[0], dmlab2d.TERMINATED)

  def test_act_discrete(self):
    env = self._create_env({'steps': '5'})
    env.start(episode=0, seed=0)
    env.act_discrete(np.array([2], np.dtype('int32')))
    _, reward = env.advance()
    self.assertEqual(reward, 2)

  def test_act_continuous(self):
    env = self._create_env({'steps': '5'})
    env.start(episode=0, seed=0)
    np.testing.assert_array_equal(env.observation('VIEW3'), [1, 2, 3])
    env.act_continuous([10])
    env.advance()
    np.testing.assert_array_equal(env.observation('VIEW3'), [11, 12, 13])

  def test_act_text(self):
    env = self._create_env({'steps': '5'})
    env.start(episode=0, seed=0)
    view = env.observation('VIEW5')
    self.assertEqual(view, b'')
    env.act_text(['Hello'])
    env.advance()
    view = env.observation('VIEW5')
    self.assertEqual(view, b'Hello')

  def test_invalid_setting(self):
    with self.assertRaises(ValueError):
      self._create_env({'missing': '5'})

  def test_bad_action_spec_name(self):
    env = self._create_env()
    with self.assertRaises(KeyError):
      env.action_discrete_spec('bad_key')
    with self.assertRaises(KeyError):
      env.action_continuous_spec('bad_key')

  def test_bad_observation_spec_name(self):
    env = self._create_env()
    with self.assertRaises(KeyError):
      env.observation_spec('bad_key')

  def test_observe_before_start(self):
    env = self._create_env()
    with self.assertRaises(RuntimeError):
      env.observation('VIEW1')

  def test_act_before_start(self):
    env = self._create_env()
    with self.assertRaises(RuntimeError):
      env.act_discrete([0])

    with self.assertRaises(RuntimeError):
      env.act_continuous([0])

    with self.assertRaises(RuntimeError):
      env.act_text([''])

  def test_act_bad_shape(self):
    env = self._create_env()
    env.start(0, 0)

    with self.assertRaises(ValueError):
      env.act_discrete([0, 1])

    with self.assertRaises(ValueError):
      env.act_continuous([0, 1])

  def test_advane_after_episode_ends(self):
    env = self._create_env({'steps': '2'})
    env.start(0, 0)
    self.assertEqual(env.advance()[0], dmlab2d.RUNNING)
    self.assertEqual(env.advance()[0], dmlab2d.TERMINATED)
    with self.assertRaises(RuntimeError):
      env.advance()

  def test_missing_properties(self):
    env = self._create_env({'steps': '5'})
    with self.assertRaises(KeyError):
      env.list_property('missing')
    with self.assertRaises(KeyError):
      env.read_property('missing')
    with self.assertRaises(KeyError):
      env.write_property('missing', '10')

  def test_invalid_ops_properties(self):
    env = self._create_env({'steps': '5'})
    with self.assertRaises(ValueError):
      env.list_property('steps')
    with self.assertRaises(ValueError):
      env.write_property('steps', 'mouse')

if __name__ == '__main__':
  absltest.main()
