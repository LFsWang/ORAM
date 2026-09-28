#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
BUILD_DIR=${BOUND_FINDER_BUILD_DIR:-"$SCRIPT_DIR/build"}
CXX=${CXX:-c++}
PYTHON=${PYTHON:-python3}

mkdir -p "$BUILD_DIR"

echo "[1/4] Compiling BoundFinder with $CXX"
"$CXX" -std=c++17 -O2 -Wall -Wextra -Wpedantic \
    "$SCRIPT_DIR/finder.cpp" -o "$BUILD_DIR/bound_finder"

echo "[2/4] Generating the LaTeX table"
"$BUILD_DIR/bound_finder" > "$BUILD_DIR/table.tex"

echo "[3/4] Comparing against the reference output"
diff -u "$SCRIPT_DIR/expected/table.tex" "$BUILD_DIR/table.tex"

cat "$BUILD_DIR/table.tex"

# Show the intermediate calculation for Theorem 4.1, Equations (4.13)-(4.15).
echo "[4/4] Computing the intermediate values for Theorem 4.1"
"$PYTHON" "$SCRIPT_DIR/ProofTheorem4.1/main.py" > "$BUILD_DIR/theorem4.1.txt"
cat "$BUILD_DIR/theorem4.1.txt"
echo "Intermediate values saved to: $BUILD_DIR/theorem4.1.txt"
echo "Verification passed: $BUILD_DIR/table.tex matches expected/table.tex"
