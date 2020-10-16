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

"""Tests for dmlab2d.python.dmlab2d_env."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

from absl.testing import absltest
from dm_env import test_utils
from dmlab2d.python import dmlab2d
from dmlab2d.python import dmlab2d_env
from dmlab2d.python import runfiles_helper


class Dmlab2dDmEnvTest(test_utils.EnvironmentTestMixin, absltest.TestCase):

  def make_object_under_test(self):
    lab2d = dmlab2d.Lab2d(runfiles_helper.find(),
                          {'levelName': 'examples/level_api'})
    return dmlab2d_env.Environment(lab2d, lab2d.observation_names(), 0)


if __name__ == '__main__':
  absltest.main()
