load("//dmlab2d/testing:lua_testing.bzl", "dmlab2d_lua_test")

licenses(["notice"])

cc_library(
    name = "lua_world",
    srcs = ["lua_world.cc"],
    hdrs = ["lua_world.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":lua_grid",
        ":lua_grid_view",
        "//dmlab2d/lua",
        "//dmlab2d/lua:bind",
        "//dmlab2d/lua:class",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:ref",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/system/grid_world:handles",
        "//dmlab2d/system/grid_world:world",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "lua_world_test",
    srcs = ["lua_world_test.cc"],
    deps = [
        ":lua_grid",
        ":lua_world",
        "//dmlab2d/lua",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or_test_util",
        "//dmlab2d/lua:push_script",
        "//dmlab2d/lua:vm",
        "//dmlab2d/lua:vm_test_util",
        "@com_google_googletest//:gtest_main",
    ],
)

# Tested in lua_tests/grid_test.lua
cc_library(
    name = "lua_grid",
    srcs = ["lua_grid.cc"],
    hdrs = ["lua_grid.h"],
    deps = [
        ":lua_handle",
        "//dmlab2d/lua",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:class",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:ref",
        "//dmlab2d/lua:stack_resetter",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/system/grid_world:grid",
        "//dmlab2d/system/grid_world:handles",
        "//dmlab2d/system/grid_world:text_tools",
        "//dmlab2d/system/grid_world:world",
        "//dmlab2d/system/math:math2d",
        "//dmlab2d/system/math/lua:math2d",
        "//dmlab2d/system/random/lua:random",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
    ],
)

cc_library(
    name = "lua_grid_view",
    srcs = ["lua_grid_view.cc"],
    hdrs = ["lua_grid_view.h"],
    deps = [
        ":lua_grid",
        ":lua_handle",
        "//dmlab2d/lua",
        "//dmlab2d/lua:class",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:ref",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/system/grid_world:grid",
        "//dmlab2d/system/grid_world:grid_view",
        "//dmlab2d/system/grid_world:handles",
        "//dmlab2d/system/grid_world:sprite_instance",
        "//dmlab2d/system/grid_world:text_tools",
        "//dmlab2d/system/grid_world:world",
        "//dmlab2d/system/grid_world/collections:fixed_handle_map",
        "//dmlab2d/system/math:math2d",
        "//dmlab2d/system/math/lua:math2d",
        "//dmlab2d/system/tensor:tensor_view",
        "//dmlab2d/system/tensor/lua:tensor",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_test(
    name = "lua_grid_view_test",
    srcs = ["lua_grid_view_test.cc"],
    deps = [
        ":lua_grid_view",
        ":lua_world",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or_test_util",
        "//dmlab2d/lua:push_script",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/lua:vm",
        "//dmlab2d/lua:vm_test_util",
        "//dmlab2d/system/math:math2d",
        "//dmlab2d/system/math/lua:math2d",
        "//dmlab2d/system/tensor/lua:tensor",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "lua_handle",
    hdrs = ["lua_handle.h"],
    deps = [
        "//dmlab2d/lua",
        "//dmlab2d/lua:push",
        "//dmlab2d/lua:read",
        "//dmlab2d/system/grid_world/collections:handle",
    ],
)

dmlab2d_lua_test(
    name = "world_lua_test",
    main = "world_test.lua",
)

dmlab2d_lua_test(
    name = "grid_lua_test",
    main = "grid_test.lua",
)
