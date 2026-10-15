#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    int rank, size;
    int message[3] = {10, 20, 30};
    int received[3] = {0, 0, 0};

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size != 2) {
        if (rank == 0)
            fprintf(stderr, "Run this example with exactly 2 processes.\n");
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        int packed_size = 0;
        MPI_Pack_size(3, MPI_INT, MPI_COMM_WORLD, &packed_size);
        int buffer_size = packed_size + MPI_BSEND_OVERHEAD;
        void *buffer = malloc((size_t)buffer_size);
        if (buffer == NULL) {
            fprintf(stderr, "Could not allocate the buffered-send space.\n");
            MPI_Abort(MPI_COMM_WORLD, EXIT_FAILURE);
        }
        MPI_Buffer_attach(buffer, buffer_size);
        MPI_Bsend(message, 3, MPI_INT, 1, 0, MPI_COMM_WORLD);
        void *detached_buffer = NULL;
        int detached_size = 0;
        MPI_Buffer_detach(&detached_buffer, &detached_size);
        free(detached_buffer);
        printf("Rank 0 buffered the message.\n");
    } else {
        MPI_Recv(received, 3, MPI_INT, 0, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Rank 1 received: %d %d %d\n", received[0], received[1], received[2]);
    }

    MPI_Finalize();
    return 0;
}
