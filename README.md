# SE3082 Lab 08 - MPI

## Exercises

- Exercise 1: repository setup and version control.
- Exercise 2: parallel sum from 1 through 10,000,000.
- Exercise 3: Monte Carlo estimate of pi using 10,000,000 samples.
- Exercise 4: run-time and speedup plots for the sum and pi programs.
- Exercise 5: mismatched message source demonstration and buffered send.
- Exercise 6: Monte Carlo result collection with `MPI_ANY_SOURCE`.
- Exercise 7: buffered result sends with `MPI_ANY_SOURCE` receives.

The C programs use 64-bit unsigned integers for work counts and sums. The
default run sizes match the lab sheet and can be overridden with a command-line
argument.

## Build and run

With an MPI C compiler and runtime on `PATH`, build the programs with:

```sh
make
mpiexec -n 4 sum_mpi.exe
mpiexec -n 4 pi_monte_carlo.exe
```

The pi programs accept the number of random points as an optional argument.
For the required ten million samples, the default is already correct.

## Exercise 4: timings and speedup

Build the programs, then run:

```sh
python benchmark.py --processes 1,2,4 --repeats 3
```

The script uses the median of repeated `MPI_Wtime` measurements and writes a
CSV table plus separate time and speedup SVG charts for both workloads into
`benchmark-results/`. Speedup is calculated as the one-process time divided by
the time at each process count. For the pi workload, one process computes the
samples itself; with multiple processes, rank 0 distributes work to workers.

## Exercise 5: unmatched messages and buffered sends

Run `mpiexec -n 3 message_mismatch.exe` to observe that rank 1 waits for a
message from rank 2, although rank 0 sends to rank 1. The receive has no
matching send, so the blocking receive does not complete and the program does
not finish normally. Stop the run after observing the printed ranks (for
example, with Ctrl+C).

`message2_bsend.exe` uses a buffered send while keeping the same `message`
data at the sender. It attaches enough storage for the packed message before
calling `MPI_Bsend`.

