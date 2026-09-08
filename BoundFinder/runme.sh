#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD_DIR=${BOUND_FINDER_BUILD_DIR:-"$SCRIPT_DIR/build"}
CXX=${CXX:-c++}

mkdir -p "$BUILD_DIR"

echo "[1/3] Compiling BoundFinder with $CXX"
"$CXX" -std=c++17 -O2 -Wall -Wextra -Wpedantic \
    "$SCRIPT_DIR/finder.cpp" -o "$BUILD_DIR/bound_finder"

echo "[2/3] Generating the LaTeX table"
"$BUILD_DIR/bound_finder" > "$BUILD_DIR/table.tex"

echo "[3/3] Comparing against the reference output"
diff -u "$SCRIPT_DIR/expected/table.tex" "$BUILD_DIR/table.tex"

cat "$BUILD_DIR/table.tex"
echo "Verification passed: $BUILD_DIR/table.tex matches expected/table.tex"

