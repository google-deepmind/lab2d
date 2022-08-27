workspace(name = "org_deepmind_lab2d")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")
load("@//:python_system.bzl", "python_repo")

http_archive(
    name = "com_google_googletest",
    strip_prefix = "googletest-main",
    urls = ["https://github.com/google/googletest/archive/main.zip"],
)

http_archive(
    name = "com_google_benchmark",
    strip_prefix = "benchmark-main",
    urls = ["https://github.com/google/benchmark/archive/main.zip"],
)

http_archive(
    name = "rules_cc",
    strip_prefix = "rules_cc-main",
    urls = ["https://github.com/bazelbuild/rules_cc/archive/main.zip"],
)

http_archive(
    name = "rules_python",
    strip_prefix = "rules_python-main",
    url = "https://github.com/bazelbuild/rules_python/archive/main.zip",
)

http_archive(
    name = "bazel_skylib",
    strip_prefix = "bazel-skylib-main",
    urls = ["https://github.com/bazelbuild/bazel-skylib/archive/main.zip"],
)

http_archive(
    name = "com_google_absl",
    strip_prefix = "abseil-cpp-master",
    urls = ["https://github.com/abseil/abseil-cpp/archive/master.zip"],
)

http_archive(
    name = "com_google_absl_py",
    strip_prefix = "abseil-py-main",
    urls = ["https://github.com/abseil/abseil-py/archive/main.zip"],
)

http_archive(
    name = "funcsigs_archive",
    build_file = "@//bazel:funcsigs.BUILD",
    strip_prefix = "funcsigs-1.0.2",
    urls = [
        "https://pypi.python.org/packages/94/4a/db842e7a0545de1cdb0439bb80e6e42dfe82aaeaadd4072f2263a4fbed23/funcsigs-1.0.2.tar.gz",
    ],
)

http_archive(
    name = "eigen_archive",
    build_file = "@//bazel:eigen.BUILD",
    sha256 = "515b3c266d798f3a112efe781dda0cf1aef7bd73f6864d8f4f16129310ae1fdf",
    strip_prefix = "eigen-b02c384ef4e8eba7b8bdef16f9dc6f8f4d6a6b2b",
    urls = [
        "https://gitlab.com/libeigen/eigen/-/archive/b02c384ef4e8eba7b8bdef16f9dc6f8f4d6a6b2b/eigen-b02c384ef4e8eba7b8bdef16f9dc6f8f4d6a6b2b.tar.gz",
        "https://storage.googleapis.com/mirror.tensorflow.org/gitlab.com/libeigen/eigen/-/archive/b02c384ef4e8eba7b8bdef16f9dc6f8f4d6a6b2b/eigen-b02c384ef4e8eba7b8bdef16f9dc6f8f4d6a6b2b.tar.gz",
    ],
)

http_archive(
    name = "png_archive",
    build_file = "@//bazel:png.BUILD",
    sha256 = "c2c50c13a727af73ecd3fc0167d78592cf5e0bca9611058ca414b6493339c784",
    strip_prefix = "libpng-1.6.37",
    urls = [
        "https://mirror.bazel.build/github.com/glennrp/libpng/archive/v1.6.37.zip",
        "https://github.com/glennrp/libpng/archive/v1.6.37.zip",
    ],
)

http_archive(
    name = "zlib_archive",
    build_file = "@//bazel:zlib.BUILD",
    sha256 = "c3e5e9fdd5004dcb542feda5ee4f0ff0744628baf8ed2dd5d66f8ca1197cb1a1",
    strip_prefix = "zlib-1.2.11",
    urls = [
        "https://mirror.bazel.build/zlib.net/zlib-1.2.11.tar.gz",
        "https://zlib.net/zlib-1.2.11.tar.gz",
    ],
)

http_archive(
    name = "lua5_1_archive",
    build_file = "@//bazel:lua5_1.BUILD",
    sha256 = "2640fc56a795f29d28ef15e13c34a47e223960b0240e8cb0a82d9b0738695333",
    strip_prefix = "lua-5.1.5/src",
    urls = [
        "https://mirror.bazel.build/www.lua.org/ftp/lua-5.1.5.tar.gz",
        "https://www.lua.org/ftp/lua-5.1.5.tar.gz",
    ],
)

http_archive(
    name = "lua5_2_archive",
    build_file = "@//bazel:lua5_2.BUILD",
    strip_prefix = "lua-5.2.4/src",
    urls = [
        "https://mirror.bazel.build/www.lua.org/ftp/lua-5.2.4.tar.gz",
        "https://www.lua.org/ftp/lua-5.2.4.tar.gz",
    ],
)

http_archive(
    name = "luajit_archive",
    build_file = "@//bazel:luajit.BUILD",
    sha256 = "409f7fe570d3c16558e594421c47bdd130238323c9d6fd6c83dedd2aaeb082a8",
    strip_prefix = "LuaJIT-2.1.0-beta3",
    urls = ["https://github.com/LuaJIT/LuaJIT/archive/v2.1.0-beta3.tar.gz"],
)

http_archive(
    name = "dm_env_archive",
    build_file = "@//bazel:dm_env.BUILD",
    strip_prefix = "dm_env-3c6844db2aa4ed5994b2c45dbfd9f31ad948fbb8",
    urls = ["https://github.com/deepmind/dm_env/archive/3c6844db2aa4ed5994b2c45dbfd9f31ad948fbb8.zip"],
)

http_archive(
    name = "tree_archive",
    build_file = "@//bazel:tree.BUILD",
    strip_prefix = "tree-master",
    urls = ["https://github.com/deepmind/tree/archive/master.zip"],
)

http_archive(
    name = "pybind11_archive",
    build_file = "@//bazel:pybind11.BUILD",
    strip_prefix = "pybind11-master",
    urls = ["https://github.com/pybind/pybind11/archive/master.zip"],
)

python_repo(
    name = "python_system",
)
