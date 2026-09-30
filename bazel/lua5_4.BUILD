# Description:
#   Build rule for Lua 5.4.

load("@rules_cc//cc:cc_library.bzl", "cc_library")

cc_library(
    name = "lua5_4",
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
