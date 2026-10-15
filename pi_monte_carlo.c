#include <inttypes.h>
#include <mpi.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

enum { WORK_TAG = 10, RESULT_TAG = 20 };

static uint64_t next_random(uint64_t *state)
{
    uint64_t x = *state;
    x ^= x >> 12;
    x ^= x << 25;
    x ^= x >> 27;
    *state = x;
    return x * 2685821657736338717ULL;
}

static uint64_t count_hits(uint64_t samples, uint64_t seed)
{
    uint64_t hits = 0;
    for (uint64_t i = 0; i < samples; ++i) {
        const double x = (double)(next_random(&seed) >> 11) * (1.0 / 9007199254740992.0);
        const double y = (double)(next_random(&seed) >> 11) * (1.0 / 9007199254740992.0);
        if (x * x + y * y <= 1.0)
            ++hits;
    }
    return hits;
}

int main(int argc, char **argv)
{
    const uint64_t samples = argc > 1 ? strtoull(argv[1], NULL, 10) : 10000000ULL;
    int rank, size;
    uint64_t total_hits = 0;
    double started, elapsed;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);
    MPI_Barrier(MPI_COMM_WORLD);
    started = MPI_Wtime();

    if (size == 1) {
        total_hits = count_hits(samples, 0x9e3779b97f4a7c15ULL);
    } else if (rank == 0) {
        uint64_t base = samples / (uint64_t)(size - 1);
        uint64_t extra = samples % (uint64_t)(size - 1);
        for (int worker = 1; worker < size; ++worker) {
            uint64_t assigned = base + ((uint64_t)(worker - 1) < extra ? 1 : 0);
            MPI_Send(&assigned, 1, MPI_UINT64_T, worker, WORK_TAG, MPI_COMM_WORLD);
        }
        for (int worker = 1; worker < size; ++worker) {
            uint64_t worker_hits = 0;
#ifdef RECEIVE_ANY_SOURCE
            MPI_Status status;
            MPI_Recv(&worker_hits, 1, MPI_UINT64_T, MPI_ANY_SOURCE, RESULT_TAG,
                     MPI_COMM_WORLD, &status);
#else
            MPI_Recv(&worker_hits, 1, MPI_UINT64_T, worker, RESULT_TAG,
                     MPI_COMM_WORLD, MPI_STATUS_IGNORE);
#endif
            total_hits += worker_hits;
        }
    } else {
        uint64_t assigned = 0;
        MPI_Recv(&assigned, 1, MPI_UINT64_T, 0, WORK_TAG,
                 MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        uint64_t worker_hits = count_hits(assigned, 0x9e3779b97f4a7c15ULL ^ (uint64_t)rank);
#ifdef BUFFERED_RESULT_SEND
        unsigned char buffer[MPI_BSEND_OVERHEAD + sizeof(worker_hits)];
        MPI_Buffer_attach(buffer, (int)sizeof(buffer));
        MPI_Bsend(&worker_hits, 1, MPI_UINT64_T, 0, RESULT_TAG, MPI_COMM_WORLD);
        void *detached_buffer = NULL;
        int detached_size = 0;
        MPI_Buffer_detach(&detached_buffer, &detached_size);
#else
        MPI_Send(&worker_hits, 1, MPI_UINT64_T, 0, RESULT_TAG, MPI_COMM_WORLD);
#endif
    }

    elapsed = MPI_Wtime() - started;
    if (rank == 0) {
        printf("samples=%" PRIu64 "\n", samples);
        printf("pi=%.10f\n", 4.0 * (double)total_hits / (double)samples);
        printf("TIME_SECONDS=%.9f\n", elapsed);
    }

    MPI_Finalize();
    return 0;
}
