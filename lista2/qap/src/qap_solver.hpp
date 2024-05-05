#pragma once
#include <vector>

#include "qap_data_reader.hpp"

typedef std::vector<int> SolutionCandidate;

class QapSolver {
   public:
    double coolingRate;
    QapSolver(const IntMatrix &distanceMatrix, const IntMatrix &flowMatrix, double coolingRate);

    std::pair<SolutionCandidate, int> solve(int max_iter, int num_cities, int exchange_period, double init_temp);

   private:
    IntMatrix distanceMatrix;
    IntMatrix flowMatrix;
    int rank;
    int num_procs;

    int cost(const SolutionCandidate &candidate);
};
