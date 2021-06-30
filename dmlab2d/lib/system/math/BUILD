# Description:
#   2D Math library.

licenses(["notice"])

cc_library(
    name = "math2d",
    hdrs = ["math2d.h"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "math2d_test",
    srcs = ["math2d_test.cc"],
    deps = [
        ":math2d",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "math2d_algorithms",
    hdrs = ["math2d_algorithms.h"],
    visibility = ["//visibility:public"],
    deps = [":math2d"],
)

cc_test(
    name = "math2d_algorithms_test",
    srcs = ["math2d_algorithms_test.cc"],
    deps = [
        ":math2d",
        ":math2d_algorithms",
        "@com_google_googletest//:gtest_main",
    ],
)
