# BoundFinder

BoundFinder enumerates the parameter `t` in the theoretical bound used by the
paper and emits the resulting table as LaTeX.

## Artifact contents

- `finder.cpp`: the bound computation and the configurations evaluated in the
  paper.
- `ProofTheorem4.1/main.py`: intermediate values for the calculation in
  Theorem 4.1 (Section 4.5).
- `runme.sh`: one-command build, execution, and output verification for Linux
  and macOS.
- `expected/table.tex`: reference output.

## Requirements

- A C++17 compiler
- Python 3 for the Theorem 4.1 calculation (standard library only).
- On Linux/macOS: a POSIX shell and `diff`.
- On Windows: PowerShell and a C++17 compiler available as `c++.exe`.

## Reproduce the table

```sh
./runme.sh
```

The script compiles `finder.cpp`, writes the reproduced table to
`build/table.tex`, and checks it against `expected/table.tex`. It also runs
`ProofTheorem4.1/main.py`, displays the intermediate values, and saves them to
`build/theorem4.1.txt`. Set `PYTHON` to select a different Python 3 executable.
A successful
run ends with `Verification passed`. The complete run normally takes only a
few seconds on a commodity CPU.

## Theorem 4.1 intermediate calculation

The main bound computation is in `finder.cpp`. The small Python program in
[`ProofTheorem4.1`](ProofTheorem4.1/README.md) shows the intermediate steps for
configuration `(4, 4, 4, 4, 5, ...)` at `t = 0.530`. Its output follows
Equations (4.13)–(4.15): `e^(-t)`, the weights from `A` and `B`, the recursive
values `g_L` through `g_{L-3}`, and the fixed point `g_p`, including the check
`g_{L-3} < g_p`.

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
