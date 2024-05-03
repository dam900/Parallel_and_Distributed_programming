#include <mpi.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "qap_data_reader.hpp"

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);

    if (argc < 2) {
        if (rank == 0) {
            std::cout << "Usage: " << argv[0] << " <n> <filename>" << std::endl;
        }
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    int n = std::stoi(argv[1]);
    std::string filename = std::string(argv[2]);
    IntMatrix flowMatrix(n, std::vector<int>(n));
    IntMatrix distanceMatrix(n, std::vector<int>(n));

    if (rank == 0) {
        QapDataReader reader = QapDataReader();
        auto [n, f, d] = reader.fromDataFile(filename);
        flowMatrix = f;
        distanceMatrix = d;
    }
    for (int i = 0; i < n; i++) {
        MPI_Bcast(flowMatrix[i].data(), n, MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(distanceMatrix[i].data(), n, MPI_INT, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}