# Description:
#   Tree provides utilities for working with nested data structures.
#   The tree package used to support Bazel, but no longer does so.
#   This BUILD file is adapted from the last official version.

load("@pybind11_bazel//:build_defs.bzl", "pybind_extension")
load("@rules_python//python:py_library.bzl", "py_library")

pybind_extension(
    name = "tree/_tree",
    srcs = [
        "tree/tree.cc",
        "tree/tree.h",
    ],
    deps = [
        "@abseil-cpp//absl/memory",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/synchronization",
    ],
)

py_library(
    name = "tree",
    srcs = ["tree/__init__.py"],
    srcs_version = "PY3",
    visibility = ["@dm_env_archive//:__pkg__"],
    deps = [
        ":sequence",
        ":tree/_tree",
    ],
)

py_library(
    name = "sequence",
    srcs = ["tree/sequence.py"],
    srcs_version = "PY3",
    deps = [":tree/_tree"],
)
