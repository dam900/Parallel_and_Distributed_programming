#include <mpi.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "qap_data_reader.hpp"
#include "qap_solver.hpp"

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
        MPI_Bcast(flowMatrix[i].data(), flowMatrix[i].size(), MPI_INT, 0, MPI_COMM_WORLD);
        MPI_Bcast(distanceMatrix[i].data(), distanceMatrix[i].size(), MPI_INT, 0, MPI_COMM_WORLD);
    }

    QapSolver solver = QapSolver(distanceMatrix, flowMatrix, 0.997);
    auto solution = solver.solve(1000, n, 100, 100);

    int bestSolution;

    MPI_Allreduce(&solution.second, &bestSolution, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
    if (bestSolution == solution.second) {
        std::cout << "Solution: ";
        for (auto val : solution.first) {
            std::cout << val << " ";
        }
        std::cout << std::endl;

        std::cout << "Cost: " << solution.second << std::endl;
    }

    MPI_Finalize();
    return 0;
}