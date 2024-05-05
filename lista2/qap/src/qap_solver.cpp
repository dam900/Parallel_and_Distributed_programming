#include "qap_solver.hpp"

#include <mpi.h>

#include "rng.hpp"

QapSolver::QapSolver(const IntMatrix &distanceMatrix, const IntMatrix &flowMatrix, double coolingRate) {
    this->distanceMatrix = distanceMatrix;
    this->flowMatrix = flowMatrix;
    this->coolingRate = coolingRate;
}

int QapSolver::cost(SolutionCandidate const &candidate) {
    int cost = 0;
    for (int i = 0; i < candidate.size(); i++) {
        for (int j = 0; j < candidate.size(); j++) {
            cost += flowMatrix[i][j] * distanceMatrix[candidate[i] - 1][candidate[j] - 1];
        }
    }
    return cost;
}

std::pair<SolutionCandidate, int> QapSolver::solve(int max_iter, int num_cities, int exchange_period, double init_temp) {
    SolutionCandidate bestSolution = SolutionCandidate(num_cities);
    for (int i = 0; i < num_cities; i++) {
        bestSolution[i] = i + 1;
    }

    IntRNG swap = IntRNG(0, num_cities - 1);
    IntRNG with = IntRNG(0, num_cities - 1);
    DoubleRNG prob = DoubleRNG(0, 1);

    double temp = init_temp;
    int bestCost = cost(bestSolution);

    for (int i = 0; i < max_iter; i++) {
        int swapIndex = swap.getNext();
        int withIndex = with.getNext();
        std::swap(bestSolution[swapIndex], bestSolution[withIndex]);
        int currentCost = cost(bestSolution);

        int delta = currentCost - bestCost;
        if (currentCost < bestCost) {
            bestCost = currentCost;
        } else if (prob.getNext() < exp(-delta / temp)) {
            bestCost = currentCost;
        } else {
            std::swap(bestSolution[swapIndex], bestSolution[withIndex]);
        }

        if (i % exchange_period == 0) {
            int bestSolutionCost;
            MPI_Allreduce(&bestCost, &bestSolutionCost, 1, MPI_INT, MPI_MIN, MPI_COMM_WORLD);
            if (bestSolutionCost != bestCost) {
                bestCost = bestSolutionCost;
            }
        }

        temp *= coolingRate;
    }

    return std::pair{bestSolution, cost(bestSolution)};
}