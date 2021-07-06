load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

licenses(["notice"])

exports_files(["LICENSE"])

config_setting(
    name = "is_linux",
    constraint_values = ["@platforms//os:linux"],
    visibility = ["//visibility:public"],
)

config_setting(
    name = "is_macos",
    constraint_values = ["@platforms//os:macos"],
    visibility = ["//visibility:public"],
)

bzl_library(
    name = "build_defs",
    srcs = ["build_defs.bzl"],
    deps = ["@bazel_skylib//lib:collections"],
)

bzl_library(
    name = "wheel_defs",
    srcs = ["wheel_defs.bzl"],
)

bzl_library(
    name = "python_system",
    srcs = ["python_system.bzl"],
)
