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

"""Helper to find runfiles location."""

from __future__ import absolute_import
from __future__ import division
from __future__ import print_function

import os
import sys


def _runfiles_part(path, sub_directory):
  """Searches runfiles directories in `path` for `sub_directory`.

  Args:
    path: Path to look for sub_directory in.
    sub_directory: Name of subdirectory to search for.

  Returns:
    Returns full path to `sub_directory` or None.
  """
  while path:
    pos = path.rfind('.runfiles')
    if pos < 0:
      pos = path.rfind('/runfiles')
      if pos < 0:
        return None
    path = path[:pos + len('.runfiles')]
    result = os.path.join(path, sub_directory)
    if os.path.isdir(result):
      return result
    path = path[:pos]
  return None


def find():
  """Searches for the folder containing DMLab2D assets."""
  sub_directory = 'org_deepmind_lab2d/dmlab2d'

  return find_directory(sub_directory)[:-len(sub_directory)]


def find_directory(sub_directory):
  """Searches for `sub_directory` heuristically.

  Searches for `sub_directory` folder in possible built-in data dependency
  directories, sys.path, working directory and absolute path.

  Args:
    sub_directory: Name of subdirectory that must exist.

  Returns:
    A path to an existing directory with suffix `sub_directory` or None.
  """
  sub_directory = sub_directory or ''

  # Try using environment variable created when running tests.
  data_directory = os.environ.get('TEST_SRCDIR')
  if data_directory:
    return os.path.join(data_directory, sub_directory)

  # Try using environment variable created by bazel run.
  data_directory = _runfiles_part(
      os.environ.get('RUNFILES_MANIFEST_FILE'), sub_directory)
  if data_directory:
    return data_directory

  # Try using path to current executable.
  data_directory = _runfiles_part(sys.argv[0], sub_directory)
  if data_directory:
    return data_directory

  # Try using path to current file.
  data_directory = _runfiles_part(os.path.abspath(__file__), sub_directory)
  if data_directory:
    return data_directory

  # Try using path to working directory.
  data_directory = _runfiles_part(os.getcwd(), sub_directory)
  if data_directory:
    return data_directory

  # Try using relative path directly.
  data_directory = os.path.join(os.getcwd(), sub_directory)
  if os.path.isdir(data_directory):
    return data_directory

  # Try using search path.
  for path in sys.path:
    data_directory = _runfiles_part(path, sub_directory)
    if data_directory:
      return data_directory
    data_directory = os.path.join(path, sub_directory)
    if os.path.isdir(data_directory):
      return data_directory

  # Try using absolute path.
  if os.path.isdir(sub_directory):
    return sub_directory
  return None
