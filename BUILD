load("@bazel_skylib//:bzl_library.bzl", "bzl_library")

licenses(["notice"])

exports_files([
    "LICENSE",
    "README.md",
])

bzl_library(
    name = "build_defs",
    srcs = ["build_defs.bzl"],
    deps = ["@bazel_skylib//lib:collections"],
)

bzl_library(
    name = "python_system",
    srcs = ["python_system.bzl"],
)
