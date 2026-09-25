load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

licenses(["notice"])

cc_library(
    name = "world",
    srcs = ["world.cc"],
    hdrs = ["world.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":handles",
        "//dmlab2d/lib/system/grid_world/collections:fixed_handle_map",
        "//dmlab2d/lib/system/grid_world/collections:handle_names",
        "@abseil-cpp//absl/container:btree",
        "@abseil-cpp//absl/strings",
    ],
)

cc_test(
    name = "world_test",
    srcs = ["world_test.cc"],
    deps = [
        ":handles",
        ":world",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "grid_shape",
    hdrs = ["grid_shape.h"],
    deps = [
        ":handles",
        "//dmlab2d/lib/system/math:math2d",
        "@abseil-cpp//absl/log",
    ],
)

cc_test(
    name = "grid_shape_test",
    srcs = ["grid_shape_test.cc"],
    deps = [
        ":grid_shape",
        "//dmlab2d/lib/system/math:math2d",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "handles",
    hdrs = ["handles.h"],
    visibility = ["//visibility:public"],
    deps = ["//dmlab2d/lib/system/grid_world/collections:handle"],
)

cc_library(
    name = "grid",
    srcs = ["grid.cc"],
    hdrs = ["grid.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":grid_shape",
        ":grid_view",
        ":handles",
        ":sprite_instance",
        ":world",
        "//dmlab2d/lib/system/grid_world/collections:fixed_handle_map",
        "//dmlab2d/lib/system/grid_world/collections:object_pool",
        "//dmlab2d/lib/system/grid_world/collections:shuffled_membership",
        "//dmlab2d/lib/system/math:math2d",
        "//dmlab2d/lib/system/math:math2d_algorithms",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/types:any",
        "@abseil-cpp//absl/types:optional",
        "@abseil-cpp//absl/types:span",
    ],
)

cc_test(
    name = "grid_test",
    size = "small",
    srcs = ["grid_test.cc"],
    deps = [
        ":grid",
        ":grid_shape",
        ":grid_view",
        ":grid_window",
        ":handles",
        ":sprite_instance",
        ":text_tools",
        ":world",
        "//dmlab2d/lib/system/grid_world/collections:fixed_handle_map",
        "//dmlab2d/lib/system/math:math2d",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/types:span",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "sprite_instance",
    hdrs = ["sprite_instance.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":handles",
        "//dmlab2d/lib/system/math:math2d",
    ],
)

cc_library(
    name = "grid_view",
    srcs = ["grid_view.cc"],
    hdrs = ["grid_view.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":grid_window",
        ":handles",
        ":sprite_instance",
        "//dmlab2d/lib/system/grid_world/collections:fixed_handle_map",
        "//dmlab2d/lib/system/math:math2d",
        "@abseil-cpp//absl/types:span",
    ],
)

cc_test(
    name = "grid_view_test",
    srcs = ["grid_view_test.cc"],
    deps = [
        ":grid_view",
        ":grid_window",
        ":handles",
        ":sprite_instance",
        "//dmlab2d/lib/system/grid_world/collections:fixed_handle_map",
        "@abseil-cpp//absl/types:span",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "grid_window",
    hdrs = ["grid_window.h"],
    deps = ["//dmlab2d/lib/system/math:math2d"],
)

cc_library(
    name = "text_tools",
    srcs = ["text_tools.cc"],
    hdrs = ["text_tools.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":handles",
        "//dmlab2d/lib/system/math:math2d",
        "@abseil-cpp//absl/strings",
    ],
)

cc_test(
    name = "text_tools_test",
    srcs = ["text_tools_test.cc"],
    deps = [
        ":text_tools",
        "@googletest//:gtest_main",
    ],
)
