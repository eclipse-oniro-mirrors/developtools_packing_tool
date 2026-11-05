#!/usr/bin/env bash
# Copyright (c) 2026-2026 Huawei Device Co., Ltd.
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

# Build the Java packing tool jars for CI/build pipelines.
# Wraps build.py with sensible defaults so it runs without the GN/hb build system.
#
# Produces in <output-dir>/<jar-subdir>/:
#   app_packing_tool.jar
#   app_unpacking_tool.jar
#   app_check_tool.jar
#   haptobin_tool.jar
#
# Usage:
#   ./build.sh                              # build all jars with defaults
#   ./build.sh -o /tmp/out                  # custom output directory
#   ./build.sh -c sdk                       # compile target: sdk (default: image)
#   ./build.sh -t linux_x86                 # custom toolchain name
#   ./build.sh -s libs                      # custom jar subdir (default: jar)
#   OUTPUT_DIR=/out ./build.sh              # override via env var

set -euo pipefail

script_dir=$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)

output_dir="${OUTPUT_DIR:-${script_dir}/build_output}"
jar_subdir="${JAR_SUBDIR:-jar}"
toolchain="${TOOLCHAIN:-linux}"
compile_target="${COMPILE_TARGET:-image}"

usage() {
    cat <<EOF
Usage: ./build.sh [OPTIONS]

Build the Java packing tool jars (app_packing_tool.jar,
app_unpacking_tool.jar, app_check_tool.jar, haptobin_tool.jar)
by delegating to build.py — the single source of truth for
Java source file lists.

Options:
  -o, --output-dir DIR      Output directory (default: ./build_output)
  -t, --toolchain NAME      Toolchain name (default: linux)
  -c, --compile-target T    Compile target: image|sdk (default: image)
  -s, --jar-subdir NAME     Jar subdirectory under output-dir (default: jar)
  -h, --help                Show this help and exit

Environment variables (lower priority than CLI options):
  OUTPUT_DIR                Equivalent to --output-dir
  TOOLCHAIN                 Equivalent to --toolchain
  COMPILE_TARGET            Equivalent to --compile-target
  JAR_SUBDIR                Equivalent to --jar-subdir

Prerequisites:
  - python3 (to run build.py)
  - javac / jar (JDK 8 or higher)
  - prebuilts/packing_tool dependency jars (relative to OpenHarmony source root)
EOF
}

while [ $# -gt 0 ]; do
    case "$1" in
        -o|--output-dir)
            output_dir="$2"
            shift 2
            ;;
        -t|--toolchain)
            toolchain="$2"
            shift 2
            ;;
        -c|--compile-target)
            compile_target="$2"
            shift 2
            ;;
        -s|--jar-subdir)
            jar_subdir="$2"
            shift 2
            ;;
        -h|--help)
            usage
            exit 0
            ;;
        *)
            echo "build.sh: unknown option: $1" >&2
            usage >&2
            exit 1
            ;;
    esac
done

case "${compile_target}" in
    image|sdk) ;;
    *)
        echo "build.sh: invalid compile-target '${compile_target}' (expected: image|sdk)" >&2
        exit 1
        ;;
esac

mkdir -p "${output_dir}"
output_dir=$(cd "${output_dir}" && pwd)

echo "=== Java Packing Tool Build ==="
echo "  output_dir:     ${output_dir}"
echo "  jar_subdir:     ${jar_subdir}"
echo "  toolchain:      ${toolchain}"
echo "  compile_target: ${compile_target}"
echo "  script_dir:     ${script_dir}"
echo

# Run build.py from the output directory so that the inner shell scripts
# (packingTool.sh etc.) resolve final_path=$(pwd) to ${output_dir}.
# All jar paths share the same ${jar_subdir} prefix: build.py's --outpath
# is used by the shell scripts to mkdir the parent directory, while the
# *Output args locate the individual jar files — they must stay consistent.
cd "${output_dir}"

python3 "${script_dir}/build.py" \
    --haptobin "${script_dir}/adapter/ohos" \
    --haptobinOutput "${jar_subdir}/haptobin_tool.jar" \
    --unpackOutput "${jar_subdir}/app_unpacking_tool.jar" \
    --packOutput "${jar_subdir}/app_packing_tool.jar" \
    --outpath "${jar_subdir}" \
    --checkOutput "${jar_subdir}/app_check_tool.jar" \
    --toolchain "${toolchain}" \
    --compileTarget "${compile_target}"

echo
echo "=== Build complete ==="
echo "Jars produced in ${output_dir}/${jar_subdir}/:"
ls -la "${output_dir}/${jar_subdir}/"*.jar 2>/dev/null || {
    echo "  WARNING: no jars found — check build output above for errors" >&2
    exit 1
}
