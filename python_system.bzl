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

"""Generates a local repository that exposes Python SOABI tags."""

_GET_PYTHON_SOABI = """
import os
from packaging import tags
tag = next(iter(tags.sys_tags()))
env_key = "PY_PLATFORM_OVERRIDE"
print(f'PY_TAGS = struct(interpreter = "{tag.interpreter}", abi = "{tag.abi}", platform = "{os.environ.get(env_key, tag.platform)}")')
""".strip()

def _python_repo_impl(repository_ctx):
    """Creates a package containing defs.bzl."""

    python3 = repository_ctx.which("python3")
    result = repository_ctx.execute([python3, "-c", _GET_PYTHON_SOABI])
    if result.return_code:
        fail("Failed to run local Python interpreter: %s" % result.stderr)
    repository_ctx.file("BUILD", "")
    repository_ctx.file("defs.bzl", result.stdout)

python_repo = repository_rule(
    implementation = _python_repo_impl,
    configure = True,
    local = True,
)
