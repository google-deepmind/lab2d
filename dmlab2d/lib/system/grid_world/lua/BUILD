load("//dmlab2d/lib/testing:lua_testing.bzl", "dmlab2d_lua_test")

licenses(["notice"])

cc_library(
    name = "lua_world",
    srcs = ["lua_world.cc"],
    hdrs = ["lua_world.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":lua_grid",
        ":lua_grid_view",
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:bind",
        "//dmlab2d/lib/lua:class",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:ref",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/system/grid_world:handles",
        "//dmlab2d/lib/system/grid_world:world",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "lua_world_test",
    srcs = ["lua_world_test.cc"],
    deps = [
        ":lua_grid",
        ":lua_world",
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:n_results_or_test_util",
        "//dmlab2d/lib/lua:push_script",
        "//dmlab2d/lib/lua:vm",
        "//dmlab2d/lib/lua:vm_test_util",
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
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:class",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:ref",
        "//dmlab2d/lib/lua:stack_resetter",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/system/grid_world:grid",
        "//dmlab2d/lib/system/grid_world:handles",
        "//dmlab2d/lib/system/grid_world:text_tools",
        "//dmlab2d/lib/system/grid_world:world",
        "//dmlab2d/lib/system/math:math2d",
        "//dmlab2d/lib/system/math/lua:math2d",
        "//dmlab2d/lib/system/random/lua:random",
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
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:class",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:ref",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/system/grid_world:grid",
        "//dmlab2d/lib/system/grid_world:grid_view",
        "//dmlab2d/lib/system/grid_world:handles",
        "//dmlab2d/lib/system/grid_world:sprite_instance",
        "//dmlab2d/lib/system/grid_world:text_tools",
        "//dmlab2d/lib/system/grid_world:world",
        "//dmlab2d/lib/system/grid_world/collections:fixed_handle_map",
        "//dmlab2d/lib/system/math:math2d",
        "//dmlab2d/lib/system/math/lua:math2d",
        "//dmlab2d/lib/system/tensor:tensor_view",
        "//dmlab2d/lib/system/tensor/lua:tensor",
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
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:n_results_or_test_util",
        "//dmlab2d/lib/lua:push_script",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/lua:vm",
        "//dmlab2d/lib/lua:vm_test_util",
        "//dmlab2d/lib/system/math:math2d",
        "//dmlab2d/lib/system/math/lua:math2d",
        "//dmlab2d/lib/system/tensor/lua:tensor",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "lua_handle",
    hdrs = ["lua_handle.h"],
    deps = [
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:push",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/system/grid_world/collections:handle",
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
