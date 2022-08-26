# EnvCApi - Lua bindings.

licenses(["notice"])

cc_library(
    name = "env_lua_api",
    srcs = ["env_lua_api.cc"],
    hdrs = ["env_lua_api.h"],
    data = select({
        "//dmlab2d/lib/lua:luajit": ["@luajit_archive//:jit_fs"],
        "//conditions:default": [],
    }),
    visibility = ["//visibility:public"],
    deps = [
        ":actions",
        ":episode",
        ":events",
        ":observations",
        ":properties",
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:bind",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:push",
        "//dmlab2d/lib/lua:push_script",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:stack_resetter",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/lua:vm",
        "//dmlab2d/lib/system/file_system",
        "//dmlab2d/lib/system/image:lua_image",
        "//dmlab2d/lib/system/random/lua:random",
        "//dmlab2d/lib/system/tensor/lua:tensor",
        "//dmlab2d/lib/util:default_read_only_file_system",
        "//dmlab2d/lib/util:file_reader_types",
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
        "//dmlab2d/lib/util:files",
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
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:class",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/system/tensor:tensor_view",
        "//dmlab2d/lib/system/tensor/lua:tensor",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/container:node_hash_map",
        "@com_google_absl//absl/log",
    ],
)

cc_library(
    name = "episode",
    srcs = ["episode.cc"],
    hdrs = ["episode.h"],
    deps = [
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:push",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:stack_resetter",
        "//dmlab2d/lib/lua:table_ref",
        "//third_party/rl_api:env_c_api",
    ],
)

cc_test(
    name = "episode_test",
    srcs = ["episode_test.cc"],
    deps = [
        ":episode",
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:n_results_or_test_util",
        "//dmlab2d/lib/lua:push_script",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/lua:vm_test_util",
        "//third_party/rl_api:env_c_api",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_test(
    name = "events_test",
    srcs = ["events_test.cc"],
    deps = [
        ":events",
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:bind",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:n_results_or_test_util",
        "//dmlab2d/lib/lua:push_script",
        "//dmlab2d/lib/lua:vm",
        "//dmlab2d/lib/lua:vm_test_util",
        "//dmlab2d/lib/system/random/lua:random",
        "//dmlab2d/lib/system/tensor/lua:tensor",
        "//dmlab2d/lib/util:default_read_only_file_system",
        "//dmlab2d/lib/util:file_reader_types",
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
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:push",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:stack_resetter",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/system/tensor/lua:tensor",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
        "@com_google_absl//absl/types:span",
    ],
)

cc_test(
    name = "actions_test",
    srcs = ["actions_test.cc"],
    deps = [
        ":actions",
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:n_results_or_test_util",
        "//dmlab2d/lib/lua:push_script",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/lua:vm_test_util",
        "//third_party/rl_api:env_c_api",
        "@com_google_googletest//:gtest_main",
    ],
)

cc_library(
    name = "observations",
    srcs = ["observations.cc"],
    hdrs = ["observations.h"],
    deps = [
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:class",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:push",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/system/tensor:tensor_view",
        "//dmlab2d/lib/system/tensor/lua:tensor",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "observations_test",
    srcs = ["observations_test.cc"],
    deps = [
        ":observations",
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:bind",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:n_results_or_test_util",
        "//dmlab2d/lib/lua:push_script",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/lua:vm",
        "//dmlab2d/lib/lua:vm_test_util",
        "//dmlab2d/lib/system/random/lua:random",
        "//dmlab2d/lib/system/tensor/lua:tensor",
        "//dmlab2d/lib/util:default_read_only_file_system",
        "//dmlab2d/lib/util:file_reader_types",
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
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:bind",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:n_results_or",
        "//dmlab2d/lib/lua:push",
        "//dmlab2d/lib/lua:read",
        "//dmlab2d/lib/lua:stack_resetter",
        "//dmlab2d/lib/lua:table_ref",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/log",
        "@com_google_absl//absl/log:check",
        "@com_google_absl//absl/strings",
    ],
)

cc_test(
    name = "properties_test",
    srcs = ["properties_test.cc"],
    deps = [
        ":properties",
        "//dmlab2d/lib/lua",
        "//dmlab2d/lib/lua:bind",
        "//dmlab2d/lib/lua:call",
        "//dmlab2d/lib/lua:n_results_or_test_util",
        "//dmlab2d/lib/lua:push_script",
        "//dmlab2d/lib/lua:table_ref",
        "//dmlab2d/lib/lua:vm_test_util",
        "//third_party/rl_api:env_c_api",
        "@com_google_absl//absl/strings",
        "@com_google_googletest//:gtest_main",
    ],
)
