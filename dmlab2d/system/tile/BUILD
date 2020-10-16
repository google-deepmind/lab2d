# Description:
#   Tile based rendering.
licenses(["notice"])

cc_library(
    name = "pixel",
    hdrs = ["pixel.h"],
    visibility = [":__subpackages__"],
    deps = ["//dmlab2d/support:logging"],
)

cc_test(
    name = "pixel_test",
    srcs = ["pixel_test.cc"],
    deps = [
        ":pixel",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "tile_renderer",
    srcs = ["tile_renderer.cc"],
    hdrs = ["tile_renderer.h"],
    visibility = [":__subpackages__"],
    deps = [
        ":pixel",
        ":tile_set",
        "//dmlab2d/support:logging",
        "//dmlab2d/system/math:math2d",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "tile_renderer_test",
    srcs = ["tile_renderer_test.cc"],
    deps = [
        ":pixel",
        ":tile_renderer",
        "//dmlab2d/system/math:math2d",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "tile_set",
    srcs = ["tile_set.cc"],
    hdrs = ["tile_set.h"],
    visibility = [":__subpackages__"],
    deps = [
        ":pixel",
        "//dmlab2d/system/math:math2d",
        "//dmlab2d/system/tensor:tensor_view",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "tile_set_test",
    srcs = ["tile_set_test.cc"],
    deps = [
        ":pixel",
        ":tile_set",
        "//dmlab2d/system/math:math2d",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)
