#include <functional>
#include <iostream>
#include <list>
#include <memory>
#include <random>

#include "rng.hpp"
#define NO_EXCHANGE_PERIOD -1
#define NO_TIME_LIMIT -1

/**
 * Interface for cooling strategies
 */
class CoolingStrategy {
   public:
    /**
     * Get the next temperature.
     * This function is called after each iteration
     * @param prev_t the previous temperature
     */
    virtual double next(double prev_t) = 0;
};

/**
 * Linear cooling strategy
 * The temperature is decreased by a constant rate
 * t_{i+1} = t_i - cooling_rate
 */
class LinearCoolingStrategy : public CoolingStrategy {
   public:
    LinearCoolingStrategy(double cooling_rate) : cooling_rate(cooling_rate) {}
    double next(double prev_t) override { return prev_t - cooling_rate; }

   private:
    double cooling_rate;
};
/**
 * Geometric cooling strategy
 * The temperature is decreased by a constant rate
 * t_{i+1} = t_i * cooling_rate

*/
class GeometricCoolingStrategy : public CoolingStrategy {
   public:
    GeometricCoolingStrategy(double cooling_rate) : cooling_rate(cooling_rate) {}
    double next(double prev_t) override { return prev_t * cooling_rate; }

   private:
    double cooling_rate;
};

/**
 * Logarithmic cooling strategy
 * The temperature is decreased by a constant rate
 * t_{i+1} = t_i / (1 + lambda * cooling_rate)
 */
class LogarithmicCoolingStrategy : public CoolingStrategy {
   public:
    LogarithmicCoolingStrategy(double cooling_rate) : cooling_rate(cooling_rate) {}
    double next(double prev_t) override { return prev_t / (1 + lambda * cooling_rate); }

   private:
    double cooling_rate;
    double lambda;
};

/**
 * Simmulated Annealing Solver, a generic solver for the simmulated annealing algorithm
 * with
 * @tparam T the type of the solution
 */
template <typename T>
class SimmulatedAnnealingSolver {
   public:
    /**
     * Constructor
     * @param cost the cost function
     * @param make_change the make change function
     * @param init_start_sol the initial start solution function
     * @param exchange_solutions the exchange solutions function
     * @param cooling_strategy the cooling strategy
     */
    SimmulatedAnnealingSolver(std::function<double(T &)> cost, std::function<void(T &)> make_change, std::function<T()> init_start_sol, std::function<std::list<T>(T &)> exchange_solutions, std::unique_ptr<CoolingStrategy> cooling_strategy) : cost(cost), make_change(make_change), init_start_sol(init_start_sol), exchange_solutions(exchange_solutions), cooling_strategy(std::move(cooling_strategy)) {}
    /**
     * Solve the problem
     * @param num_iter the number of iterations
     * @param inital_temp the initial temperature
     * @param exchange_period the exchange period
     * @param time_limit the time limit
     * @return a pair of the best solution and the cost of the best solution
     */
    std::pair<T, double> solve(int num_iter, double inital_temp, int exchange_period = NO_EXCHANGE_PERIOD, double time_limit = NO_TIME_LIMIT) {
        DoubleRNG prob = DoubleRNG(0, 1);
        if (exchange_period == NO_EXCHANGE_PERIOD) {
            exchange_period = num_iter;
        }

        T best_solution = init_start_sol();
        T global_best_solution = init_start_sol();
        double best_cost = cost(best_solution);
        double t = inital_temp;

        double start = MPI_Wtime();
        for (int i = 0; i < num_iter; i++) {
            if (time_limit != NO_TIME_LIMIT && MPI_Wtime() - start > time_limit) {
                MPI_Barrier(MPI_COMM_WORLD);
                break;
            }
            T prev_solution = best_solution;
            make_change(best_solution);
            double current_cost = cost(best_solution);
            auto delta = current_cost - best_cost;
            if (current_cost < best_cost) {
                best_cost = current_cost;
                global_best_solution = best_solution;
            } else if (prob.getNext() < exp(-delta / t)) {
                best_cost = current_cost;
            } else {
                best_solution = prev_solution;
            }

            if (i % exchange_period == 0 && i != 0) {
                auto gathered_solutions = exchange_solutions(global_best_solution);

                for (auto &solution : gathered_solutions) {
                    double solution_cost = cost(solution);
                    if (solution_cost < best_cost) {
                        best_cost = solution_cost;
                        best_solution = solution;
                    }
                }
            }
            t = cooling_strategy->next(t);
        }

        return {global_best_solution, cost(global_best_solution)};
    }

   private:
    /**
     * The cooling strategy
     * Some predefined cooling strategies are available
     * If you want to use a custom cooling strategy, you can implement the CoolingStrategy interface
     * @param LinearCoolingStrategy
     * @param GeometricCoolingStrategy
     * @param LogarithmicCoolingStrategy
     * @param CoolingStrategy
     */
    std::unique_ptr<CoolingStrategy> cooling_strategy;
    /**
     * The cost function
     * This function should return the cost of the solution
     * @param T the solution
     */
    std::function<double(T &)> cost;
    /**
     * The make change function
     * This function should change the solution in some way
     * This function is called in each iteration
     * The solution is passed by reference
     *
     * @param T the solution
     */
    std::function<void(T &)> make_change;
    /**
     * The initial start solution function
     * This function should return the initial solution
     */
    std::function<T()> init_start_sol;
    /**
     * The exchange solutions function used in parrallel implementation of the algorithm.
     * This function should gather solutions from other processes/workers.
     * The implementation of this was designed with openmpi in mind (multi processing, not threading)
     * After the function is called, the best solution is selected and used as the current solution
     * @param T the best solution of the process / worker / thread
     */
    std::function<std::list<T>(T &)> exchange_solutions;
};
