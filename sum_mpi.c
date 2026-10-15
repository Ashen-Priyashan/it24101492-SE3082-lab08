#include <inttypes.h>
#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    const uint64_t limit = argc > 1 ? strtoull(argv[1], NULL, 10) : 10000000ULL;
    int rank, size;
    uint64_t local_sum = 0, total_sum = 0;
    double started, elapsed;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Barrier(MPI_COMM_WORLD);
    started = MPI_Wtime();

    for (uint64_t value = (uint64_t)rank + 1; value <= limit; value += (uint64_t)size)
        local_sum += value;

    MPI_Reduce(&local_sum, &total_sum, 1, MPI_UINT64_T, MPI_SUM, 0, MPI_COMM_WORLD);
    elapsed = MPI_Wtime() - started;

    if (rank == 0) {
        printf("sum=%" PRIu64 "\n", total_sum);
        printf("TIME_SECONDS=%.9f\n", elapsed);
        if (limit == 10000000ULL && total_sum != 50000005000000ULL) {
            fprintf(stderr, "Unexpected sum result.\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
    }

    MPI_Finalize();
    return 0;
}
