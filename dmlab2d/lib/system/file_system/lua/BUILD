# Lua bindings for file_system.
load("//dmlab2d/lib/testing:lua_testing.bzl", "dmlab2d_lua_test")

licenses(["notice"])

cc_library(
    name = "file_system",
    srcs = ["file_system.cc"],
    hdrs = ["file_system.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:class",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:push",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/system/file_system",
        "//dmlab2d/lib/util:file_reader",
        "//dmlab2d/lib/util:file_reader_types",
        "//dmlab2d/lib/util:files",
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
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:bind",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:n_results_or_test_util",
        "//dmlab2d/lib/lua:push_script",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:vm",
        "//dmlab2d/lib/lua:vm_test_util",
        "//dmlab2d/lib/system/file_system",
        "//dmlab2d/lib/util:default_read_only_file_system",
        "//dmlab2d/lib/util:file_reader_types",
        "//dmlab2d/lib/util:files",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

dmlab2d_lua_test(
    name = "lua_file_system_test",
    main = "file_system_test.lua",
)
