# Description:
#  Python bindings for Pushbox level generator.

load("//:build_defs.bzl", "pybind_extension")

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
