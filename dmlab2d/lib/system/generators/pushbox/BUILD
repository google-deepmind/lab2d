# Description:
#   Pushbox level generator.

load("@rules_cc//cc:cc_library.bzl", "cc_library")

package(default_applicable_licenses = ["//:license"])

licenses(["notice"])

cc_library(
    name = "pushbox",
    srcs = [
        "constants.h",
        "pushbox.cc",
        "random_room_generator.cc",
        "random_room_generator.h",
        "room.cc",
        "room.h",
        "room_candidate_generator.cc",
        "room_candidate_generator.h",
    ],
    hdrs = ["pushbox.h"],
    visibility = ["//visibility:public"],
    deps = [
        "//dmlab2d/lib/system/math:math2d",
        "@abseil-cpp//absl/container:flat_hash_set",
        "@abseil-cpp//absl/log",
        "@abseil-cpp//absl/log:check",
        "@abseil-cpp//absl/strings",
        "@abseil-cpp//absl/strings:str_format",
        "@abseil-cpp//absl/types:span",
    ],
)
