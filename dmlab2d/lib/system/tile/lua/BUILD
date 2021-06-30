# Description:
#   Tile based rendering.
load("//dmlab2d/lib/testing:lua_testing.bzl", "dmlab2d_lua_test")

licenses(["notice"])

cc_library(
    name = "tile",
    srcs = ["tile.cc"],
    hdrs = ["tile.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":tile_scene",
        ":tile_set",
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:bind",
        "//dmlab2d/lib/lua:table_ref",
    ],
)

cc_library(
    name = "tile_scene",
    srcs = ["tile_scene.cc"],
    hdrs = ["tile_scene.h"],
    deps = [
        ":tile_set",
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:class",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/system/math:math2d",
        "//dmlab2d/lib/system/math/lua:math2d",
        "//dmlab2d/lib/system/tensor:tensor_view",
        "//dmlab2d/lib/system/tensor/lua:tensor",
        "//dmlab2d/lib/system/tile:pixel",
        "//dmlab2d/lib/system/tile:tile_renderer",
        "//dmlab2d/lib/system/tile:tile_set",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "tile_scene_test",
    srcs = ["tile_scene_test.cc"],
    deps = [
        ":tile",
        "//dmlab2d/lib/lua:bind",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:n_results_or_test_util",
        "//dmlab2d/lib/lua:push_script",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:vm",
        "//dmlab2d/lib/lua:vm_test_util",
        "//dmlab2d/lib/system/random/lua:random",
        "//dmlab2d/lib/system/tensor:tensor_view",
        "//dmlab2d/lib/system/tensor/lua:tensor",
        "//dmlab2d/lib/util:default_read_only_file_system",
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
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:class",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:ref",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/system/math:math2d",
        "//dmlab2d/lib/system/math/lua:math2d",
        "//dmlab2d/lib/system/tensor/lua:tensor",
        "//dmlab2d/lib/system/tile:tile_set",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
    ],
)

cc_test(
    name = "tile_set_test",
    srcs = ["tile_set_test.cc"],
    deps = [
        ":tile_set",
        "//dmlab2d/lib/lua:bind",
        "//dmlab2d/lib/lua:n_results_or_test_util",
        "//dmlab2d/lib/lua:push_script",
        "//dmlab2d/lib/lua:vm_test_util",
        "//dmlab2d/lib/system/math:math2d",
        "//dmlab2d/lib/system/math/lua:math2d",
        "//dmlab2d/lib/system/tensor/lua:tensor",
        "//dmlab2d/lib/system/tile:pixel",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

dmlab2d_lua_test(name = "tile_test")
