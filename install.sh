#!/bin/bash
#
# Copyright 2023 DeepMind Technologies Limited.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     https://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Installs dmlab2d from source on Linux/macOS.

set -euxo pipefail

function check_version_gt() {
  local required="$1"
  local input lowest
  input="$(grep -Eo '[0-9]+\.[0-9]+' /dev/stdin | head -n 1)"
  lowest="$(printf "${required}\n${input}" | sort -V | head -n 1)"
  [[ ${lowest} == ${required} ]]
}

function check_setup() {
  echo -e "\nChecking OS is Linux or macOS..."
  uname -a
  [[ "$(uname -s)" =~ (Linux|Darwin) ]]

  echo -e "\nChecking python version..."
  python --version | tee /dev/stdout | check_version_gt '3.8'

  echo -e "\nChecking pip version..."
  pip install --upgrade pip
  pip --version | tee /dev/stdout | check_version_gt '20.3'

  echo -e "\nChecking clang version ..."
  clang --version | tee /dev/stdout | check_version_gt '14.0'

  echo -e "\nChecking bazel version..."
  bazel --version | tee /dev/stdout | check_version_gt '6.2'
}

function install_dmlab2d() {
  echo -e "\nInstalling dmlab2d requirements..."
  pip install packaging

  echo -e "\nBuilding dmlab2d wheel..."
  case "$(uname -srp)" in
    Linux*)
      local -r EXTRA_CONFIG=(
          --linkopt=-fuse-ld=lld
      )
      ;;
    'Darwin 22.'*arm)
      local -r EXTRA_CONFIG=(
          --config=libc++
          --config=macos_arm64
          --repo_env=PY_PLATFORM_OVERRIDE=macosx_13_0_arm64
      )
      ;;
    'Darwin 21.'*arm)
      local -r EXTRA_CONFIG=(
          --config=libc++
          --config=macos_arm64
          --repo_env=PY_PLATFORM_OVERRIDE=macosx_12_0_arm64
      )
      ;;
    Darwin*i386)
      local -r EXTRA_CONFIG=(
          --config=libc++
          --config=macos
      )
      ;;
    *)
      echo "ERROR: no supported config for ${platform}..." >&2
      exit 1
      ;;
  esac
  C=clang CXX=clang++ bazel --bazelrc=.bazelrc build \
      --compilation_mode=opt \
      --dynamic_mode=off \
      --config=luajit \
      "${EXTRA_CONFIG[@]}" \
      --subcommands \
      --verbose_failures \
      --experimental_ui_max_stdouterr_bytes=-1 \
      --sandbox_debug \
      //dmlab2d:dmlab2d_wheel

  echo -e "\nInstalling dmlab2d..."
  pip install -vvv --find-links=bazel-bin/dmlab2d dmlab2d
}

function test_dmlab2d() {
  echo -e "\nTesting dmlab2d..."
  python dmlab2d/dmlab2d_test.py
}

function main() {
  check_setup
  install_dmlab2d
  test_dmlab2d
}

main "$@"
