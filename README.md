# DPD – Build and Run Guide

## Folder structure

```
project/
├── bin/            # Build artifacts (object files and the executable `dpd`)
├── configs/        # CLI options files passed via xargs (e.g., `teste.config`)
├── input/          # Input instances (.csv)
│   ├── easy/       # Easier instances
│   │   └── work/   # Workspace with tested and working input files
│   └── hard/       # Harder instances - (not tested yet)
├── output/         # Timestamped run outputs: .lp, .out, .sol, and the used config
├── src/            # C source code
├── Makefile        # Build targets (`make`, `make run`, `make clean`, `make format`)
├── Makefile-Edna   # Alternative makefile (if applicable)
├── run_dpd.sh      # Helper script (optional)
└── README.md       # This document
```

## Prerequisites

- GNU Make and GCC
- SCIP installed and linkable at build and runtime (`-lscip`)
  - If the linker cannot find SCIP, set `LDFLAGS` to include the library path, e.g.:
    - `make LDFLAGS="-L /path/to/scip/lib"`
  - If the runtime loader cannot find SCIP, set `LD_LIBRARY_PATH`, e.g.:
    - `export LD_LIBRARY_PATH=/path/to/scip/lib:$LD_LIBRARY_PATH`
- Optional: `clang-format` for `make format`

## Build

- Default build:
  - `make`
- Clean:
  - `make clean`
- Format sources:
  - `make format`

## Run

You can run the solver either with the provided Make target or explicitly with the one‑liner.

- Using the Make target:

```
make run
```

- Explicit command (same as `make run`):

```
make && xargs -a ./configs/teste.config ./bin/dpd ./input/easy/work/input1.csv
```

## Input Generation

The project includes a Python script `main.py` that generates random input instances for the DPD problem.

### What it does

The script generates a CSV file with:

- **Courses (Materias)**: Academic subjects with properties like course load (CH), area, and semester
- **Professors**: Each with minimum/maximum workload constraints and preferences for specific courses
- **Preferences**: Professor-course preference ratings (0-10 scale)

### How to use

The script accepts `NUMBER_OF_COURSES` and `RELATIONSHIP_COURSE_PROFESSOR` as command-line arguments.

1.  **Run the generator**:

    ```bash
    python3 main.py <number_of_courses> <relationship_course_professor>
    ```

    - `<number_of_courses>`: Number of different course types.
    - `<relationship_course_professor>`: Controls professor-to-course ratio. A smaller value increases the number of professors.

    Example:

    ```bash
    python3 main.py 20 2
    ```

2.  **Output**: Creates `saida.csv` in the current directory with the generated instance

You can then move the generated file to `input/easy/work/` or another input directory to use it with the solver.

What this does:

- `make` compiles the project and produces `bin/dpd`.
- `xargs -a ./configs/teste.config` reads additional command‑line options from `configs/teste.config` and appends them to the program invocation.
- `./bin/dpd` is executed with the input CSV path `./input/easy/work/input1.csv` followed by the options from the config file.

Notes:

- Edit `configs/teste.config` to tweak run parameters; each token is passed as a CLI argument.
- Replace the input CSV path with any file from `input/easy/`, `input/hard/`, or your own under `input/easy/work/`.

## Output

Each run creates a timestamped directory in `output/` containing, for example:

- `dpd.lp` / `lns.lp`: model exports
- `*.out`: program logs
- `*.sol`: solution files
- `teste.config`: copy of the used configuration

Inspect these files to analyze model, logs, and solutions for each run.

## Debug

Enable AddressSanitizer and debug symbols (from the Makefile section "## TO DEBUG"):

- Edit Makefile and uncomment:

```
# CFLAGS=-D NO_CONFIG_HEADER -D SCIP_VERSION_MAJOR -g -fsanitize=address -fno-omit-frame-pointer
# $LDFLAGS=-fsanitize=address -L $(SCIP_LIB)
```

Build and run with ASan:

```
make
xargs -a ./configs/teste.config ./bin/dpd ./input/easy/work/input1.csv
```

Debug with GDB (keeping config args):

```
gdb --args ./bin/dpd ./input/entrada_real.csv $(cat ./configs/teste.config)
```
