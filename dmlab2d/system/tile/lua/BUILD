# Description:
#   Tile based rendering.
load("//dmlab2d/testing:lua_testing.bzl", "dmlab2d_lua_test")

licenses(["notice"])

cc_library(
    name = "tile",
    srcs = ["tile.cc"],
    hdrs = ["tile.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":tile_scene",
        ":tile_set",
        "//dmlab2d/lua",
        "//dmlab2d/lua:bind",
        "//dmlab2d/lua:table_ref",
    ],
)

cc_library(
    name = "tile_scene",
    srcs = ["tile_scene.cc"],
    hdrs = ["tile_scene.h"],
    deps = [
        ":tile_set",
        "//dmlab2d/lua",
        "//dmlab2d/lua:class",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/system/math:math2d",
        "//dmlab2d/system/math/lua:math2d",
        "//dmlab2d/system/tensor:tensor_view",
        "//dmlab2d/system/tensor/lua:tensor",
        "//dmlab2d/system/tile:pixel",
        "//dmlab2d/system/tile:tile_renderer",
        "//dmlab2d/system/tile:tile_set",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "tile_scene_test",
    srcs = ["tile_scene_test.cc"],
    deps = [
        ":tile",
        "//dmlab2d/lua:bind",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or_test_util",
        "//dmlab2d/lua:push_script",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:vm",
        "//dmlab2d/lua:vm_test_util",
        "//dmlab2d/system/random/lua:random",
        "//dmlab2d/system/tensor:tensor_view",
        "//dmlab2d/system/tensor/lua:tensor",
        "//dmlab2d/util:default_read_only_file_system",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "tile_set",
    srcs = ["tile_set.cc"],
    hdrs = ["tile_set.h"],
    deps = [
        "//dmlab2d/lua",
        "//dmlab2d/lua:class",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:ref",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/system/math:math2d",
        "//dmlab2d/system/math/lua:math2d",
        "//dmlab2d/system/tensor/lua:tensor",
        "//dmlab2d/system/tile:tile_set",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_test(
    name = "tile_set_test",
    srcs = ["tile_set_test.cc"],
    deps = [
        ":tile_set",
        "//dmlab2d/lua:bind",
        "//dmlab2d/lua:n_results_or_test_util",
        "//dmlab2d/lua:push_script",
        "//dmlab2d/lua:vm_test_util",
        "//dmlab2d/system/math:math2d",
        "//dmlab2d/system/math/lua:math2d",
        "//dmlab2d/system/tensor/lua:tensor",
        "//dmlab2d/system/tile:pixel",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

dmlab2d_lua_test(name = "tile_test")
