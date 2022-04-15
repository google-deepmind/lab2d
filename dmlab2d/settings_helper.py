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


"""Function for flattening dictionary settings."""

import numbers
from typing import Mapping, Sequence


def _flatten_args(pairs_in, args_out, prefix, visited_stack):
  """Helper function for flatten_args. See `flatten_args` below for details."""
  for key, v in pairs_in:
    if not isinstance(key, str):
      raise ValueError('Keys must be strings. %r' % key)
    flat_key = prefix + '.' + key if prefix else key
    if v is None:
      args_out[flat_key] = 'none'
    elif isinstance(v, str):
      args_out[flat_key] = v
    elif isinstance(v, bool):
      args_out[flat_key] = 'true' if v else 'false'
    elif isinstance(v, numbers.Number):
      args_out[flat_key] = str(v)
    elif isinstance(v, Mapping):
      if not any(v is entry for entry in visited_stack):
        _flatten_args(v.items(), args_out, flat_key, visited_stack + [v])
    elif isinstance(v, Sequence):
      if not any(v is entry for entry in visited_stack):
        _flatten_args(((str(i + 1), vv) for i, vv in enumerate(v)), args_out,
                      flat_key, visited_stack + [v])

    else:
      raise ValueError('Value for \'{}\' cannot be type: \'{}\''.format(
          flat_key, str(type(v))))


def flatten_args(args_in):
  """Converts a dictionary of dictionarys and lists into a flat table.

  Args:
    args_in: dictionary containing a hierachy of dictionaries and lists. Leaf
      values can be strings, bools, numbers..

  Returns:
    A flat dictionary with keys separated by '.' and string values.
  """
  args_out = {}
  _flatten_args(args_in.items(), args_out, None, [args_in])
  return args_out
