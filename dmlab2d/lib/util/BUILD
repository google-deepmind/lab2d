# Description:
#   General utilities.

load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

package(default_applicable_licenses = ["//:license"])

licenses(["notice"])

# Library for platform-specific file operations.
cc_library(
    name = "files",
    srcs = ["files.cc"],
    hdrs = ["files.h"],
    visibility = ["//visibility:public"],
    deps = ["@abseil-cpp//absl/strings"],
)

cc_test(
    name = "files_test",
    srcs = ["files_test.cc"],
    deps = [
        ":files",
        "@googletest//:gtest_main",
    ],
)

# Default readonly file system.
cc_library(
    name = "default_read_only_file_system",
    srcs = ["default_read_only_file_system.cc"],
    hdrs = ["default_read_only_file_system.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":file_reader_types",
        "@abseil-cpp//absl/strings",
    ],
)

# Library for read-only file operations.
cc_library(
    name = "file_reader",
    srcs = ["file_reader.cc"],
    hdrs = ["file_reader.h"],
    visibility = ["//visibility:public"],
    deps = [":file_reader_types"],
)

cc_test(
    name = "file_reader_test",
    srcs = ["file_reader_test.cc"],
    deps = [
        ":default_read_only_file_system",
        ":file_reader",
        ":file_reader_types",
        ":files",
        "@abseil-cpp//absl/log:check",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "file_reader_types",
    hdrs = ["file_reader_types.h"],
    visibility = ["//visibility:public"],
)

cc_library(
    name = "visit_set_difference_and_intersection",
    hdrs = ["visit_set_difference_and_intersection.h"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "visit_set_difference_and_intersection_test",
    srcs = ["visit_set_difference_and_intersection_test.cc"],
    deps = [
        ":visit_set_difference_and_intersection",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "test_srcdir",
    testonly = True,
    srcs = ["test_srcdir.cc"],
    hdrs = ["test_srcdir.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@abseil-cpp//absl/strings",
        "@googletest//:gtest",
    ],
)
