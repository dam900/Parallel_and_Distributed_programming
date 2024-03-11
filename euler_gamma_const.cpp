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
    double sendbuf = 0;

    double amount = floor((n / (processes-1)));    
    double bottom_bound = rank*amount+1; 
    double upper_bound = (rank+1)*amount;

    if (rank == processes-2) {
        upper_bound = (double)n-(processes-3)*amount;  
    }

    for (int i = bottom_bound; i < upper_bound; i++){
        sendbuf += 1 / i;
    }

    MPI_Reduce(&sendbuf, &sum, 1, MPI_DOUBLE, MPI_SUM, processes-1, MPI_COMM_WORLD);

    if (rank == processes-1){
        sum -= log((double)n);
        std::cout << "sum is equal to " << sum << std::endl;
    }
    MPI_Finalize(); // Finalize the MPI environment.
}
