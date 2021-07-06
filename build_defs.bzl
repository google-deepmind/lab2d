# Copyright 2019 DeepMind Technologies Limited.
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

"""A BUILD rule for Python extensions that are defined via pybind11."""

load("@bazel_skylib//lib:collections.bzl", "collections")

def pybind_extension(
        name,
        deps = [],
        data = [],
        copts = [],
        features = [],
        module_name = None,
        tags = [],
        visibility = None,
        **cc_kwargs):
    """Creates a pybind11-based C++ library to extend Python.

    Args:
      name: A unique name for this target.
      deps: The list of other libraries that the C++ target depends upon.
      data: The list of files needed by this rule at runtime.
      copts: Add these options to the C++ compilation command.
      features: Modify the features currently enabled on the package level via
          the `features` attribute.
      module_name: The extension module's name, if it differs from the rule's name.
          Must match the module name specified in the C++ source code, otherwise
          it cannot be loaded by Python.
      tags: A list of tags to be applied to all generated rules.
      visibility: The visibility of all generated rules.
      **cc_kwargs: Additional keyword arguments that are passed to a generated
          `cc_binary` rule.

    Returns:
      A `py_library` rule.
    """
    pybind11_deps = [
        "@pybind11_archive//:pybind11",
        "@python_system//:python_headers",
    ]
    pybind11_copts = ["-fexceptions"]
    pybind11_features = ["-use_header_modules"]

    if module_name == None:
        module_name = name

    py_kwargs = dict()

    if visibility != None:
        for kwargs in (cc_kwargs, py_kwargs):
            kwargs["visibility"] = visibility

    shared_lib_name = module_name + ".so"
    native.cc_binary(
        name = shared_lib_name,
        deps = collections.uniq(deps + pybind11_deps),
        copts = collections.uniq(copts + pybind11_copts),
        features = collections.uniq(features + pybind11_features),
        tags = tags,
        linkshared = 1,
        linkstatic = 1,
        **cc_kwargs
    )

    return native.py_library(
        name = name,
        data = data + [module_name + ".so"],
        tags = tags,
        **py_kwargs
    )

def pytype_strict_library(name, **kwargs):
    native.py_library(name = name, **kwargs)

def pytype_strict_binary(name, **kwargs):
    native.py_binary(name = name, **kwargs)
