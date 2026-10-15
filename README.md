# SE3082 Lab 08 - MPI

## Exercises

- Exercise 1: repository setup and version control.
- Exercise 2: parallel sum from 1 through 10,000,000.
- Exercise 3: Monte Carlo estimate of pi using 10,000,000 samples.
- Exercise 4: run-time and speedup plots for the sum and pi programs.
- Exercise 5: mismatched message source demonstration and buffered send.
- Exercise 6: Monte Carlo result collection with `MPI_ANY_SOURCE`.
- Exercise 7: buffered result sends with `MPI_ANY_SOURCE` receives.

## Build and run

With an MPI C compiler and runtime on `PATH`, Windows users can build the
programs from PowerShell with:

```powershell
.\build.ps1
```

Other environments can use `make`. Then run:

```sh
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

## Exercise 6: receive from any worker

`pi_any_source.exe` runs the same ten-million-point Monte Carlo workload as
Exercise 3. The coordinator receives each result using `MPI_ANY_SOURCE` and
the result tag, so worker completion order does not have to match rank order.
The reported estimate should be close to pi; timings can vary between runs.

## Exercise 7: buffered worker replies

`pi_any_source_bsend.exe` keeps the `MPI_ANY_SOURCE` coordinator receive and
changes each worker's result send to `MPI_Bsend`. Each worker attaches a buffer
large enough for the packed 64-bit result, sends it, detaches the buffer, and
then frees the detached storage. It uses the same sample count and random
sequence as the other pi program variants, so the estimate is identical for a
given process count.

