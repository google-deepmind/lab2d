# Description:
#   Lua testing libraries.

load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

licenses(["notice"])

# Library for running tests using DeepMind Lab2D's Lua VM.
cc_library(
    name = "lua_unit_test_lib",
    testonly = 1,
    srcs = ["lua_unit_test.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d/lib:dmlab2d",
        "//dmlab2d/lib/util:test_srcdir",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest",
    ],
)

# Tests whether level script produces a valid environment.
cc_library(
    name = "lua_level_test_lib",
    testonly = 1,
    srcs = ["lua_level_test.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d/lib:dmlab2d",
        "//dmlab2d/lib/util:test_srcdir",
        "//third_party/rl_api:env_c_api",
        "//third_party/rl_api:env_c_api_test_suite",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest",
    ],
)

bzl_library(
    name = "lua_testing_bzl",
    srcs = ["lua_testing.bzl"],
    visibility = ["//visibility:private"],
)
