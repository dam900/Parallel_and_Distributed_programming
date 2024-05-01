#pragma once
#include <list>
#include <vector>

// 2d vector wrapper type
typedef std::vector<std::vector<double>> Matrix;

// list of integers wrapper type
typedef std::list<int> Path;

/**
 * Parallel Ant Colony System.
 * Using MPI for communication between colonies
 */
class MPI_PACS {
   public:
    /**
     * Set the parameters for the ACO algorithm
     * @param beta distance importance
     * @param rho evaporation rate
     * @param theta pheromone deposit amount
     * @param q some constant
     * @param tau initial pheromone level
     */
    MPI_PACS(double beta, double rho, double theta, double q, double tau);
    /**
     * Set the adjacency matrix
     * @param adj_mat adjacency matrix
     */
    void set_adj_mat(const Matrix &adj_mat);
    /**
     * Set the pheromones matrix
     * @param pheromones pheromones matrix
     */
    void set_pheromones(const Matrix &pheromones);

    /**
     * Run the ACO algorithm
     * @param num_ants number of ants
     * @param num_iter number of iterations
     * @param num_cities number of cities
     * @param comm_freq communication frequency
     */
    std::pair<double, Path> run(int num_ants, int num_iter, int num_cities, int comm_freq);

   private:
    double BETA = -2.0;  // Distance importance
    double RHO = 0.3;    // Evaporation rate
    double THETA = 3.0;  // Pheromone deposit amount
    double TAU = 0.6;    // Initial pheromone level
    double Q = 100.0;    // Some constant

    int num_procs;  // Number of MPI processes
    int rank;       // Rank of the MPI process

    Matrix adj_mat;     // Adjacency matrix
    Matrix pheromones;  // Pheromones matrix

    /**
     * Pheromone update strategies
     * @param Local: update pheromones after single ant completes its path in a single node (MPI process)
     * @param Global: update pheromones after all ants have completed their paths by taking the best path from all paths taken in a single node (MPI process)
     */
    enum class PHEROMONE_UPDATE_STRATEGY {
        GLOBAL = 0,
        LOCAL = 1,
    };

    /**
     * Generate a path for an ant
     * @param start starting point
     * @param n number of cities
     */
    Path generate_path(int start, int n);

    /**
     * Calculate the cost of a path
     * @param path path to calculate the cost for
     */
    double cost(const Path &path);

    /**
     * Update the pheromones based on the path taken by an ant
     * @param path path taken by an ant
     * @param strategy pheromone update strategy
     */
    void update_pheromones(const Path &path, PHEROMONE_UPDATE_STRATEGY strategy);

    /**
     * Local update strategy
     * @param path path taken by an ant
     */
    void local_update_strategy(const Path &path);

    /**
     * Global update strategy
     * @param path path taken by an ant
     */
    void global_update_strategy(const Path &path);

    /**
     * Construct a list of unvisited cities
     * @param start starting point
     * @param n number of cities
     */
    std::list<int> construct_unvisited_list(int start, int n);
};
