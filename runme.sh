#!/usr/bin/env sh
set -eu

SCRIPT_DIR=$(CDPATH= cd -- "$(dirname -- "$0")" && pwd)
PROFILE=${1:-quick}
CONFIRM=${2:-}
JOBS=${JOBS:-4}
OUTPUT_ROOT=${ARTIFACT_OUTPUT_DIR:-"$SCRIPT_DIR/artifact-output/$PROFILE"}

if [ "$#" -gt 2 ]; then
    echo "Too many arguments." >&2
    exit 2
fi

usage() {
    cat <<'EOF'
Usage:
  ./runme.sh [quick]
  ./runme.sh full --confirm-full

Environment variables:
  ARTIFACT_OUTPUT_DIR  Output directory (default: artifact-output/PROFILE)
  JOBS                 Parallel compiler jobs (default: 4)

The quick profile validates the complete build, experiment, data, and plotting
pipeline with reduced sample counts. It does not reproduce paper-scale
statistics. The full profile may take hours or days.
EOF
}

case "$PROFILE" in
    -h|--help)
        usage
        exit 0
        ;;
    quick)
        if [ -n "$CONFIRM" ]; then
            echo "Unexpected argument for quick profile: $CONFIRM" >&2
            usage >&2
            exit 2
        fi
        ;;
    full)
        if [ "$CONFIRM" != "--confirm-full" ]; then
            echo "Refusing full run. Use: ./runme.sh full --confirm-full" >&2
            exit 2
        fi
        ;;
    *)
        echo "Unknown profile '$PROFILE'; expected quick or full." >&2
        usage >&2
        exit 2
        ;;
esac

require_nonempty_file() {
    if [ ! -s "$1" ]; then
        echo "Verification failed: missing or empty file: $1" >&2
        exit 1
    fi
}

require_log() {
    log_path="$ORAM_LOG_ROOT/$1/$2.log"
    require_nonempty_file "$log_path"
    grep -q "metadata dataset=$1 variant=$2 N=32768 seed=0" "$log_path" || {
        echo "Verification failed: metadata mismatch in $log_path" >&2
        exit 1
    }
    grep -q "Stash size distribution finished:" "$log_path" || {
        echo "Verification failed: incomplete histogram in $log_path" >&2
        exit 1
    }
}

mkdir -p "$OUTPUT_ROOT"

echo "============================================================"
echo "LPORAM artifact evaluation: profile=$PROFILE"
echo "Output directory: $OUTPUT_ROOT"
echo "============================================================"

echo "[1/4] Build, run, and verify BoundFinder"
BOUND_OUTPUT="$OUTPUT_ROOT/bound-finder"
BOUND_FINDER_BUILD_DIR="$BOUND_OUTPUT" \
    "$SCRIPT_DIR/BoundFinder/runme.sh"

echo "[2/4] Build and run all ORAMEmulator datasets"
ORAM_OUTPUT="$OUTPUT_ROOT/oram-emulator"
OUTPUT_ROOT="$ORAM_OUTPUT" JOBS="$JOBS" \
    "$SCRIPT_DIR/ORAMEmulator/runme.sh" "$PROFILE" "$CONFIRM"

echo "[3/4] Verify generated data and figures"
require_nonempty_file "$BOUND_OUTPUT/table.tex"

ORAM_LOG_ROOT="$ORAM_OUTPUT/logs"
for variant in z3 z4 z5 z6; do
    require_log uniform-z "$variant"
done
for layer in 0 1 2 3 4 5 6 7 8 9 10 11 12 13 14; do
    require_log single-layer "s$layer"
done
for capacity in 0 1 2 3; do
    require_log leaf-size "leaf$capacity"
done

FIGURE_ROOT="$ORAM_OUTPUT/reproduced"
for figure in \
    figZ3-6_rate.png \
    figZ45-Zmid_rate.png \
    figZ5sL0-4_rate.png \
    tableZ5x0-14_counts.png; do
    require_nonempty_file "$FIGURE_ROOT/$figure"
done

echo "[4/4] Summarize verified outputs"
echo "  exact reference table: 1"
echo "  completed histogram logs: 23"
echo "  generated figures: 4"
echo "PASS: LPORAM $PROFILE artifact workflow completed successfully."
echo "Results: $OUTPUT_ROOT"
