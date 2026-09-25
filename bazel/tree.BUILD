# Description:
#   Tree provides utilities for working with nested data structures.
#   The tree package used to support Bazel, but no longer does so.
#   This BUILD file is adapted from the last official version.

load("@rules_cc//cc:defs.bzl", "cc_binary")
load("@rules_python//python:py_library.bzl", "py_library")

cc_binary(
    name = "tree/_tree.so",
    srcs = [
        "tree/tree.cc",
        "tree/tree.h",
    ],
    linkopts = select({
        "@platforms//os:macos": ["-Wl,-undefined,dynamic_lookup"],
        "//conditions:default": [],
    }),
    linkshared = 1,
    linkstatic = 1,
    deps = [
        "@abseil-cpp//absl/memory",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/synchronization",
        "@pybind11",
        "@rules_python//python/cc:current_py_cc_headers",
    ],
)

py_library(
    name = "tree",
    srcs = ["tree/__init__.py"],
    data = [":tree/_tree.so"],
    srcs_version = "PY3",
    visibility = ["@dm_env_archive//:__pkg__"],
    deps = [":sequence"],
)

py_library(
    name = "sequence",
    srcs = ["tree/sequence.py"],
    data = [":tree/_tree.so"],
    srcs_version = "PY3",
)
