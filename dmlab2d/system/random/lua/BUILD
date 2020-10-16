licenses(["notice"])

# Library for generating random numbers from Lua.
cc_library(
    name = "random",
    srcs = ["random.cc"],
    hdrs = ["random.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d/lua",
        "//dmlab2d/lua:class",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:push",
        "//dmlab2d/lua:read",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "random_test",
    size = "small",
    srcs = ["random_test.cc"],
    deps = [
        ":random",
        "//dmlab2d/lua:bind",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or_test_util",
        "//dmlab2d/lua:push_script",
        "//dmlab2d/lua:vm",
        "//dmlab2d/lua:vm_test_util",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)
