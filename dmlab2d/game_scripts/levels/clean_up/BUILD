load("//:build_defs.bzl", "pytype_strict_binary")
load("//dmlab2d/testing:lua_testing.bzl", "dmlab2d_lua_level_test")

licenses(["notice"])

filegroup(
    name = "clean_up",
    srcs = glob([
        "*.lua",
        "images/**/*.png",
    ]),
    visibility = ["//visibility:public"],
)

dmlab2d_lua_level_test(
    name = "clean_up_test",
    main = "init.lua",
)

pytype_strict_binary(
    name = "play",
    srcs = ["play.py"],
    python_version = "PY3",
    srcs_version = "PY3",
    deps = ["//dmlab2d/python:ui_renderer"],
)
