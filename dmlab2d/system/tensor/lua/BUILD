# Description:
#  Bindings to tensor library.

licenses(["notice"])

cc_library(
    name = "tensor",
    srcs = ["tensor.cc"],
    hdrs = ["tensor.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d/lua",
        "//dmlab2d/lua:bind",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:class",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:push",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/support:logging",
        "//dmlab2d/system/random/lua:random",
        "//dmlab2d/system/tensor:tensor_view",
        "//dmlab2d/util:file_reader",
        "//dmlab2d/util:file_reader_types",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "tensor_test",
    size = "small",
    srcs = ["tensor_test.cc"],
    deps = [
        ":tensor",
        "//dmlab2d/lua:bind",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:n_results_or_test_util",
        "//dmlab2d/lua:push_script",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/lua:vm",
        "//dmlab2d/lua:vm_test_util",
        "//dmlab2d/util:default_read_only_file_system",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "tensor_benchmark",
    size = "small",
    srcs = ["tensor_benchmark.cc"],
    deps = [
        ":tensor",
        "//dmlab2d/lua",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:n_results_or_test_util",
        "//dmlab2d/lua:push_script",
        "//dmlab2d/lua:vm",
        "//dmlab2d/util:default_read_only_file_system",
        "//dmlab2d/util:file_reader_types",
        "@com_google_absl//absl/strings",
        "@com_google_benchmark//:benchmark",
        "@com_google_benchmark//:benchmark_main",
    ],
)
