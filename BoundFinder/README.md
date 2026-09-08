# BoundFinder

BoundFinder enumerates the parameter `t` in the theoretical bound used by the
paper and emits the resulting table as LaTeX.

## Artifact contents

- `finder.cpp`: the bound computation and the configurations evaluated in the
  paper.
- `runme.sh`: one-command build, execution, and output verification for Linux
  and macOS.
- `runme.ps1`: equivalent entry point for Windows PowerShell.
- `expected/table.tex`: reference output.

## Requirements

- A C++17 compiler
- On Linux/macOS: a POSIX shell and `diff`.
- On Windows: PowerShell and a C++17 compiler available as `c++.exe`.

## Reproduce the table

```sh
./runme.sh
```

```

The script compiles `finder.cpp`, writes the reproduced table to
`build/table.tex`, and checks it against `expected/table.tex`. A successful
run ends with `Verification passed`. The complete run normally takes only a
few seconds on a commodity CPU.

## Method and output interpretation

For each listed bucket configuration `Z`, the program scans
`0 <= t < ln(2)` at increments of `10^-6`. It checks the feasibility
conditions implemented by `solver(Z, t)` and reports:

1. the weighted average bucket capacity `avg.Z`;
2. the minimum and maximum feasible values of `t`; and
3. a bound of the form `C * (e^-t)^R`.

The reported value of `t` used for the bound is the largest feasible value
rounded down to three decimal places (and never below the minimum feasible
value). `divergent` means that no feasible value was found on the scan grid.
The generated file is a LaTeX table fragment rather than a complete document.

The configurations used for the paper are the `Cases` vector in `main()`.
Changing that vector is useful for exploration, but the reference-output check
is expected to fail after such a change until the new result has been reviewed.
```
