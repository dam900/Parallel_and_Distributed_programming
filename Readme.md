#Some helpfull notes for MPI programs

##Compiling 
for **C** use `mpicc -o <output filename> <input filename>`
for **C++** use `mpic++ -o <output filename> <input filename>`

##Running 
for both use `mpiexec -n <number of processes> <executable filename>`

##Basic program structure

```c++
...
#include <mpi.h> // Import MPI lib

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv); // Initialize the MPI environment

    int processes;
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD, &processes); // Get the number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get the rank of the process
    ...
    if (rank == 0) {
        // target some process by its rank/id
        ... // do something if this is the process with rank 0
    } else {
        ... // if not do something else 
    }
    ...
    MPI_Finalize(); // Finalize the MPI environment.

}

```