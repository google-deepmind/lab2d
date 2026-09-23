# Description:
#   Tiny tensor library.

load("@rules_cc//cc:defs.bzl", "cc_library", "cc_test")

licenses(["notice"])

cc_library(
    name = "tensor_view",
    srcs = ["tensor_view.cc"],
    hdrs = ["tensor_view.h"],
    visibility = ["//visibility:public"],
    deps = ["@eigen_archive//:eigen"],
)

cc_test(
    name = "tensor_view_test",
    size = "small",
    srcs = ["tensor_view_test.cc"],
    deps = [
        ":tensor_view",
        "@com_google_googletest//:gtest_main",
    ],
)
