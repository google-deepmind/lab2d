# Description:
#   Tiny tensor library.

load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

package(default_applicable_licenses = ["//:license"])

licenses(["notice"])

cc_library(
    name = "tensor_view",
    srcs = ["tensor_view.cc"],
    hdrs = ["tensor_view.h"],
    visibility = ["//visibility:public"],
    deps = ["@eigen"],
)

cc_test(
    name = "tensor_view_test",
    size = "small",
    srcs = ["tensor_view_test.cc"],
    deps = [
        ":tensor_view",
        "@googletest//:gtest_main",
    ],
)
