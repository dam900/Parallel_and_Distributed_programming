#include <mpi.h> // Import MPI lib
#include <stdio.h>
#include <time.h>
#include <unistd.h>
#include <list>

#define TABLE_RANK 0

#define GRAB_FORKS_REQUEST 0
#define PUT_DOWN_FORKS_REQUEST 1
#define GRAB_FORKS_PERMISSION_RESPONSE 2


#define DEBUG 0

void run_table_task(int, int);
void run_philosopher_task(int);

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv); // Initialize the MPI environment
    int processes;
    int rank;
    MPI_Comm_size(MPI_COMM_WORLD, &processes); // Get the number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get the rank of the process

    if (rank == TABLE_RANK) {
        run_table_task(rank, processes);
    } else {
        run_philosopher_task(rank);
    }

    MPI_Finalize(); // Finalize the MPI environment.
}

void run_table_task(int my_rank, int num_procs)
{
    if (DEBUG) printf("Hello from table task: %d \n",my_rank);
 
    int buffer_in;
    int buffer_out;
    int philosopher;
    MPI_Status status;

    std::list<int> queue;

    bool forks[num_procs-1];
    for (int i =0; i<num_procs-1;i++){
        forks[i] = true;
    }

    while (true) {
        MPI_Recv(&buffer_in, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
        philosopher = status.MPI_SOURCE;

        if (status.MPI_TAG == GRAB_FORKS_REQUEST){
            if (DEBUG) std::cout << "Got GRAB_FORKS_REQUEST from " << status.MPI_SOURCE << std::endl;
            if (forks[philosopher%(num_procs-1)] == true && forks[philosopher-1] == true){
                forks[philosopher%(num_procs-1)] = false;
                forks[philosopher-1] = false;
                MPI_Send(&buffer_out, 1, MPI_INT, philosopher, GRAB_FORKS_PERMISSION_RESPONSE, MPI_COMM_WORLD);
                if (DEBUG) std::cout << "Sent GRAB_FORKS_PERMISSION_RESPONSE to " << status.MPI_SOURCE << std::endl;
            } else {
                if (DEBUG) std::cout << "Got GRAB_FORKS_REQUEST from " << status.MPI_SOURCE << std::endl;
                if (DEBUG) std::cout << "There are no available forks for " << status.MPI_SOURCE << std::endl << "Adding " << philosopher << " to the queue" << std::endl;
                queue.push_back(philosopher);
            }
        }
        else if (status.MPI_TAG == PUT_DOWN_FORKS_REQUEST){
            if (DEBUG) std::cout << "Got PUT_DOWN_FORKS_REQUEST from " << status.MPI_SOURCE << std::endl;
            forks[philosopher%(num_procs-1)] = true;
            forks[philosopher-1] = true;
            if (DEBUG) std::cout << "Put down forks from philosopher " << philosopher << std::endl;
            if (!queue.empty()) {
                if (DEBUG) std::cout << "Processing queue" << std::endl;
                for (std::list<int>::iterator it = queue.begin(); it != queue.end(); it++){
                    philosopher = *it;
                    if (forks[philosopher%(num_procs-1)] == true && forks[philosopher-1] == true){
                        forks[philosopher%(num_procs-1)] = false;
                        forks[philosopher-1] = false;
                        MPI_Send(&buffer_out, 1, MPI_INT, philosopher, GRAB_FORKS_PERMISSION_RESPONSE, MPI_COMM_WORLD);
                        if (DEBUG) std::cout << "Sent GRAB_FORKS_PERMISSION_RESPONSE to " << status.MPI_SOURCE << std::endl;
                        it = queue.erase(it);
                    }
                }
            }
        }

    }
}

void run_philosopher_task(int my_rank)
{
    if (DEBUG) printf("Hello from philosopher task: %d\n",my_rank);
    srand(time(NULL) + my_rank);
    int buffer_in;
    int buffer_out;
    MPI_Status status;
    int seconds;

    while (true)
    {   
        seconds = rand() % 10; 
        printf("Philosopher %d is thinking for %d seconds\n", my_rank, seconds);
        sleep(seconds); // Think
        printf("Philosopher %d is waiting for forks\n", my_rank);

        // Grab forks
        MPI_Send(&buffer_out, 1,MPI_INT, TABLE_RANK, GRAB_FORKS_REQUEST, MPI_COMM_WORLD);
        MPI_Recv(&buffer_in, 1, MPI_INT, TABLE_RANK, GRAB_FORKS_PERMISSION_RESPONSE, MPI_COMM_WORLD, &status);

        seconds = rand() % 10; 
        printf("Philosopher %d is eating for %d seconds\n", my_rank, seconds);
        sleep(seconds); // Eat
        printf("Philosopher %d is done eating\n", my_rank);
        MPI_Send(&buffer_out, 1,MPI_INT, TABLE_RANK, PUT_DOWN_FORKS_REQUEST, MPI_COMM_WORLD);
    }
    
}
