# Description:
#  Bindings to tensor library.

licenses(["notice"])

cc_library(
    name = "tensor",
    srcs = ["tensor.cc"],
    hdrs = ["tensor.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:bind",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:class",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:push",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/support:logging",
        "//dmlab2d/lib/system/random/lua:random",
        "//dmlab2d/lib/system/tensor:tensor_view",
        "//dmlab2d/lib/util:file_reader",
        "//dmlab2d/lib/util:file_reader_types",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "tensor_test",
    size = "small",
    srcs = ["tensor_test.cc"],
    deps = [
        ":tensor",
        "//dmlab2d/lib/lua:bind",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:n_results_or_test_util",
        "//dmlab2d/lib/lua:push_script",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/lua:vm",
        "//dmlab2d/lib/lua:vm_test_util",
        "//dmlab2d/lib/util:default_read_only_file_system",
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
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:n_results_or_test_util",
        "//dmlab2d/lib/lua:push_script",
        "//dmlab2d/lib/lua:vm",
        "//dmlab2d/lib/util:default_read_only_file_system",
        "//dmlab2d/lib/util:file_reader_types",
        "@com_google_absl//absl/strings",
        "@com_google_benchmark//:benchmark",
        "@com_google_benchmark//:benchmark_main",
    ],
)
