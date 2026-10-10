# Generalizing Path ORAM with Non-Uniform Bucket Sizes: Theoretical and Practical Improvements

There are two programs in this package:

- ORAMEmulator : A Path ORAM implementation for our experiments.
- BoundFinder : A tool for enumerating the parameter `t` to find theoretical bounds.

## authors

- Syaheer Altaf (National Tsing Hua University)
- Hsin-Pei Han (National Tsing Hua University)
- Wing-Kai Hon (National Tsing Hua University)
- Wang-Yang Li (National Tsing Hua University)

## Quick start with Docker

### Prerequisites

From this repository root, build the image:

```sh
docker build --pull --tag lporam-ae:latest .
```

Run the complete reduced-size validation and retain its results on the host:

```sh
mkdir -p artifact-output
docker run --rm --init \
  --volume "$(pwd)/artifact-output:/artifact/artifact-output" \
  lporam-ae:latest quick
```

A successful run ends with:

```text
PASS: LPORAM quick artifact workflow completed successfully.
```

The quick run is a smoke test of every stage. It uses 1,000,000 measured
accesses per emulator variant.

The output layout is:

```text
artifact-output/quick/
|-- bound-finder/
|   |-- bound_finder
|   `-- table.tex
`-- oram-emulator/
    |-- logs/
    |   |-- uniform-z/       # z3.log ... z6.log
    |   |-- single-layer/    # s0.log ... s14.log
    |   `-- leaf-size/       # leaf0.log ... leaf3.log
    `-- reproduced/
        |-- figZ3-6_rate.png
        |-- figZ45-Zmid_rate.png
        |-- figZ5sL0-4_rate.png
        `-- tableZ5x0-14_counts.png
```

## Artifact-to-result map

| Component or dataset | Role                                                         | Produced result                                                     |
| -------------------- | ------------------------------------------------------------ | ------------------------------------------------------------------- |
| `BoundFinder`        | Enumerates feasible `t` values for each bucket configuration | LaTeX theoretical-bound table                                       |
| `uniform-z`          | Uniform `Z = 3, 4, 5, 6`                                     | `figZ3-6_rate.png`; Z=4 and Z=5 baselines                           |
| `single-layer`       | `Z = 5`, with one layer `S = 0..14` reduced to 4             | `figZ45-Zmid_rate.png`, `tableZ5x0-14_counts.png`; leaf-size-4 data |
| `leaf-size`          | Internal `Z = 5`, leaf capacity 0, 1, 2, or 3                | Part of `figZ5sL0-4_rate.png`                                       |

The leaf-capacity figure reuses `single-layer/s14.log` for capacity 4 and
`uniform-z/z5.log` for capacity 5. This prevents duplicate paper-scale runs.
The four images under `ORAMEmulator/img/` are author-supplied paper references;
new results are written under `artifact-output`, never over those references.

## Quick and full profiles

| Profile | Correctness operations | Warm-up per variant | Measurements per variant | Intended use                            |
| ------- | ---------------------: | ------------------: | -----------------------: | --------------------------------------- |
| `quick` |              1,000,000 |             100,000 |                1,000,000 | Complete pipeline and format validation |
| `full`  |             10,000,000 |       1,000,000,000 |           10,000,000,000 | Paper-scale result reproduction         |

Both profiles use `N = 2^15 = 32768`, layers `0..14`, seed `0`, uniformly
random warm-up addresses, and deterministic round-robin measurement addresses.
Each log records the effective parameters.

Quick correctness and speed tests each perform 1,000,000 operations. The
top-level quick runner uses the serial executable. The full runner first runs
the quick self-test (1,000,000 operations), then the full datasets; the
10,000,000 correctness count above applies when invoking the executable's
full self-test directly.

The full profile performs billions of accesses and may take hours or days. It
is guarded against accidental execution. After planning sufficient compute and
storage, run:

```sh
docker run --rm --init \
  --volume "$(pwd)/artifact-output:/artifact/artifact-output" \
  lporam-ae:latest full --confirm-full
```

Set `JOBS` to control compilation parallelism, for example `--env JOBS=8`.
Inside the container, the full profile uses the OpenMP build for independent
variants within each dataset. The `uniform-z`, `single-layer`, and `leaf-size`
datasets still run sequentially in that order. Thread count can be limited with
`--env OMP_NUM_THREADS=8`; `JOBS` does not control experiment threads.

## Native execution (without Docker)

Requirements are a POSIX shell, GNU Make, `diff`, a C++20 compiler, Python
3.9+, and Matplotlib. Install the Python dependency, then use the same runner:

```sh
python3 -m pip install -r ORAMEmulator/requirements.txt
./runme.sh quick
```

To place outputs elsewhere:

```sh
ARTIFACT_OUTPUT_DIR=/path/to/results JOBS=8 ./runme.sh quick
```

Component-specific details and individual experiment commands are documented
in `BoundFinder/README.md` and `ORAMEmulator/README.md`.

## Source inventory

- `Dockerfile`: Debian-based reproducible build and runtime environment.
- `runme.sh`: guarded top-level build, execution, collection, and verification.
- `BoundFinder/`: theoretical-bound source, reference table, and local runner.
- `ORAMEmulator/`: C++ simulator, plotting code, vendored spdlog, reference
  images, and component-level runners.

## Troubleshooting

- **The container is killed:** increase Docker's memory allocation to at least
  4 GiB or lower OpenMP concurrency for a full run.
- **Full run is refused:** the exact second argument `--confirm-full` is
  intentionally required.
