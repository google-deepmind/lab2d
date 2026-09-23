# Description:
#   Build rule for Lua 5.1.

load("@rules_cc//cc:defs.bzl", "cc_library")

cc_library(
    name = "lua5_1",
    srcs = glob(
        include = [
            "*.c",
            "*.h",
        ],
        exclude = [
            "lauxlib.h",
            "lua.c",
            "lua.h",
            "luac.c",
            "lualib.h",
            "print.c",
        ],
    ),
    hdrs = [
        "lauxlib.h",
        "lua.h",
        "lualib.h",
    ],
    visibility = ["//visibility:public"],
)
