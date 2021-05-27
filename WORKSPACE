workspace(name = "org_deepmind_lab2d")

load("@bazel_tools//tools/build_defs/repo:http.bzl", "http_archive")

http_archive(
    name = "com_google_googletest",
    strip_prefix = "googletest-master",
    urls = ["https://github.com/google/googletest/archive/master.zip"],
)

http_archive(
    name = "com_google_benchmark",
    strip_prefix = "benchmark-master",
    urls = ["https://github.com/google/benchmark/archive/master.zip"],
)

http_archive(
    name = "rules_cc",
    strip_prefix = "rules_cc-master",
    urls = ["https://github.com/bazelbuild/rules_cc/archive/master.zip"],
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
    strip_prefix = "abseil-py-master",
    urls = ["https://github.com/abseil/abseil-py/archive/master.zip"],
)

http_archive(
    name = "enum34_archive",
    build_file = "@com_google_absl_py//third_party:enum34.BUILD",
    sha256 = "8ad8c4783bf61ded74527bffb48ed9b54166685e4230386a9ed9b1279e2df5b1",
    urls = [
        "https://mirror.bazel.build/pypi.python.org/packages/bf/3e/31d502c25302814a7c2f1d3959d2a3b3f78e509002ba91aea64993936876/enum34-1.1.6.tar.gz",
        "https://pypi.python.org/packages/bf/3e/31d502c25302814a7c2f1d3959d2a3b3f78e509002ba91aea64993936876/enum34-1.1.6.tar.gz",
    ],
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
    sha256 = "9a01fed6311df359f3f9af119fcf298a3353aef7d1b1bc86f6c8ae0ca6a2f842",
    strip_prefix = "/eigen-eigen-5d5dd50b2eb6",
    urls = [
        "https://mirror.bazel.build/bitbucket.org/eigen/eigen/get/5d5dd50b2eb6.zip",
        "https://bitbucket.org/eigen/eigen/get/5d5dd50b2eb6.zip",
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
    name = "six_archive",
    build_file = "@//bazel:six.BUILD",
    sha256 = "105f8d68616f8248e24bf0e9372ef04d3cc10104f1980f54d57b2ce73a5ad56a",
    strip_prefix = "six-1.10.0",
    urls = [
        "https://mirror.bazel.build/pypi.python.org/packages/source/s/six/six-1.10.0.tar.gz",
        "https://pypi.python.org/packages/source/s/six/six-1.10.0.tar.gz",
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
    strip_prefix = "dm_env-master",
    urls = ["https://github.com/deepmind/dm_env/archive/master.zip"],
)

http_archive(
    name = "tree_archive",
    repo_mapping = {
        "@python_headers": "@python_system",
    },
    strip_prefix = "tree-master",
    urls = ["https://github.com/deepmind/tree/archive/master.zip"],
)

http_archive(
    name = "pybind11_archive",
    build_file = "@tree_archive//external:pybind11.BUILD",
    strip_prefix = "pybind11-master",
    urls = ["https://github.com/pybind/pybind11/archive/master.zip"],
)

new_local_repository(
    name = "python_system",
    build_file = "@//bazel:python.BUILD",
    path = "/",
)
