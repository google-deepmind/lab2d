# Description:
#   Command line interface for profiling and inspecting different Lab2D environments.

licenses(["notice"])

cc_binary(
    name = "command_line_interface",
    srcs = ["command_line_interface_main.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/flags:flag",
        "@com_google_absl//absl/flags:parse",
        "@com_google_absl//absl/flags:usage",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
    ],
)
