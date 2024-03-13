#include <mpi.h> // Import MPI lib
#include <iostream>
#include <cmath>

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv); // Initialize the MPI environment
    int processes;
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD, &processes); // Get the number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get the rank of the process

    int n = std::stoi(argv[2]); // provide imaginary flag -n / --number <number of elemnts to sum>

    double sum;
    double sendbuf = 0.0;
    
    for (int i = rank+1; i < n+1; i+=processes){
        sendbuf += 1.0 / i;
    }

    MPI_Reduce(&sendbuf, &sum, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);

    if (rank == 0){
        sum -= log(n);
        std::cout << "gamma is equal to " << sum << std::endl;
    }
    MPI_Finalize(); // Finalize the MPI environment.
}