#include <mpi.h>
#include <omp.h>

#include <fstream>
#include <iostream>
#include <vector>

#include "neh_data_reader.hpp"
#include "rng.hpp"
#include "simulated_annealing_solver.hpp"

typedef std::vector<int> solution_t;

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv);
    int rank;
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    int num_procs;
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);

    int n = 50;
    int M = 20;
    std::string filename = std::string("./data/neh50_20.dat");

    IntMatrix tasks(n, std::vector<int>(n));

    if (rank == 0) {
        NehDataReader reader = NehDataReader();
        tasks = reader.fromDataFile(filename);
    }

    for (int i = 0; i < n; i++) {
        MPI_Bcast(tasks[i].data(), tasks[i].size(), MPI_INT, 0, MPI_COMM_WORLD);
    }

    IntRNG swap = IntRNG(0, n - 1);
    IntRNG with = IntRNG(0, n - 1);

    std::function<void(solution_t&)> make_change = [&](solution_t& candidate) {
        std::swap(candidate[swap.getNext()], candidate[with.getNext()]);
    };

    std::function<double(const solution_t&)> cost = [&](const solution_t& candidate) {
        std::vector<int> Cmaxs(M, 0);
        for (int p = 0; p < n; p++) {
            int t_index = candidate[p] - 1;
            for (int m = 0; m < M; m++) {
                if (m != 0) {
                    Cmaxs[m] = std::max(Cmaxs[m - 1], Cmaxs[m]) + tasks[t_index][m];
                } else {
                    Cmaxs[m] = Cmaxs[m] + tasks[t_index][m];
                }
            }
        }
        int maxCmax = *std::max_element(Cmaxs.begin(), Cmaxs.end());
        return maxCmax;
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

    std::ofstream f;
    std::ofstream f2;
    auto f_name = "./data_out/cost/out_rank" + std::to_string(rank) + ".data";
    auto f_name_2 = "./data_out/path/out_rank_solutions" + std::to_string(rank) + ".data";
    f.open(f_name);
    f2.open(f_name_2);

    std::function<void(solution_t&, double)> on_new_solution = [&](solution_t& new_solution, double new_cost) {
        f << new_cost << std::endl;
        for (auto item : new_solution) {
            f2 << item << ",";
        }
        f2 << std::endl;
    };

    // std::unique_ptr<CoolingStrategy> cooling_strategy = std::make_unique<GeometricCoolingStrategy>(0.996);
    std::unique_ptr<CoolingStrategy> cooling_strategy = std::make_unique<LogarithmicCoolingStrategy>(0.001);
    // std::unique_ptr<CoolingStrategy> cooling_strategy = std::make_unique<LinearCoolingStrategy>(100 / 1000 + 1);
    SimmulatedAnnealingSolver<solution_t> solver = SimmulatedAnnealingSolver<solution_t>(cost, make_change, init_start_sol, exchange_solutions, std::move(cooling_strategy), on_new_solution);

    double s = MPI_Wtime();
    auto solution = solver.solve(1000, 100, 120, NO_TIME_LIMIT);
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

    f.close();
    MPI_Finalize();
    return 0;
}
