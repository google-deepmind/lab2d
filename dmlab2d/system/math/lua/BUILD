# Description:
#   Lua bindings to 2D Math.

licenses(["notice"])

cc_library(
    name = "math2d",
    srcs = ["math2d.cc"],
    hdrs = ["math2d.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d/lua",
        "//dmlab2d/lua:push",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/system/math:math2d",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "math2d_test",
    srcs = ["math2d_test.cc"],
    deps = [
        ":math2d",
        "//dmlab2d/lua:push",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/lua:vm_test_util",
        "//dmlab2d/system/math:math2d",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)
