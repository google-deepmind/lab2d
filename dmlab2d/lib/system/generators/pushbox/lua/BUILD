# Description:
#  Lua bindings for Pushbox level generator.

load("//dmlab2d/lib/testing:lua_testing.bzl", "dmlab2d_lua_test")

licenses(["notice"])

cc_library(
    name = "pushbox",
    srcs = ["pushbox.cc"],
    hdrs = ["pushbox.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:bind",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:push",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/system/generators/pushbox",
    ],
)

dmlab2d_lua_test(name = "pushbox_test")
