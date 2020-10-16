licenses(["notice"])

cc_library(
    name = "world",
    srcs = ["world.cc"],
    hdrs = ["world.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":handles",
        "//dmlab2d/system/grid_world/collections:fixed_handle_map",
        "//dmlab2d/system/grid_world/collections:handle_names",
        "@com_google_absl//absl/container:btree",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "world_test",
    srcs = ["world_test.cc"],
    deps = [
        ":handles",
        ":world",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "grid_shape",
    hdrs = ["grid_shape.h"],
    deps = [
        ":handles",
        "//dmlab2d/system/math:math2d",
    ],
)

cc_test(
    name = "grid_shape_test",
    srcs = ["grid_shape_test.cc"],
    deps = [
        ":grid_shape",
        "//dmlab2d/system/math:math2d",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "handles",
    hdrs = ["handles.h"],
    visibility = ["//visibility:public"],
    deps = ["//dmlab2d/system/grid_world/collections:handle"],
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
        "//dmlab2d/system/grid_world/collections:fixed_handle_map",
        "//dmlab2d/system/grid_world/collections:object_pool",
        "//dmlab2d/system/grid_world/collections:shuffled_membership",
        "//dmlab2d/system/math:math2d",
        "//dmlab2d/system/math:math2d_algorithms",
        "@com_google_absl//absl/types:any",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
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
        "//dmlab2d/system/grid_world/collections:fixed_handle_map",
        "//dmlab2d/system/math:math2d",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "sprite_instance",
    hdrs = ["sprite_instance.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":handles",
        "//dmlab2d/system/math:math2d",
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
        "//dmlab2d/system/grid_world/collections:fixed_handle_map",
        "//dmlab2d/system/math:math2d",
        "@com_google_absl//absl/types:span",
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
        "//dmlab2d/system/grid_world/collections:fixed_handle_map",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "grid_window",
    hdrs = ["grid_window.h"],
    deps = ["//dmlab2d/system/math:math2d"],
)

cc_library(
    name = "text_tools",
    srcs = ["text_tools.cc"],
    hdrs = ["text_tools.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":handles",
        "//dmlab2d/system/math:math2d",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "text_tools_test",
    srcs = ["text_tools_test.cc"],
    deps = [
        ":text_tools",
        "@com_google_googletest//:gtest_main",
    ],
)
