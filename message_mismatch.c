#include <mpi.h>
#include <stdio.h>

int main(int argc, char **argv)
{
    int rank, size;
    int message = 42;
    int received = 0;

    MPI_Init(&argc, &argv);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Comm_size(MPI_COMM_WORLD, &size);

    if (size < 3) {
        if (rank == 0)
            fprintf(stderr, "Run this demonstration with at least 3 processes.\n");
        MPI_Finalize();
        return 1;
    }

    if (rank == 0) {
        printf("Rank 0 sends to rank 1.\n");
        fflush(stdout);
        MPI_Send(&message, 1, MPI_INT, 1, 0, MPI_COMM_WORLD);
    } else if (rank == 1) {
        printf("Rank 1 waits for a message from rank 2.\n");
        fflush(stdout);
        MPI_Recv(&received, 1, MPI_INT, 2, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        printf("Received %d\n", received);
    } else if (rank == 2) {
        printf("Rank 2 does not send a message.\n");
        fflush(stdout);
    }

    MPI_Finalize();
    return 0;
}
