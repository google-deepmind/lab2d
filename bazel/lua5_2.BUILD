# Description:
#   Build rule for Lua 5.2.

cc_library(
    name = "lua5_2",
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
