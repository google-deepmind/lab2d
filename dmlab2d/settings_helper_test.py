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
"""Tests for dmlab2d.settings_helper."""

from absl.testing import absltest
from dmlab2d import settings_helper


class SettingsHelperTest(absltest.TestCase):

  def test_flatten_real(self):
    config = {}
    config['levelName'] = 'Name'
    config['levelDirectory'] = 'Dir'
    config['simulation'] = {'positions': ('cat', 1, 2)}
    self.assertEqual(
        settings_helper.flatten_args(config), {
            'levelName': 'Name',
            'levelDirectory': 'Dir',
            'simulation.positions.1': 'cat',
            'simulation.positions.2': '1',
            'simulation.positions.3': '2',
        })

  def test_flatten_args_self_ref(self):
    a = {'key': 1}
    b = {'key': 2}
    b['a'] = a
    a['b'] = b
    self.assertEqual(
        settings_helper.flatten_args(a), {
            'key': '1',
            'b.key': '2',
        })
    self.assertEqual(
        settings_helper.flatten_args(b), {
            'key': '2',
            'a.key': '1',
        })

  def test_flatten_args_ref_same(self):
    b = {'key': 2}
    a = {'b1': b, 'b2': b}
    self.assertEqual(
        settings_helper.flatten_args(a), {
            'b1.key': '2',
            'b2.key': '2',
        })
    self.assertEqual(settings_helper.flatten_args(b), {'key': '2'})

  def test_flatten_args(self):
    self.assertEqual(settings_helper.flatten_args({'key': 10}), {'key': '10'})

  def test_flatten_args_tree(self):
    args = {
        'level_name': {
            'rewards': {
                'flag': 1.0
            }
        },
        'team_rewards': {
            'flag': 0.0
        },
        'teams': [1, 5],
        'flags':
            True,
        'foo':
            False,
        'bar':
            None,
        'nested': [
            {
                'a': 3,
                'b': 5
            },
            {
                'c': tuple(range(12))
            },  # Verify sorting is correct.
        ]
    }
    flat_args = settings_helper.flatten_args(args)

    expected_args = {
        'level_name.rewards.flag': '1.0',
        'team_rewards.flag': '0.0',
        'teams.1': '1',
        'teams.2': '5',
        'flags': 'true',
        'foo': 'false',
        'bar': 'none',
        'nested.1.a': '3',
        'nested.1.b': '5',
        'nested.2.c.1': '0',
        'nested.2.c.2': '1',
        'nested.2.c.3': '2',
        'nested.2.c.4': '3',
        'nested.2.c.5': '4',
        'nested.2.c.6': '5',
        'nested.2.c.7': '6',
        'nested.2.c.8': '7',
        'nested.2.c.9': '8',
        'nested.2.c.10': '9',
        'nested.2.c.11': '10',
        'nested.2.c.12': '11',
    }
    self.assertDictEqual(flat_args, expected_args)


if __name__ == '__main__':
  absltest.main()
