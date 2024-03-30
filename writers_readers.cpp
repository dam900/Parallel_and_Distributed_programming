#include <mpi.h> // Import MPI lib
#include <iostream>
#include <time.h>
#include <list>
#include <unistd.h>

#define COORDINATOR_RANK 0

#define WRITE_TAG 0
#define READ_TAG 1
#define SUCCESS_TAG 2

void reader(int);
void writer(int);
void coordinator(int);

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv); // Initialize the MPI environment
    int processes;
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD, &processes); // Get the number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get the rank of the process
    
    if (rank == 0) {
        coordinator(rank);
    } else if (rank < 3) {
        writer(rank);
    } else {
        reader(rank);
    }

    MPI_Finalize(); // Finalize the MPI environment.
}

void coordinator(int rank){
    int val = -1;
    int r;
    MPI_Status status;
    int num_reads = 0;
    std::list<int> readers_queue;
    int current = -1;

    while (true) {
        MPI_Recv(&val, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        if (status.MPI_TAG == WRITE_TAG) {
            if (num_reads > 3 || current == -1) {
                current = val;
                num_reads = 0;
                std::cout << "Writer " << status.MPI_SOURCE << " wrote " << val << std::endl;
            }
        } else {
            if (current == -1){
                readers_queue.push_back(status.MPI_SOURCE);
            } else {
                MPI_Send(&current, 1, MPI_INT, status.MPI_SOURCE, SUCCESS_TAG, MPI_COMM_WORLD);
                num_reads++;
            }
            if (!readers_queue.empty() && current != -1) {
                for (std::list<int>::iterator it = readers_queue.begin(); it != readers_queue.end(); it++){
                        r = *it;
                        MPI_Send(&current, 1, MPI_INT, r, SUCCESS_TAG, MPI_COMM_WORLD);
                        num_reads++;
                        it = readers_queue.erase(it);
                    }
                }
            }
        }
    }

void writer(int rank) {
    srand(time(NULL) + rank);
    int sleep_time;
    int val;

    while (true) {
        val = rand() % 10;
        sleep_time = rand() % 10;
        sleep(sleep_time);
        MPI_Send(&val, 1, MPI_INT, COORDINATOR_RANK, WRITE_TAG, MPI_COMM_WORLD);
    }   
}

void reader(int rank) {
    srand(time(NULL) + rank);
    int sleep_time;
    int val;
    while (true) {
        sleep_time = rand() % 10;
        sleep(sleep_time);
        MPI_Send(&val, 1, MPI_INT, COORDINATOR_RANK, READ_TAG, MPI_COMM_WORLD);
        MPI_Recv(&val, 1, MPI_INT, COORDINATOR_RANK, SUCCESS_TAG, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        std::cout << "Reader " << rank << " is reading " << val << std::endl;
    }
}