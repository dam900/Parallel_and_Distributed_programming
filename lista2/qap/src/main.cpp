#include <mpi.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "qap_data_reader.hpp"

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (rank == 0) {
        std::string filename = "/home/dam900/Downloads/qapdata/chr20a.dat";
        QapDataReader reader = QapDataReader();
        auto [n, flowMatrix, distanceMatrix] = reader.fromDataFile(filename);

        std::cout << "n: " << n << std::endl;
        std::cout << "Flow Matrix: " << std::endl;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                std::cout << flowMatrix[i][j] << " ";
            }
            std::cout << std::endl;
        }
        std::cout << "Distance Matrix: " << std::endl;
        for (int i = 0; i < n; i++) {
            for (int j = 0; j < n; j++) {
                std::cout << distanceMatrix[i][j] << " ";
            }
            std::cout << std::endl;
        }
    }
    MPI_Finalize();
    return 0;
}