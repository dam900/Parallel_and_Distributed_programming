#include <mpi.h> // Import MPI lib
#include <iostream>
#include <cmath>

#define DEBUG 1

void run_client_task(int, int, int);
void run_worker_task(int, int, int);

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv); // Initialize the MPI environment
    int processes;
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD, &processes); // Get the number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get the rank of the process

    int n = std::stoi(argv[2]); // provide imaginary flag -n / --number <number of elemnts to sum>

    if (rank == processes -1) {
        run_client_task(rank, processes, n);
    } else {
        run_worker_task(rank, processes, n);
    }
    MPI_Finalize(); // Finalize the MPI environment.
}

void run_client_task(int my_rank, int num_procs, int num_elements) {

    if (DEBUG) std::cout << "Hello from client task: " << my_rank << std::endl;

    double sum;
    double sendbuf;

    MPI_Reduce(&sendbuf, &sum, 1, MPI_DOUBLE, MPI_SUM, num_procs-1, MPI_COMM_WORLD);
    sum -= log(num_elements);

    std::cout << "sum is equal to " << sum << std::endl;
}

void run_worker_task(int my_rank, int num_procs, int num_elements) {
    if (DEBUG) std::cout << "Hello from worker task: " << my_rank << std::endl;
    double amount = floor((num_elements / (num_procs-1)));    
    double bottom_bound = my_rank*amount+1; 
    double upper_bound = (my_rank+1)*amount;

    if (my_rank == num_procs-2) {
        upper_bound = (double)num_elements-(num_procs-3)*amount;  
    }

    double recvbuf;
    double sum = 0;
    for (int i = bottom_bound; i < upper_bound; i++){
        sum += 1 / i;
    }
    MPI_Reduce(&sum, &recvbuf, 1, MPI_DOUBLE, MPI_SUM, num_procs-1, MPI_COMM_WORLD);
}
