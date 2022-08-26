# Description:
#   A collection of libraries for interacting with Lua.

load("@bazel_skylib//rules:common_settings.bzl", "string_flag")

package(default_visibility = ["//visibility:public"])

licenses(["notice"])

string_flag(
    name = "lua_version",
    build_setting_default = "luajit",
    values = [
        "luajit",
        "lua5_1",
        "lua5_2",
    ],
)

config_setting(
    name = "luajit",
    flag_values = {"lua_version": "luajit"},
)

config_setting(
    name = "lua5.1",
    flag_values = {"lua_version": "lua5_1"},
)

config_setting(
    name = "lua5.2",
    flag_values = {"lua_version": "lua5_2"},
)

cc_library(
    name = "lua",
    hdrs = ["lua.h"],
    linkopts = select({
        "lua5.1": ["-ldl"],
        "lua5.2": ["-ldl"],
        "//conditions:default": [],
    }),
    deps = select({
        "//conditions:default": ["@luajit_archive//:luajit"],
        ":lua5.1": ["@lua5_1_archive//:lua5_1"],
        ":lua5.2": ["@lua5_2_archive//:lua5_2"],
    }),
)

cc_library(
    name = "vm",
    srcs = ["vm.cc"],
    hdrs = ["vm.h"],
    deps = [
        ":lua",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "vm_test_util",
    testonly = True,
    hdrs = ["vm_test_util.h"],
    deps = [
        ":lua",
        ":vm",
        "@com_google_absl//absl/log:check",
        "@com_google_googletest//:gtest",
    ],
)

cc_test(
    name = "vm_test",
    size = "small",
    srcs = ["vm_test.cc"],
    data = [
        "vm_test_data/module.lua",
        "vm_test_data/module_as_dir/init.lua",
    ],
    deps = [
        ":call",
        ":n_results_or",
        ":n_results_or_test_util",
        ":push_script",
        ":read",
        ":table_ref",
        ":vm",
        "//dmlab2d/lib/util:test_srcdir",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "n_results_or",
    hdrs = ["n_results_or.h"],
)

cc_library(
    name = "n_results_or_test_util",
    testonly = True,
    hdrs = ["n_results_or_test_util.h"],
    deps = [
        ":n_results_or",
        "@com_google_googletest//:gtest",
    ],
)

cc_library(
    name = "bind",
    hdrs = ["bind.h"],
    deps = [
        ":lua",
        ":n_results_or",
    ],
)

cc_test(
    name = "bind_test",
    size = "small",
    srcs = ["bind_test.cc"],
    deps = [
        ":bind",
        ":push",
        ":read",
        ":vm_test_util",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "call",
    srcs = ["call.cc"],
    hdrs = ["call.h"],
    deps = [
        ":lua",
        ":n_results_or",
        ":push",
        ":read",
        "@com_google_absl//absl/log:check",
    ],
)

cc_test(
    name = "call_test",
    size = "small",
    srcs = ["call_test.cc"],
    deps = [
        ":bind",
        ":call",
        ":n_results_or_test_util",
        ":push",
        ":push_script",
        ":read",
        ":vm_test_util",
        "@com_google_googletest//:gtest_main",
    ],
)

# This test is currently not part of the OSS release.
cc_test(
    name = "push_script_test",
    size = "small",
    srcs = ["push_script_test.cc"],
    deps = [
        ":n_results_or_test_util",
        ":push",
        ":push_script",
        ":vm_test_util",
        "@com_google_absl//absl/flags:flag",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "push",
    hdrs = ["push.h"],
    deps = [
        ":lua",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_test(
    name = "push_test",
    size = "small",
    srcs = ["push_test.cc"],
    deps = [
        ":push",
        ":vm_test_util",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "push_script",
    srcs = ["push_script.cc"],
    hdrs = ["push_script.h"],
    deps = [
        ":lua",
        ":n_results_or",
        ":read",
        "@com_google_absl//absl/strings",
    ],
)

cc_library(
    name = "read",
    hdrs = ["read.h"],
    deps = [
        ":lua",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_absl//absl/types:variant",
    ],
)

cc_test(
    name = "read_test",
    size = "small",
    srcs = ["read_test.cc"],
    deps = [
        ":push",
        ":read",
        ":vm_test_util",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_absl//absl/types:variant",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "table_ref",
    srcs = ["table_ref.cc"],
    hdrs = ["table_ref.h"],
    deps = [
        ":lua",
        ":push",
        ":read",
        "@com_google_absl//absl/log:check",
    ],
)

cc_test(
    name = "table_ref_test",
    size = "small",
    srcs = ["table_ref_test.cc"],
    deps = [
        ":call",
        ":class",
        ":n_results_or",
        ":n_results_or_test_util",
        ":push_script",
        ":read",
        ":table_ref",
        ":vm_test_util",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "ref",
    srcs = ["ref.cc"],
    hdrs = ["ref.h"],
    deps = [
        ":call",
        ":lua",
        ":n_results_or",
        ":push",
        ":read",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/utility",
    ],
)

cc_test(
    name = "ref_test",
    size = "small",
    srcs = ["ref_test.cc"],
    deps = [
        ":n_results_or_test_util",
        ":push",
        ":push_script",
        ":read",
        ":ref",
        ":vm_test_util",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "class",
    hdrs = ["class.h"],
    deps = [
        ":lua",
        ":n_results_or",
        ":push",
        ":read",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "class_test",
    size = "small",
    srcs = ["class_test.cc"],
    deps = [
        ":bind",
        ":call",
        ":class",
        ":n_results_or_test_util",
        ":push_script",
        ":read",
        ":table_ref",
        ":vm",
        ":vm_test_util",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "stack_resetter",
    hdrs = ["stack_resetter.h"],
    deps = [":lua"],
)

cc_test(
    name = "stack_resetter_test",
    size = "small",
    srcs = ["stack_resetter_test.cc"],
    deps = [
        ":call",
        ":n_results_or",
        ":n_results_or_test_util",
        ":push",
        ":push_script",
        ":read",
        ":stack_resetter",
        ":vm_test_util",
        "@com_google_googletest//:gtest_main",
    ],
)
