# ORAMEmulator artifact

ORAMEmulator evaluates the stash-size distribution of non-recursive Path ORAM
with a configurable bucket capacity `Z(layer)`. The artifact separates the
simulation into three non-overlapping datasets and derives four paper figures
from those datasets.

The reference paper images supplied by the authors are in `img/`. Full raw
logs are not yet included; quick logs are intended only to validate the build
and analysis pipeline, not the paper's statistical claims.

## Experimental setup

Both profiles use the paper's structural parameters:

- ORAM capacity: `N = 2^15 = 32768` blocks.
- Tree height: `L' = ceil(log2(N)) - 1 = 14`, with layers numbered `0..14`.
- Initialization: insert every logical block once.
- Warm-up workload: uniformly random addresses.
- Measurement workload: deterministic round-robin addresses.
- PRNG seed: `0` by default and recorded in every log.

The paper describes logical addresses as `1..N`; the implementation uses the
equivalent zero-based representation `0..N-1`.

| Profile | Correctness operations | Warm-up per variant | Measurement per variant | Purpose                  |
| ------- | ---------------------: | ------------------: | ----------------------: | ------------------------ |
| `quick` |                100,000 |              10,000 |                 100,000 | Pipeline smoke test only |
| `full`  |             10,000,000 |       1,000,000,000 |          10,000,000,000 | Paper-scale evaluation   |

The full profile can take hours or days. It is protected by a second explicit
`--confirm-full` flag and is never selected by default.

## Dataset and figure mapping

| Dataset command | Variants generated                                                        | Used by paper image                                                           |
| --------------- | ------------------------------------------------------------------------- | ----------------------------------------------------------------------------- |
| `uniform-z`     | Uniform `Z=3,4,5,6`                                                       | `figZ3-6_rate.png`; Z=4/Z=5 baselines in two other figures                    |
| `single-layer`  | `Z=5`, except layer `S in {0..14}` has capacity 4                         | `figZ45-Zmid_rate.png`, `tableZ5x0-14_counts.png`; `S=14` is also leaf size 4 |
| `leaf-size`     | Internal layers have capacity 5; leaf layer 14 has capacity 0, 1, 2, or 3 | `figZ5sL0-4_rate.png`                                                         |

For `figZ5sL0-4_rate.png`, leaf size 4 reuses `single-layer/s14.log`, and
leaf size 5 reuses the `uniform-z/z5.log` baseline. This avoids repeating two
paper-scale simulations.

The source image named `tableZ5x0-14_counts.png` plots probabilities despite
the historical `_counts` suffix; the filename is retained to match the paper.

## Build

Requirements:

- GNU Make and a C++20 compiler (GCC 10 or newer recommended).
- OpenMP only for the optional parallel executable.
- Python 3.9+ and Matplotlib for figure generation.

The bundled spdlog is built once as a compiled static library. Application
sources are compiled into separate object files, so incremental builds do not
recompile spdlog.

```bash
# Serial executable: build/oram-emulator.exe
make -j4

# Optional OpenMP executable: build/oram-emulator-omp.exe
make -j4 omp
```

The `.exe` suffix is used consistently on every platform; it does not imply
that the executable requires Windows.

## Safe quick validation

Running the executable without arguments only prints help. To run all reduced
datasets and generate all four smoke-test figures with one command:

```bash
./runme.sh
```

Equivalent explicit commands are:

```bash
build/oram-emulator.exe --experiment self-test --profile quick
build/oram-emulator.exe --experiment uniform-z --profile quick
build/oram-emulator.exe --experiment single-layer --profile quick
build/oram-emulator.exe --experiment leaf-size --profile quick
```

Use `--seed N` to select a recorded seed and `--output DIR` to place logs in a
different directory.

## Full paper-scale execution

Run each non-overlapping dataset once, preferably with the OpenMP executable:

```bash
make omp
build/oram-emulator-omp.exe --experiment uniform-z --profile full --confirm-full
build/oram-emulator-omp.exe --experiment single-layer --profile full --confirm-full
build/oram-emulator-omp.exe --experiment leaf-size --profile full --confirm-full
```

The OpenMP executable runs the independent variants inside each dataset in
parallel. Dataset commands remain sequential, so logs and figures retain the
same grouping as the serial workflow. Set `OMP_NUM_THREADS=N` to control
experiment concurrency; the `JOBS` variable controls compilation only.

These commands are documentation, not part of the quick validation procedure.
The guarded one-command equivalents are `./runme.sh full --confirm-full` and
`./runme.ps1 -Profile full -ConfirmFull`.

## Generate figures

Install the plotting dependency in a virtual environment:

```bash
python3 -m pip install -r requirements.txt
python3 res.py --logs-root logs --output-dir reproduced
```

On Windows, `py` can be used in place of `python3`. `res.py` exits with a
non-zero status if a required log or final histogram is missing. It writes:

- `reproduced/figZ3-6_rate.png`
- `reproduced/figZ45-Zmid_rate.png`
- `reproduced/figZ5sL0-4_rate.png`
- `reproduced/tableZ5x0-14_counts.png`

Quick-profile figures will have short tails because they contain only 100,000
measurements. They verify plumbing and labels, not agreement with the
paper-scale distributions.

## Source layout

- `ORAM/`: header-only ORAM implementation.
- `Experiment/00-selfTest.cpp`: correctness comparison against plain memory.
- `Experiment/01-Zeq3456.cpp`: uniform-Z dataset.
- `Experiment/02-Z45.cpp`: single-layer and leaf-capacity datasets.
- `res.py`: deterministic log parser and four-figure generator.
- `runme.sh`, `runme.ps1`: guarded quick/full orchestration.
- `img/`: reference images used in the paper.
- `logs/`: generated histogram logs.
- `3rd_lib/spdlog-1.15.3/`: vendored logging dependency and license.

`log2csv.py`, `analysisSingle.py`, and `comp.py` are retained legacy analysis
utilities; they are not used by the paper-figure reproduction path above.
