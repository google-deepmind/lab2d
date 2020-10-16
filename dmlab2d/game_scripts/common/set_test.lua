--[[ Copyright (C) 2020 The DMLab2D Authors.

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

     https://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
]]

local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local args = require 'common.args'
local set = require 'common.set'

local Set = set.Set

local tests = {}

function tests.createSetFromList()
  local newSet = Set{'foo', 'bar', 'foo'}
  asserts.EQ(set.calculateNumberOfElements(newSet), 2)
  asserts.EQ(newSet['foo'], true)
  asserts.EQ(newSet['bar'], true)
end

function tests.createSetFromKeys()
  local newSet = set.SetFromKeys{foo = 1, bar = 2, baz = 3}
  asserts.EQ(set.calculateNumberOfElements(newSet), 3)
  asserts.EQ(newSet['foo'], true)
  asserts.EQ(newSet['bar'], true)
  asserts.EQ(newSet['baz'], true)
end

function tests.isEmpty()
  asserts.EQ(set.isEmpty(Set{}), true)
  asserts.EQ(set.isEmpty(Set{'bar'}), false)
  asserts.EQ(set.isEmpty(Set{false}), false)
  asserts.EQ(set.isEmpty(Set{true}), false)
end

function tests.isSame()
  asserts.EQ(set.isSame(Set{}, Set{}), true)
  asserts.EQ(set.isSame(Set{}, Set{'foo'}), false)
  asserts.EQ(set.isSame(Set{'bar'}, Set{}), false)
  asserts.EQ(set.isSame(Set{'bar'}, Set{'foo'}), false)
  asserts.EQ(set.isSame(Set{'foo', 'bar'}, Set{'foo', 'bar'}), true)
  asserts.EQ(set.isSame(Set{'foo', 'bar'}, Set{'bar', 'foo'}), true)

  -- Check operator works.
  asserts.EQ(Set{'foo', 'bar'} == Set{'bar', 'foo'}, true)
  asserts.EQ(Set{'foo', 'bar'} == Set{'bar', 'baz'}, false)
  asserts.EQ(Set{'foo', 'bar'} ~= Set{'bar', 'foo'}, false)
  asserts.EQ(Set{'foo', 'bar'} ~= Set{'bar', 'baz'}, true)
end

