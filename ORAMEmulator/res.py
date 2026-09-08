#!/usr/bin/env python3
"""Rebuild the four paper figures from ORAM histogram logs."""

import argparse
import re
import sys
from pathlib import Path

import matplotlib

matplotlib.use("Agg")
import matplotlib.pyplot as plt


HISTOGRAM_LINE = re.compile(r"Stash size (\d+):\s+(\d+)\s+times")


def read_final_histogram(path: Path) -> dict[int, int]:
    """Read the last stash-distribution section from a spdlog file."""
    histogram: dict[int, int] = {}
    saw_header = False

    with path.open("r", encoding="utf-8") as source:
        for line in source:
            if "Stash size distribution" in line:
                histogram = {}
                saw_header = True
                continue
            if saw_header and (match := HISTOGRAM_LINE.search(line)):
                histogram[int(match.group(1))] = int(match.group(2))

    if not histogram:
        raise ValueError(f"no final stash histogram found in {path}")
    return histogram


def plot_probability(series: list[tuple[str, Path]], title: str,
                     output: Path) -> None:
    plt.figure(figsize=(10, 6))
    for label, path in series:
        histogram = read_final_histogram(path)
        total = sum(histogram.values())
        if total <= 0:
            raise ValueError(f"histogram has no observations: {path}")
        x_values = sorted(histogram)
        probabilities = [histogram[x] / total for x in x_values]
        plt.plot(x_values, probabilities, marker="o", label=label, alpha=0.8)

    plt.yscale("log")
    if title:
        plt.title(title, fontsize=24)
    plt.xlabel("Stash Size", fontsize=16)
    plt.ylabel("Probability (Log Scale)", fontsize=16)
    plt.xlim(left=0, right=25)
    plt.grid(True, which="both", linestyle="--", alpha=0.5)
    plt.legend()
    plt.tight_layout()
    output.parent.mkdir(parents=True, exist_ok=True)
    plt.savefig(output, dpi=300)
    plt.close()
    print(f"wrote {output}")


def figure_specs(log_root: Path) -> dict[str, tuple[str, list[tuple[str, Path]]]]:
    uniform = log_root / "uniform-z"
    layers = log_root / "single-layer"
    leaves = log_root / "leaf-size"

    return {
        "figZ3-6_rate.png": (
            "Stash Distribution for Uniform Z (Probability)",
            [(f"Z={z}", uniform / f"z{z}.log") for z in range(3, 7)],
        ),
        "figZ45-Zmid_rate.png": (
            "",
            [
                ("Z=4", uniform / "z4.log"),
                (r"$Z=5,\ S=\{0\}$", layers / "s0.log"),
                (r"$Z=5,\ S=\{14\}$", layers / "s14.log"),
                ("Z=5", uniform / "z5.log"),
            ],
        ),
        "figZ5sL0-4_rate.png": (
            "Z=5, Different Leaf Sizes",
            [
                *[(f"Leaf size={z}", leaves / f"leaf{z}.log")
                  for z in range(4)],
                ("Leaf size=4", layers / "s14.log"),
                ("Z=5 Uniform", uniform / "z5.log"),
            ],
        ),
        "tableZ5x0-14_counts.png": (
            "",
            [
                ("Z=4", uniform / "z4.log"),
                *[(f"S={layer}", layers / f"s{layer}.log")
                  for layer in range(15)],
                ("Z=5", uniform / "z5.log"),
            ],
        ),
    }


def parse_args() -> argparse.Namespace:
    parser = argparse.ArgumentParser(
        description="Generate paper figures from ORAMEmulator logs.")
    parser.add_argument("--logs-root", type=Path, default=Path("logs"))
    parser.add_argument("--output-dir", type=Path, default=Path("reproduced"))
    parser.add_argument(
        "--figure", default="all",
        choices=["all", "figZ3-6_rate.png", "figZ45-Zmid_rate.png",
                 "figZ5sL0-4_rate.png", "tableZ5x0-14_counts.png"],
    )
    return parser.parse_args()


def main() -> int:
    args = parse_args()
    specs = figure_specs(args.logs_root)
    selected = specs if args.figure == "all" else {args.figure: specs[args.figure]}

    try:
        for filename, (title, series) in selected.items():
            plot_probability(series, title, args.output_dir / filename)
    except (FileNotFoundError, ValueError) as error:
        print(f"error: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
