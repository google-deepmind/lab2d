licenses(["notice"])

cc_library(
    name = "handle",
    hdrs = ["handle.h"],
    visibility = ["//visibility:public"],
    deps = ["//dmlab2d/lib/support:logging"],
)

cc_test(
    name = "handle_test",
    srcs = ["handle_test.cc"],
    deps = [
        ":handle",
        "@com_google_absl//absl/hash:hash_testing",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_googletest//:gtest_main",
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
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "handle_names",
    hdrs = ["handle_names.h"],
    visibility = ["//visibility:public"],
    deps = [
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "handle_names_test",
    srcs = ["handle_names_test.cc"],
    deps = [
        ":handle",
        ":handle_names",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "object_pool",
    hdrs = ["object_pool.h"],
    visibility = ["//visibility:public"],
    deps = ["//dmlab2d/lib/support:logging"],
)

cc_test(
    name = "object_pool_test",
    srcs = ["object_pool_test.cc"],
    deps = [
        ":handle",
        ":object_pool",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "shuffled_set",
    hdrs = ["shuffled_set.h"],
    deps = [
        "//dmlab2d/lib/support:logging",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "shuffled_set_test",
    srcs = ["shuffled_set_test.cc"],
    deps = [
        ":shuffled_set",
        "@com_google_googletest//:gtest_main",
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
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "shuffled_membership_test",
    srcs = ["shuffled_membership_test.cc"],
    deps = [
        ":handle",
        ":shuffled_membership",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)
