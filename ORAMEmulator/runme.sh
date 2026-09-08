#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROFILE=${1:-quick}
CONFIRM=${2:-}
JOBS=${JOBS:-4}
OUTPUT_ROOT=${OUTPUT_ROOT:-"$SCRIPT_DIR/artifact-output/$PROFILE"}

case "$PROFILE" in
    quick)
        make -C "$SCRIPT_DIR" -j"$JOBS" all
        EXECUTABLE="$SCRIPT_DIR/build/oram-emulator.exe"
        "$EXECUTABLE" --experiment all --profile quick \
            --output "$OUTPUT_ROOT/logs"
        ;;
    full)
        if [ "$CONFIRM" != "--confirm-full" ]; then
            echo "Refusing full run. Use: ./runme.sh full --confirm-full" >&2
            exit 2
        fi
        make -C "$SCRIPT_DIR" -j"$JOBS" omp
        EXECUTABLE="$SCRIPT_DIR/build/oram-emulator-omp.exe"
        "$EXECUTABLE" --experiment self-test --profile quick \
            --output "$OUTPUT_ROOT/logs"
        for dataset in uniform-z single-layer leaf-size; do
            "$EXECUTABLE" --experiment "$dataset" --profile full \
                --confirm-full --output "$OUTPUT_ROOT/logs"
        done
        ;;
    *)
        echo "Unknown profile '$PROFILE'; expected quick or full." >&2
        exit 2
        ;;
esac

if [ -n "${PYTHON:-}" ]; then
    PYTHON_CMD=$PYTHON
elif command -v python3 >/dev/null 2>&1; then
    PYTHON_CMD=python3
elif command -v python >/dev/null 2>&1; then
    PYTHON_CMD=python
elif command -v py >/dev/null 2>&1; then
    PYTHON_CMD=py
else
    echo "Python not found. Install Python and run: pip install -r requirements.txt" >&2
    exit 1
fi

"$PYTHON_CMD" -c "import matplotlib" 2>/dev/null || {
    echo "Matplotlib not found. Run: $PYTHON_CMD -m pip install -r requirements.txt" >&2
    exit 1
}
"$PYTHON_CMD" "$SCRIPT_DIR/res.py" --logs-root "$OUTPUT_ROOT/logs" \
    --output-dir "$OUTPUT_ROOT/reproduced"

echo "Artifact run completed: $OUTPUT_ROOT"
