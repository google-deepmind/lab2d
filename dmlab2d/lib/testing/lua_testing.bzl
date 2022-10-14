"""Build rules for Lua tests using DMLab 2D's built-in libraries.

Example 1 No suplementary data required:

Content: //some/path/to/test/BUILD
//some/path/to/test/file_test.lua

dmlab2d_lua_test(name = "file_test")

Example 2 With 2 data files:

//some/path/to/test/file_test.lua
//some/path/to/test/file_test_data/data1.txt
//some/path/to/test/file_test_data/data2.txt

Content: //some/path/to/BUILD

load("//dmlab2d/lib/testing/lua_testing.bzl", "dmlab2d_lua_test")

dmlab2d_lua_test(name = "file_test")

Content: //some/path/to/test/file_test.lua

local asserts = require 'testing.asserts'
local test_runner = require 'testing.test_runner'
local path = require 'common.path'

local TEST_DIR = test_runner.testDir(...)
local DATA1_PATH = paths.join(TEST_DIR, "data1.txt")
local DATA2_PATH = paths.join(TEST_DIR, "data2.txt")

local tests = {}

function tests:dataFilesExists()
  assert(paths.fileExists(DATA1_PATH), 'data1.txt does not exist?')
  assert(paths.fileExists(DATA2_PATH), 'data2.txt does not exist?')
end

return test_runner.run(tests)
"""

def dmlab2d_lua_test(name, main = None, root = None, data = None, **kwargs):
    """Creates rule that will run tests including DM Lab2D built-in libraries.

    Args:
      name: Name of test.
      main: Optional relative path of test file including '.lua' suffix.
      root: Optional absoloute path of this rule used to find relative data.
      data: Optional data to include.
      **kwargs: Additional arguments to pass on to cc_test .
    """
    extra_data = [] if data == None else data
    root = "" + native.package_name() if root == None else root
    main = name + ".lua" if main == None else main
    level_script = main[:-len(".lua")]
    size = kwargs.pop("size", "small")
    native.cc_test(
        name = name,
        args = [level_script, root],
        data = [main] + native.glob([level_script + "_data/**/*"]) + extra_data,
        deps = ["//dmlab2d/lib/testing:lua_unit_test_lib"],
        size = size,
        **kwargs
    )

def dmlab2d_lua_level_test(name, main = None, root = None, data = None):
    """Creates rule that will run environment specification test.

    Args:
      name: Name of level to test.
      main: Optional relative path of level file including '.lua' suffix.
      root: Optional absoloute path of this rule used to find relative data.
      data: Optional data to include.
    """
    extra_data = [] if data == None else data
    root = "" + native.package_name() if root == None else root
    main = name + ".lua" if main == None else main
    level_script = main[:-len(".lua")]
    native.cc_test(
        name = name,
        args = [level_script, root],
        data = [main] + extra_data,
        deps = ["//dmlab2d/lib/testing:lua_level_test_lib"],
    )
