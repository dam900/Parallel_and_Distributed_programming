#include <mpi.h> // Import MPI lib
#include <iostream>
#include <list>
#include <unistd.h>
#include <cmath>

#define DEBUG 1

// MPI tags
#define P_PARAMETER_INFO 0
#define PART_OF_SUM 1

void run_client_task(int, int);
void run_worker_task(int, int, int);

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv); // Initialize the MPI environment
    int processes;
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD, &processes); // Get the number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get the rank of the process

    int p = std::stoi(argv[2]); // provide imaginary flag -p <workload>

    if (rank == processes -1) {
        run_client_task(rank, processes);
    } else {
        run_worker_task(rank, processes, p);
    }
    MPI_Finalize(); // Finalize the MPI environment.
}

void run_client_task(int my_rank, int num_procs) {

    if (DEBUG) std::cout << "Hello from client task: " << my_rank << std::endl;

    double sum;
    double sendbuf;
    MPI_Reduce(&sendbuf, &sum, 1, MPI_DOUBLE, MPI_SUM, num_procs-1, MPI_COMM_WORLD);

    std::cout << "sum is equal to " << sum << std::endl;

}

void run_worker_task(int my_rank, int num_procs, int p) {
    if (DEBUG) std::cout << "Hello from worker task: " << my_rank << std::endl;
    double recvbuf;
    double sum = 0;
    double log_part = log((double)((num_procs-1)*p));

    for (int i = my_rank*p + 1; i < (my_rank+1)*p; i++){
        sum += (1 / i ) - log_part;
    }
    MPI_Reduce(&sum, &recvbuf, 1, MPI_DOUBLE, MPI_SUM, num_procs-1, MPI_COMM_WORLD);
}