function tests.toSortedListShouldReturnDistinctKeys()
  local list = set.toSortedList(Set{'foo', 'bar', 'foo'})
  asserts.EQ(#list, 2)
  asserts.tablesEQ(list, {'bar', 'foo'})
end

function tests.intersectShouldReturnNewSetOfCommonItems()
  local lhsElements = {'foo', 'bar', 'baz'}
  local rhsElements = {'bar', 'baz', 'fub'}
  local lhs = Set(lhsElements)
  local rhs = Set(rhsElements)

  local intersection = set.intersect(lhs, rhs)
  asserts.EQ(set.calculateNumberOfElements(intersection), 2)
  asserts.EQ(intersection['bar'], true)
  asserts.EQ(intersection['baz'], true)

  -- Check inputs were not changed
  assert(set.isSame(lhs, Set(lhsElements)))
  assert(set.isSame(rhs, Set(rhsElements)))
end

function tests.differenceShouldReturnNewSetWithFirstMinusSecond()
  local lhsElements = {'foo', 'bar', 'baz'}
  local rhsElements = {'bar', 'baz', 'fub'}
  local lhs = Set(lhsElements)
  local rhs = Set(rhsElements)

  local difference = set.difference(lhs, rhs)
  asserts.EQ(set.calculateNumberOfElements(difference), 1)
  asserts.EQ(difference['foo'], true)

  -- Check inputs were not changed
  assert(set.isSame(lhs, Set(lhsElements)))
  assert(set.isSame(rhs, Set(rhsElements)))

  -- Check operator works.
  assert(set.isSame(lhs - rhs, difference))
end

function tests.unionShouldReturnFirstPlusSecond()
  local lhsElements = {'foo', 'bar', 'baz'}
  local rhsElements = {'bar', 'baz', 'fub'}
  local lhs = Set(lhsElements)
  local rhs = Set(rhsElements)

  local union = set.union(lhs, rhs)
  asserts.EQ(set.calculateNumberOfElements(union), 4)
  asserts.EQ(union['foo'], true)
  asserts.EQ(union['bar'], true)
  asserts.EQ(union['baz'], true)
  asserts.EQ(union['fub'], true)

  -- Check inputs were not changed
  assert(set.isSame(lhs, Set(lhsElements)))
  assert(set.isSame(rhs, Set(rhsElements)))

  -- Check operator works.
  assert(set.isSame(lhs + rhs, union))
end

function tests.insertShouldAddNewValuesFromList()
  local newSet = Set{'foo', 'bar', 'foo'}
  asserts.EQ(set.calculateNumberOfElements(newSet), 2)
  asserts.EQ(newSet['foo'], true)
  asserts.EQ(newSet['bar'], true)

  set.insert(newSet, {'bar', 'baz'})
  asserts.EQ(set.calculateNumberOfElements(newSet), 3)
  asserts.EQ(newSet['foo'], true)
  asserts.EQ(newSet['bar'], true)
  asserts.EQ(newSet['baz'], true)
end

function tests.isDisjoint()
  asserts.EQ(set.isDisjoint(Set{'A'}, Set{}), true)
  asserts.EQ(set.isDisjoint(Set{'A'}, Set{'B'}), true)
  asserts.EQ(set.isDisjoint(Set{}, Set{'B'}), true)
  asserts.EQ(set.isDisjoint(Set{}, Set{}), true)
  asserts.EQ(set.isDisjoint(Set{1, 2, 3}, Set{4, 5, 6}), true)

  asserts.EQ(set.isDisjoint(Set{'A'}, Set{'A'}), false)
  asserts.EQ(set.isDisjoint(Set{'A', 'B'}, Set{'B'}), false)
  asserts.EQ(set.isDisjoint(Set{'A'}, Set{'A', 'B'}), false)
  asserts.EQ(set.isDisjoint(Set{1, 2, 3}, Set{2, 4, 6}), false)
end

function tests.isSubset()
  asserts.EQ(set.isSubsetOf(Set{'A', 'B'}, Set{'A', 'B'}), true)
  asserts.EQ(set.isSubsetOf(Set{'B'}, Set{'A', 'B'}), true)
  asserts.EQ(set.isSubsetOf(Set{'C'}, Set{'A', 'B'}), false)
  asserts.EQ(set.isSubsetOf(Set{}, Set{'A', 'B'}), true)
  asserts.EQ(Set{'A', 'B'} >= Set{'A', 'B'}, true)
  asserts.EQ(Set{'A', 'B'} >= Set{'B'}, true)
  asserts.EQ(Set{'A', 'B'} >= Set{'C'}, false)
  asserts.EQ(Set{'A', 'B'} >= Set{}, true)
end

function tests.isProperSubset()
  asserts.EQ(Set{'A', 'B'} > Set{'A', 'B'}, false)
  asserts.EQ(Set{'A', 'B'} > Set{'B'}, true)
  asserts.EQ(Set{'A', 'B'} > Set{'C'}, false)
  asserts.EQ(Set{'A', 'B'} > Set{}, true)
end

function tests.isSet()
  asserts.EQ(set.isSet(Set{}), true)
  asserts.EQ(set.isSet({}), false)
end

function tests.toString()
  asserts.EQ(tostring(Set{}), 'Set{}')
  asserts.EQ(tostring(Set{1}), 'Set{1}')
  asserts.that(tostring(Set{'A', 'B'}),
               args.oneOf('Set{"A", "B"}', 'Set{"B", "A"}'))
  asserts.that(tostring(Set{1, 2}),
               args.oneOf('Set{1, 2}', 'Set{2, 1}'))
  asserts.that(tostring(Set{true, false}),
               args.oneOf('Set{true, false}', 'Set{false, true}'))
end

return test_runner.run(tests)
