load("//:build_defs.bzl", "pytype_strict_binary")
load("//dmlab2d/testing:lua_testing.bzl", "dmlab2d_lua_level_test")

licenses(["notice"])

filegroup(
    name = "chase_eat",
    srcs = glob(["*.lua"]),
    visibility = ["//visibility:public"],
)

dmlab2d_lua_level_test(
    name = "chase_eat_test",
    data = [":chase_eat"],
    main = "init.lua",
)

pytype_strict_binary(
    name = "play",
    srcs = ["play.py"],
    data = [":chase_eat"],
    python_version = "PY3",
    srcs_version = "PY3",
    deps = ["//dmlab2d/python:ui_renderer"],
)
