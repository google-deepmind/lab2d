load("//dmlab2d/lib/testing:lua_testing.bzl", "dmlab2d_lua_test")

licenses(["notice"])

# Library for loading and manipulating images as Tensors.
cc_library(
    name = "lua_image",
    srcs = ["lua_image.cc"],
    hdrs = ["lua_image.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:bind",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:push",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/system/tensor:tensor_view",
        "//dmlab2d/lib/system/tensor/lua:tensor",
        "//dmlab2d/lib/util:file_reader",
        "//dmlab2d/lib/util:file_reader_types",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
        "@png_archive//:png",
    ],
)

cc_test(
    name = "lua_image_test",
    size = "small",
    srcs = ["lua_image_test.cc"],
    deps = [
        ":lua_image",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:n_results_or_test_util",
        "//dmlab2d/lib/lua:push",
        "//dmlab2d/lib/lua:push_script",
        "//dmlab2d/lib/lua:vm",
        "//dmlab2d/lib/lua:vm_test_util",
        "//dmlab2d/lib/system/tensor/lua:tensor",
        "//dmlab2d/lib/util:default_read_only_file_system",
        "//dmlab2d/lib/util:file_reader_types",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

dmlab2d_lua_test(name = "image_test")
