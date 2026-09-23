# Description:
#  Python bindings for Pushbox level generator.

load("@pybind11_bazel//:build_defs.bzl", "pybind_extension")
load("@rules_python//python:py_test.bzl", "py_test")

licenses(["notice"])

pybind_extension(
    name = "pushbox",
    srcs = ["pushbox.cc"],
    copts = ["-fexceptions"],
    visibility = ["//visibility:private"],
    deps = [
        "//dmlab2d/lib/system/generators/pushbox",
        "@com_google_absl//absl/types:optional",
    ],
)

py_test(
    name = "pushbox_test",
    srcs = ["pushbox_test.py"],
    deps = [
        ":pushbox",
        "@com_google_absl_py//absl/testing:absltest",
    ],
)
