load("//dmlab2d/testing:lua_testing.bzl", "dmlab2d_lua_test")

licenses(["notice"])

# Library for loading and manipulating images as Tensors.
cc_library(
    name = "lua_image",
    srcs = ["lua_image.cc"],
    hdrs = ["lua_image.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d/lua",
        "//dmlab2d/lua:bind",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:push",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/support:logging",
        "//dmlab2d/system/tensor:tensor_view",
        "//dmlab2d/system/tensor/lua:tensor",
        "//dmlab2d/util:file_reader",
        "//dmlab2d/util:file_reader_types",
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
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:n_results_or_test_util",
        "//dmlab2d/lua:push",
        "//dmlab2d/lua:push_script",
        "//dmlab2d/lua:vm",
        "//dmlab2d/lua:vm_test_util",
        "//dmlab2d/system/tensor/lua:tensor",
        "//dmlab2d/util:default_read_only_file_system",
        "//dmlab2d/util:file_reader_types",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

dmlab2d_lua_test(name = "image_test")
