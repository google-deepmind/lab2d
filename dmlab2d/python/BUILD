load("//:build_defs.bzl", "py2and3_strict_test")
load("//:build_defs.bzl", "pytype_strict_library")
load("//:build_defs.bzl", "pybind_extension")

licenses(["notice"])

pybind_extension(
    name = "dmlab2d",
    srcs = ["dmlab2d.cc"],
    copts = ["-fexceptions"],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d",
        "//dmlab2d/support:logging",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
    ],
)

py2and3_strict_test(
    name = "dmlab2d_test",
    srcs = ["dmlab2d_test.py"],
    deps = [
        ":dmlab2d",
        ":expect_numpy_installed",
        ":runfiles_helper",
        "@com_google_absl_py//absl/testing:absltest",
    ],
)

pytype_strict_library(
    name = "dmlab2d_env",
    srcs = ["dmlab2d_env.py"],
    visibility = ["//visibility:public"],
    deps = [
        ":dmlab2d",
        ":expect_numpy_installed",
        "@dm_env_archive//:dm_env",
        "@six_archive//:six",
    ],
)

py_test(
    name = "dmlab2d_env_test",
    srcs = ["dmlab2d_env_test.py"],
    deps = [
        ":dmlab2d",
        ":dmlab2d_env",
        ":runfiles_helper",
        "@com_google_absl_py//absl/testing:absltest",
        "@dm_env_archive//:test_utils",
    ],
)

pytype_strict_library(
    name = "settings_helper",
    srcs = ["settings_helper.py"],
    visibility = ["//visibility:public"],
)

py2and3_strict_test(
    name = "settings_helper_test",
    srcs = ["settings_helper_test.py"],
    deps = [
        ":settings_helper",
        "@com_google_absl_py//absl/testing:absltest",
    ],
)

pytype_strict_library(
    name = "ui_renderer",
    srcs = ["ui_renderer.py"],
    visibility = ["//visibility:public"],
    deps = [
        ":dmlab2d",
        ":expect_dataclasses_installed",
        ":expect_numpy_installed",
        ":expect_pygame_installed",
        ":expect_typing_installed",
        ":runfiles_helper",
    ],
)

py_binary(
    name = "random_agent",
    srcs = ["random_agent.py"],
    python_version = "PY3",
    srcs_version = "PY3",
    deps = [
        ":dmlab2d",
        ":dmlab2d_env",
        ":expect_numpy_installed",
        ":expect_pygame_installed",
        ":runfiles_helper",
    ],
)

pytype_strict_library(
    name = "runfiles_helper",
    srcs = ["runfiles_helper.py"],
    visibility = ["//visibility:public"],
)

py_library(name = "expect_numpy_installed")

py_library(name = "expect_pygame_installed")

py_library(name = "expect_dataclasses_installed")

py_library(name = "expect_typing_installed")
