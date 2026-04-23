This repository contains flag algebraic calculations for the paper "Seeking the dense half in K_4-free graphs".

# Acknowledgement
The original code used for this project was based on Bernard Lidický's work. It was modified and extended for these problems by Ignacy Buczek.

# Requirements
To run calculations in this repository, you need:
- `csdp` -- compiled with more lenient exit conditions. You can find the exact source code in the Csdp library of this repository.
- `sage`
- `cmake`.

# Setup
```
./init-cmake.sh
```

# Seeking the dense half in K_4-free graphs

### Creating and proving the infeasibility of the SDP instance
```
cmake --build build --target max-2-case-<case-number>-solve-approximate
```
`<case-number>` should be replaced with an integer between 1 and 8.
