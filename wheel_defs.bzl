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

"""Creates Python wheel file."""

def _fix(s):
    if s.startswith("dmlab2d/lib/game_scripts"):
        return "dmlab2d/org_deepmind_lab2d/{}".format(s)
    else:
        return s

def _py_wheel_impl(ctx):
    runfiles = [s[DefaultInfo].default_runfiles for s in ctx.attr.deps]
    file_inputs = [f for r in runfiles for f in r.files.to_list()]

    outfile = ctx.actions.declare_file("-".join([
        ctx.attr.distribution,
        ctx.attr.version,
        ctx.attr.python_tag,
        ctx.attr.abi,
        ctx.attr.platform,
    ]) + ".whl")

    ctx.actions.run(
        inputs = file_inputs,
        outputs = [outfile],
        arguments = [
            "--name={}".format(ctx.attr.distribution),
            "--version={}".format(ctx.attr.version),
            "--python_tag={}".format(ctx.attr.python_tag),
            "--abi={}".format(ctx.attr.abi),
            "--platform={}".format(ctx.attr.platform),
            "--out={}".format(outfile.path),
            "--requires=dm-env",
            "--extra_requires=pygame;ui_renderer",
        ] + ["--input_file={p};{s}".format(p = _fix(f.short_path), s = f.path) for f in file_inputs if not f.short_path.startswith("..")],
        executable = ctx.executable._wheelmaker,
        progress_message = "Building wheel",
    )

    return [DefaultInfo(
        data_runfiles = ctx.runfiles(files = [outfile]),
        files = depset([outfile]),
    )]

py_wheel = rule(
    attrs = {
        "abi": attr.string(default = "none"),
        "distribution": attr.string(mandatory = True),
        "platform": attr.string(default = "any"),
        "python_tag": attr.string(
            values = [
                "py2",
                "py3",
            ],
            mandatory = True,
        ),
        "version": attr.string(mandatory = True),
        "deps": attr.label_list(),
        "_wheelmaker": attr.label(
            executable = True,
            cfg = "host",
            default = "@rules_python//tools:wheelmaker",
        ),
    },
    implementation = _py_wheel_impl,
)
