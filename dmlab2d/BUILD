# Description:
#   Lab2D ("dmlab2d") is a 2D multiplayer grid-world
#   environment for machine learning.

load("//dmlab2d/testing:lua_testing.bzl", "dmlab2d_lua_level_test", "dmlab2d_lua_test")

licenses(["notice"])

exports_files(["LICENSE"])

cc_library(
    name = "dmlab2d",
    srcs = ["dmlab2d.cc"],
    hdrs = ["dmlab2d.h"],
    data = [
        ":game_scripts",
        "//dmlab2d/game_scripts/levels/chase_eat",
        "//dmlab2d/game_scripts/levels/clean_up",
        "//dmlab2d/game_scripts/levels/commons_harvest",
        "//dmlab2d/game_scripts/levels/pushbox",
        "//dmlab2d/game_scripts/levels/running_with_scissors",
    ],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d/env_lua_api",
        "//dmlab2d/lua:bind",
        "//dmlab2d/system/file_system/lua:file_system",
        "//dmlab2d/system/generators/pushbox/lua:pushbox",
        "//dmlab2d/system/grid_world/lua:lua_world",
        "//dmlab2d/system/tile/lua:tile",
        "//third_party/rl_api:env_c_api",
        "//third_party/rl_api:env_c_api_bind",
        "@com_google_absl//absl/memory",
    ],
)

cc_test(
    name = "dmlab2d_test",
    srcs = ["dmlab2d_test.cc"],
    deps = [
        ":dmlab2d",
        "//dmlab2d/util:test_srcdir",
        "//third_party/rl_api:env_c_api",
        "//third_party/rl_api:env_c_api_test_suite",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)

filegroup(
    name = "game_scripts",
    srcs = glob(
        [
            "game_scripts/**/*.lua",
            "game_scripts/**/*.png",
        ],
        exclude = [
            "game_scripts/**/*_test_data/*",
            "game_scripts/**/*_test.lua",
        ],
    ),
    visibility = ["//dmlab2d/lua_interp:__pkg__"],
)

TEST_SCRIPTS = [test[:-len(".lua")] for test in glob(["**/*_test.lua"])]

test_suite(
    name = "lua_tests",
    tests = TEST_SCRIPTS,
)

[
    dmlab2d_lua_test(name = test_script)
    for test_script in TEST_SCRIPTS
]

# Find all directories containing file 'init.lua'.
LEVEL_DIRS = [p[:-len("/init.lua")] for p in glob(["game_scripts/levels/**/init.lua"])]

# Find single levels not in subdirectorys of LEVEL_DIRS
SINGLE_LEVEL_SCRIPTS = [test[:-len(".lua")] for test in glob(
    ["game_scripts/levels/**/*.lua"],
    exclude = ["**/*_test.lua"] + [p + "/**/*.lua" for p in LEVEL_DIRS],
)]

LEVEL_SCRIPTS = SINGLE_LEVEL_SCRIPTS + [p + "/init" for p in LEVEL_DIRS]

test_suite(
    name = "lua_level_tests",
    tests = LEVEL_SCRIPTS,
)

[
    dmlab2d_lua_level_test(name = level_script)
    for level_script in LEVEL_SCRIPTS
]
