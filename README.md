# Path ORAM with Pluggable Z Function

A C++ implementation of Non-Recursive Path ORAM with configurable per-layer bucket size (Z function). This project benchmarks stash size distributions across various Z function configurations.

## Requirements

- **Compiler**: GCC 10+ (with C++20 support)
- **Optional**: OpenMP (for parallel execution in Test02)
- **Python 3.7+** with `matplotlib` and `numpy` (for result visualization)

## Build

```bash
# Build both single-threaded and OpenMP versions
make

# This produces:
#   main.exe       - single-threaded
#   main-omp.exe   - OpenMP parallel (recommended for Test02)
```

## Usage

```bash
# Run all experiments sequentially (single-threaded)
./main.exe

# Run with OpenMP parallelism (recommended)
./main-omp.exe
```

## Experiments

All experiments are defined in the `Experiment/` directory and invoked from `Experiment/experiment.cpp`.

### Test00 — Correctness and Speed Test

**File**: `Experiment/00-selfTest.cpp`

- Verifies the Path ORAM implementation against a plain memory baseline using 10 million random read/write operations on 32K memory slots.
- Also runs a standalone speed benchmark with the same parameters.
- **Runtime**: A few minutes.

### Test01 — Uniform Z Comparison (Z=3, 4, 5, 6)

**File**: `Experiment/01-Zeq3456.cpp`

- Compares four uniform Z functions (Z=3 through Z=6).
- Warm-up phase: 1 billion random read operations.
- Test phase: 10 billion round-robin read operations, logging stash size distributions. (The original experiment uses 250 billion rounds, estimated ~1 week.)
- Output logs: `logs/Test1/NPathORAM_Uniform*.log`
- **Runtime**: Long (hours to days).

### Test02 — Per-Layer Z Variant Comparison

**File**: `Experiment/02-Z45.cpp`

- Tests 19 Z function variants in parallel (using OpenMP):
  - **Z5s0 ~ Z5s14**: Z=5 at all layers except one layer uses Z=4 (one variant per layer 0-14).
  - **Z3uniform, Z4uniform, Z5uniform, Z6uniform**: Baseline uniform Z functions.
- Warm-up phase: 1 billion random read operations per variant.
- Test phase: 10 billion round-robin read operations per variant.
- Output logs: `logs/Test2/NPathORAM_*.log`
- **Runtime**: Long (hours to days, depending on core count).

## Enabling / Disabling Experiments

Edit `Experiment/experiment.cpp` and comment out the tests you do not need:

```cpp
void RunAllTests() {
    Test00();   // comment out to skip correctness test
    Test01();   // comment out to skip uniform Z long-run
    Test02();   // comment out to skip per-layer Z variants
}
```

For example, to run only the quick self-test:

```cpp
void RunAllTests() {
    Test00();
    // Test01();
    // Test02();
}
```

### Selecting Specific Z Variants within a Test

Inside each test file, the `Zfuncs` array defines which Z function variants to run. You can comment out individual entries to skip specific variants. For example, in `Experiment/02-Z45.cpp`:

```cpp
std::tuple<std::string, NonRecursivePathORAM<uint32_t>::Zfunction> Zfuncs[] = {
    std::make_tuple("Z5s14", Z5s14),
    std::make_tuple("Z5s13", Z5s13),
    // std::make_tuple("Z5s12", Z5s12),  // skip this variant
    // std::make_tuple("Z5s11", Z5s11),  // skip this variant
    ...
    std::make_tuple("Z4uniform", Z4uniform),
    std::make_tuple("Z5uniform", Z5uniform),
};
```

The same approach applies to the `Zfuncs` array in `Experiment/01-Zeq3456.cpp`.

## Visualization

After experiments produce log files, use the Python scripts to generate charts:

```bash
# Generate stash distribution figures and tables
python res.py

# Convert raw log to CSV (stash size vs. opnum)
python log2csv.py

# Compare multiple datasets
python comp.py

# Analyze a single dataset
python analysisSingle.py
```

## Project Structure

```
.
├── 3rd_lib/                 # Third-party libraries
│   └── spdlog-1.15.3/      #   Logging library
├── ORAM/                    # Core ORAM implementation (header-only)
│   ├── Base.h               #   ORAM abstract base class
│   ├── PathOram.h           #   Non-Recursive Path ORAM with pluggable Z
│   ├── PlainMemory.h        #   Plain memory (for correctness testing)
│   ├── TestContent.h        #   Test environment (RNG, logger, stash stats)
│   └── ORAM.h               #   Aggregate header
├── Experiment/              # Experiment definitions
│   ├── experiment.h         #   Declarations
│   ├── experiment.cpp       #   Entry point (enable/disable tests here)
│   ├── 00-selfTest.cpp      #   Test00: correctness + speed
│   ├── 01-Zeq3456.cpp       #   Test01: uniform Z=3,4,5,6
│   └── 02-Z45.cpp           #   Test02: per-layer Z variants
├── main.cpp                 # Program entry point
├── Makefile                 # Build configuration
├── res.py                   # Result visualization (stash distribution plots)
├── log2csv.py               # Log-to-CSV converter
├── comp.py                  # Multi-dataset comparison plots
├── analysisSingle.py        # Single-dataset analysis plot
└── logs/                    # Output directory for experiment logs
    ├── Test1/               #   Test01 raw logs (created at runtime)
    ├── Test2/               #   Test02 raw logs (created at runtime)
    ├── figZ3-6/             #   Visualization output from res.py fig1()
    ├── figZ45-Zmid/         #   Visualization output from res.py fig2()
    └── tableZ5x0-14/        #   Visualization output from res.py tab3()
```
