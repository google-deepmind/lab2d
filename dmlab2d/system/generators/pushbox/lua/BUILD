# Description:
#  Lua bindings for Pushbox level generator.

load("//dmlab2d/testing:lua_testing.bzl", "dmlab2d_lua_test")

licenses(["notice"])

cc_library(
    name = "pushbox",
    srcs = ["pushbox.cc"],
    hdrs = ["pushbox.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d/lua",
        "//dmlab2d/lua:bind",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:push",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/system/generators/pushbox",
    ],
)

dmlab2d_lua_test(name = "pushbox_test")
