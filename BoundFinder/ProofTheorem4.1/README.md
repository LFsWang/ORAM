# Proof of Theorem 4.1

This small program prints the intermediate values used in the calculation for
Theorem 4.1 in Section 4.5, with bucket configuration `(4, 4, 4, 4, 5, ...)`
and `t = 0.530`. The main bound computation is provided in
[`../finder.cpp`](../finder.cpp); this script makes the intermediate steps
explicit for reference.

Run from this directory with Python 3 (no additional packages required):

```sh
python3 main.py
```

The output follows Equations (4.13)–(4.15), showing `e^(-t)`, the weights
computed by `A(t, Z)` and `B(t, Z)`, and the recursive values from `g_L` through
`g_{L-3}`. It also prints the larger fixed point `g_p` of
`g = A(t, 5) * (1 + g)^2` and checks that `g_{L-3} < g_p`.
