#pragma once
#include <list>
#include <vector>

/**
 * Set the parameters for the ACO algorithm
 * @param beta distance importance
 * @param rho evaporation rate
 * @param theta pheromone deposit amount
 * @param gamma pheromone importance
 * @param q some constant
 * @param tau initial pheromone level
 */
void set_params(double beta, double rho, double theta, double gamma, double q, double tau);

// 2d vector wrapper type
typedef std::vector<std::vector<double>>
    Table;

// list of integers wrapper type
typedef std::list<int> AntPath;

/**
 * Pheromone update strategies
 * @param Local: update pheromones after all ants have completed their paths in a single node (MPI process)
 * @param Global: update pheromones after all ants have completed their paths by taking the best paths from all nodes (MPI processes)
 */
enum class PHEROMONE_UPDATE_STRATEGY {
    GLOBAL = 0,
    LOCAL = 1
};

/**
 * Generate a path for an ant
 * @param start starting point
 * @param n number of cities
 * @param adj_mat adjacency matrix
 * @param pheromones pheromones matrix
 */
AntPath generate_path(int start, int n, Table &adj_mat, Table &pheromones);

/**
 * Calculate the cost of a path
 * @param adj_mat adjacency matrix
 * @param path path to calculate the cost for
 */
double cost(const Table &adj_mat, const AntPath &path);

/**
 * Update the pheromones based on the path taken by an ant
 * @param pheromones pheromones matrix
 * @param adj_mat adjacency matrix
 * @param path path taken by an ant
 * @param strategy pheromone update strategy
 */
void update_pheromones(Table &pheromones, const Table &adj_mat, const AntPath &path, PHEROMONE_UPDATE_STRATEGY strategy);
