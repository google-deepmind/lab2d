# Description:
#   Tile based rendering.

load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

package(default_applicable_licenses = ["//:license"])

licenses(["notice"])

cc_library(
    name = "pixel",
    hdrs = ["pixel.h"],
    visibility = [":__subpackages__"],
    deps = ["@abseil-cpp//absl/log:check"],
)

cc_test(
    name = "pixel_test",
    srcs = ["pixel_test.cc"],
    deps = [
        ":pixel",
        "@googletest//:gtest_main",
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
        "//dmlab2d/lib/system/math:math2d",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/log:check",
        "@abseil-cpp//absl/types:span",
    ],
)

cc_test(
    name = "tile_renderer_test",
    srcs = ["tile_renderer_test.cc"],
    deps = [
        ":pixel",
        ":tile_renderer",
        ":tile_set",
        "//dmlab2d/lib/system/math:math2d",
        "//dmlab2d/lib/system/tensor:tensor_view",
        "@abseil-cpp//absl/log:check",
        "@abseil-cpp//absl/types:span",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "tile_set",
    srcs = ["tile_set.cc"],
    hdrs = ["tile_set.h"],
    visibility = [":__subpackages__"],
    deps = [
        ":pixel",
        "//dmlab2d/lib/system/math:math2d",
        "//dmlab2d/lib/system/tensor:tensor_view",
        "@abseil-cpp//absl/types:span",
    ],
)

cc_test(
    name = "tile_set_test",
    srcs = ["tile_set_test.cc"],
    deps = [
        ":pixel",
        ":tile_set",
        "//dmlab2d/lib/system/math:math2d",
        "//dmlab2d/lib/system/tensor:tensor_view",
        "@abseil-cpp//absl/log:check",
        "@googletest//:gtest_main",
    ],
)
