load("@rules_python//python:py_binary.bzl", "py_binary")
load("//dmlab2d/lib/testing:lua_testing.bzl", "dmlab2d_lua_level_test")

package(default_applicable_licenses = ["//:license"])

licenses(["notice"])

filegroup(
    name = "running_with_scissors",
    srcs = glob(["*.lua"]),
    visibility = ["//visibility:public"],
)

dmlab2d_lua_level_test(
    name = "running_with_scissors_test",
    main = "init.lua",
)

py_binary(
    name = "play",
    srcs = ["play.py"],
    deps = ["//dmlab2d:ui_renderer"],
)
