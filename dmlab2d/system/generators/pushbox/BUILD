# Description:
#  Pushbox level generator.

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
        "//dmlab2d/support:logging",
        "//dmlab2d/system/math:math2d",
        "@com_google_absl//absl/container:flat_hash_set",
        "@com_google_absl//absl/random",
        "@com_google_absl//absl/random:distributions",
        "@com_google_absl//absl/status",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/strings:str_format",
        "@com_google_absl//absl/types:optional",
        "@com_google_absl//absl/types:span",
    ],
)
