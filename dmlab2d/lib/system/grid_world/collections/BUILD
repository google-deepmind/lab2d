load("@rules_cc//cc:cc_library.bzl", "cc_library")
load("@rules_cc//cc:cc_test.bzl", "cc_test")

licenses(["notice"])

cc_library(
    name = "handle",
    hdrs = ["handle.h"],
    visibility = ["//visibility:public"],
    deps = ["@abseil-cpp//absl/log:check"],
)

cc_test(
    name = "handle_test",
    srcs = ["handle_test.cc"],
    deps = [
        ":handle",
        "@abseil-cpp//absl/hash:hash_testing",
        "@abseil-cpp//absl/strings:str_format",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "fixed_handle_map",
    hdrs = ["fixed_handle_map.h"],
    visibility = ["//visibility:public"],
)

cc_test(
    name = "fixed_handle_map_test",
    srcs = ["fixed_handle_map_test.cc"],
    deps = [
        ":fixed_handle_map",
        ":handle",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "handle_names",
    hdrs = ["handle_names.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@abseil-cpp//absl/container:flat_hash_map",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/types:span",
    ],
)

cc_test(
    name = "handle_names_test",
    srcs = ["handle_names_test.cc"],
    deps = [
        ":handle",
        ":handle_names",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "object_pool",
    hdrs = ["object_pool.h"],
    visibility = ["//visibility:public"],
    deps = ["@abseil-cpp//absl/log:check"],
)

cc_test(
    name = "object_pool_test",
    srcs = ["object_pool_test.cc"],
    deps = [
        ":handle",
        ":object_pool",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "shuffled_set",
    hdrs = ["shuffled_set.h"],
    deps = [
        "@abseil-cpp//absl/log:check",
        "@abseil-cpp//absl/types:span",
    ],
)

cc_test(
    name = "shuffled_set_test",
    srcs = ["shuffled_set_test.cc"],
    deps = [
        ":shuffled_set",
        "@googletest//:gtest_main",
    ],
)

cc_library(
    name = "shuffled_membership",
    hdrs = ["shuffled_membership.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":fixed_handle_map",
        ":shuffled_set",
        "//dmlab2d/lib/util:visit_set_difference_and_intersection",
        "@abseil-cpp//absl/types:span",
    ],
)

cc_test(
    name = "shuffled_membership_test",
    srcs = ["shuffled_membership_test.cc"],
    deps = [
        ":handle",
        ":shuffled_membership",
        "@abseil-cpp//absl/types:span",
        "@googletest//:gtest_main",
    ],
)
