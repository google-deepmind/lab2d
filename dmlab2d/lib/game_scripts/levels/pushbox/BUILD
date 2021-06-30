load("//:build_defs.bzl", "pytype_strict_binary")
load("//dmlab2d/lib/testing:lua_testing.bzl", "dmlab2d_lua_level_test")

licenses(["notice"])

filegroup(
    name = "pushbox",
    srcs = glob(["*.lua"]),
    visibility = ["//visibility:public"],
)

dmlab2d_lua_level_test(
    name = "pushbox_test",
    main = "init.lua",
)

pytype_strict_binary(
    name = "play",
    srcs = ["play.py"],
    python_version = "PY3",
    srcs_version = "PY3",
    deps = ["//dmlab2d:ui_renderer"],
)
