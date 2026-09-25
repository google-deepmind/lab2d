# Description:
#   Python bindings for Pushbox level generator.

load("@pybind11_bazel//:build_defs.bzl", "pybind_extension")
load("@rules_python//python:py_test.bzl", "py_test")

package(default_applicable_licenses = ["//:license"])

licenses(["notice"])

pybind_extension(
    name = "pushbox",
    srcs = ["pushbox.cc"],
    copts = ["-fexceptions"],
    visibility = ["//visibility:private"],
    deps = [
        "//dmlab2d/lib/system/generators/pushbox",
        "@abseil-cpp//absl/types:optional",
    ],
)

py_test(
    name = "pushbox_test",
    srcs = ["pushbox_test.py"],
    deps = [
        ":pushbox",
        "@abseil-py//absl/testing:absltest",
    ],
)
