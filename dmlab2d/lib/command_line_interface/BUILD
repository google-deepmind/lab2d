# Description:
#   Command line interface for profiling and inspecting different Lab2D environments.

load("@rules_cc//cc:cc_binary.bzl", "cc_binary")

package(default_applicable_licenses = ["//:license"])

licenses(["notice"])

cc_binary(
    name = "command_line_interface",
    srcs = ["command_line_interface_main.cc"],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d/lib:dmlab2d",
        "//third_party/rl_api:env_c_api",
        "@abseil-cpp//absl/flags:flag",
        "@abseil-cpp//absl/flags:parse",
        "@abseil-cpp//absl/flags:usage",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/strings:str_format",
        "@abseil-cpp//absl/types:optional",
    ],
)
