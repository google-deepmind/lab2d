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
"""Tests for dmlab2d.system.generators.pushbox.python.pushbox."""

from absl.testing import absltest
from dmlab2d.system.generators.pushbox.python import pushbox


class PythonPushboxTest(absltest.TestCase):

  def test_generate_level(self):
    level = pushbox.Generate(seed=10, height=11, width=14, num_boxes=5)
    # Test goal count
    self.assertEqual(level.count('X') + level.count('&'), 5)
    # Test box count
    self.assertEqual(level.count('B') + level.count('&'), 5)
    # Test spawn point count
    self.assertEqual(level.count('P'), 1)

    self.assertLen(level.split('\n'), 11)
    self.assertEqual(level.find('\n'), 14)

  def test_bad_inputs(self):
    with self.assertRaises(ValueError):
      pushbox.Generate(seed=10, height=5, width=5, num_boxes=36)


if __name__ == '__main__':
  absltest.main()
