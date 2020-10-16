# Lua bindings for file_system.
load("//dmlab2d/testing:lua_testing.bzl", "dmlab2d_lua_test")

licenses(["notice"])

cc_library(
    name = "file_system",
    srcs = ["file_system.cc"],
    hdrs = ["file_system.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d/lua",
        "//dmlab2d/lua:class",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:push",
        "//dmlab2d/lua:read",
        "//dmlab2d/system/file_system",
        "//dmlab2d/util:file_reader",
        "//dmlab2d/util:file_reader_types",
        "//dmlab2d/util:files",
        "@com_google_absl//absl/memory",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "file_system_test",
    size = "small",
    srcs = ["file_system_test.cc"],
    deps = [
        ":file_system",
        "//dmlab2d/lua",
        "//dmlab2d/lua:bind",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or_test_util",
        "//dmlab2d/lua:push_script",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:vm",
        "//dmlab2d/lua:vm_test_util",
        "//dmlab2d/system/file_system",
        "//dmlab2d/util:default_read_only_file_system",
        "//dmlab2d/util:file_reader_types",
        "//dmlab2d/util:files",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

dmlab2d_lua_test(
    name = "lua_file_system_test",
    main = "file_system_test.lua",
)
