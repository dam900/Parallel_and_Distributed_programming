#include <mpi.h>
#include <omp.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "qap_data_reader.hpp"
#include "rng.hpp"
#include "simulated_annealing_solver.hpp"

typedef std::vector<int> solution_t;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int num_procs;
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

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

    IntRNG swap = IntRNG(0, n - 1);
    IntRNG with = IntRNG(0, n - 1);

    std::function<void(solution_t&)> make_change = [&](solution_t& candidate) {
        std::swap(candidate[swap.getNext()], candidate[with.getNext()]);
    };

    std::function<double(const solution_t&)> cost = [&](const solution_t& candidate) {
        double cost = 0;
        int i, j;
        for (i = 0; i < candidate.size(); i++) {
            for (j = 0; j < candidate.size(); j++) {
                cost += flowMatrix[i][j] * distanceMatrix[candidate[i] - 1][candidate[j] - 1];
            }
        }
        return cost;
    };

    std::function<solution_t()> init_start_sol = [&]() {
        solution_t bestSolution = solution_t(n);
        for (int i = 0; i < n; i++) {
            bestSolution[i] = i + 1;
        }
        std::shuffle(bestSolution.begin(), bestSolution.end(), std::default_random_engine());
        return bestSolution;
    };

    std::function<std::list<solution_t>(solution_t&)> exchange_solutions = [&](solution_t& candidate) {
        solution_t globalSolutions = solution_t(n * num_procs);
        MPI_Allgather(candidate.data(), n, MPI_INT, globalSolutions.data(), n, MPI_INT, MPI_COMM_WORLD);
        std::list<solution_t> solutions = std::list<solution_t>();
        for (int j = 0; j < num_procs; j++) {
            solution_t c = solution_t(globalSolutions.begin() + j * n, globalSolutions.begin() + (j + 1) * n);
            solutions.push_back(c);
        }
        return solutions;
    };

    std::unique_ptr<CoolingStrategy> cooling_strategy = std::make_unique<GeometricCoolingStrategy>(0.996);
    SimmulatedAnnealingSolver<solution_t> solver = SimmulatedAnnealingSolver<solution_t>(cost, make_change, init_start_sol, exchange_solutions, std::move(cooling_strategy));

    double s = MPI_Wtime();
    auto solution = solver.solve(1000, 100, NO_EXCHANGE_PERIOD, NO_TIME_LIMIT);
    std::cout << "RANK[" << rank << "] " << "Time: " << MPI_Wtime() - s << std::endl;
    MPI_Barrier(MPI_COMM_WORLD);

    double bestSolution;
    MPI_Allreduce(&solution.second, &bestSolution, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
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
