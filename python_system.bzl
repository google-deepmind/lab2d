# Copyright 2021 DeepMind Technologies Limited.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#    http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.
# ============================================================================

"""Generates a local repository that points at the system's Python installation."""

_BUILD_FILE = '''# Description:
#   Build rule for Python

exports_files(["defs.bzl"])

cc_library(
    name = "python_headers",
    hdrs = glob(["python3/**/*.h"]),
    includes = ["python3"],
    visibility = ["//visibility:public"],
)
'''

_GET_PYTHON_INCLUDE_DIR = """
import sys
from distutils.sysconfig import get_python_inc
sys.stdout.write(get_python_inc())
""".strip()

_GET_PYTHON_SOABI = """
from packaging import tags
tag = next(iter(tags.sys_tags()))
print(f'PY_TAGS = struct(interpreter = "{tag.interpreter}", abi = "{tag.abi}", platform = "{tag.platform}")')
""".strip()

def _python_repo_impl(repository_ctx):
    """Creates external/<reponame>/BUILD, a python3 symlink, and other files."""

    repository_ctx.file("BUILD", _BUILD_FILE)

    result = repository_ctx.execute(["python3", "-c", _GET_PYTHON_INCLUDE_DIR])
    if result.return_code:
        fail("Failed to run local Python interpreter: %s" % result.stderr)
    repository_ctx.symlink(result.stdout, "python3")

    result = repository_ctx.execute(["python3", "-c", _GET_PYTHON_SOABI])
    if result.return_code:
        fail("Failed to run local Python interpreter: %s" % result.stderr)
    repository_ctx.file("defs.bzl", result.stdout)

python_repo = repository_rule(
    implementation = _python_repo_impl,
    configure = True,
    local = True,
)
