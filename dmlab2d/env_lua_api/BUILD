# EnvCApi - Lua bindings.

licenses(["notice"])

cc_library(
    name = "env_lua_api",
    srcs = ["env_lua_api.cc"],
    hdrs = ["env_lua_api.h"],
    visibility = ["//visibility:public"],
    deps = [
        ":actions",
        ":episode",
        ":events",
        ":observations",
        ":properties",
        "//dmlab2d/lua",
        "//dmlab2d/lua:bind",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:push",
        "//dmlab2d/lua:push_script",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:stack_resetter",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/lua:vm",
        "//dmlab2d/system/file_system",
        "//dmlab2d/system/image:lua_image",
        "//dmlab2d/system/random/lua:random",
        "//dmlab2d/system/tensor/lua:tensor",
        "//dmlab2d/util:default_read_only_file_system",
        "//dmlab2d/util:file_reader_types",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/container:flat_hash_map",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "env_lua_api_test",
    srcs = ["env_lua_api_test.cc"],
    deps = [
        ":env_lua_api",
        "//dmlab2d/util:files",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "events",
    srcs = ["events.cc"],
    hdrs = ["events.h"],
    deps = [
        "//dmlab2d/lua",
        "//dmlab2d/lua:class",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:read",
        "//dmlab2d/support:logging",
        "//dmlab2d/system/tensor:tensor_view",
        "//dmlab2d/system/tensor/lua:tensor",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/container:node_hash_map",
    ],
)

cc_library(
    name = "episode",
    srcs = ["episode.cc"],
    hdrs = ["episode.h"],
    deps = [
        "//dmlab2d/lua",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:push",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:stack_resetter",
        "//dmlab2d/lua:table_ref",
        "//third_party/rl_api:env_c_api",
    ],
)

cc_test(
    name = "episode_test",
    srcs = ["episode_test.cc"],
    deps = [
        ":episode",
        "//dmlab2d/lua",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or_test_util",
        "//dmlab2d/lua:push_script",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/lua:vm_test_util",
        "//third_party/rl_api:env_c_api",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "events_test",
    srcs = ["events_test.cc"],
    deps = [
        ":events",
        "//dmlab2d/lua",
        "//dmlab2d/lua:bind",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or_test_util",
        "//dmlab2d/lua:push_script",
        "//dmlab2d/lua:vm",
        "//dmlab2d/lua:vm_test_util",
        "//dmlab2d/system/random/lua:random",
        "//dmlab2d/system/tensor/lua:tensor",
        "//dmlab2d/util:default_read_only_file_system",
        "//dmlab2d/util:file_reader_types",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "actions",
    srcs = ["actions.cc"],
    hdrs = ["actions.h"],
    deps = [
        "//dmlab2d/lua",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:push",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:stack_resetter",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/support:logging",
        "//dmlab2d/system/tensor/lua:tensor",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "actions_test",
    srcs = ["actions_test.cc"],
    deps = [
        ":actions",
        "//dmlab2d/lua",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or_test_util",
        "//dmlab2d/lua:push_script",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/lua:vm_test_util",
        "//third_party/rl_api:env_c_api",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "observations",
    srcs = ["observations.cc"],
    hdrs = ["observations.h"],
    deps = [
        "//dmlab2d/lua",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:class",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:push",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/support:logging",
        "//dmlab2d/system/tensor:tensor_view",
        "//dmlab2d/system/tensor/lua:tensor",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "observations_test",
    srcs = ["observations_test.cc"],
    deps = [
        ":observations",
        "//dmlab2d/lua",
        "//dmlab2d/lua:bind",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or_test_util",
        "//dmlab2d/lua:push_script",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/lua:vm",
        "//dmlab2d/lua:vm_test_util",
        "//dmlab2d/system/random/lua:random",
        "//dmlab2d/system/tensor/lua:tensor",
        "//dmlab2d/util:default_read_only_file_system",
        "//dmlab2d/util:file_reader_types",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/types:span",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "properties",
    srcs = ["properties.cc"],
    hdrs = ["properties.h"],
    deps = [
        "//dmlab2d/lua",
        "//dmlab2d/lua:bind",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or",
        "//dmlab2d/lua:push",
        "//dmlab2d/lua:read",
        "//dmlab2d/lua:stack_resetter",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/support:logging",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "properties_test",
    srcs = ["properties_test.cc"],
    deps = [
        ":properties",
        "//dmlab2d/lua",
        "//dmlab2d/lua:bind",
        "//dmlab2d/lua:call",
        "//dmlab2d/lua:n_results_or_test_util",
        "//dmlab2d/lua:push_script",
        "//dmlab2d/lua:table_ref",
        "//dmlab2d/lua:vm_test_util",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)
