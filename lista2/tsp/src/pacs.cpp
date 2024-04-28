
#include "pacs.hpp"

#include <math.h>

#include <iostream>
#include <limits>
#include <map>

static double BETA = -2.0;  // Distance importance
static double RHO = 0.3;    // Evaporation rate
static double THETA = 3.0;  // Pheromone deposit amount
static double TAU = 0.6;    // Initial pheromone level
static double GAMMA = 0.5;
static double Q = 100.0;

void set_params(double beta, double rho, double theta, double gamma, double q, double tau) {
    BETA = beta;
    RHO = rho;
    THETA = theta;
    GAMMA = gamma;
    Q = q;
    TAU = tau;
}

// Helper functions

static void local_update_strategy(Table &pheromones, const Table &adj_mat, const AntPath &path);
static void global_update_strategy(Table &pheromones, const Table &adj_mat, const AntPath &path);
static std::list<int> construct_unvisited_list(int start, int n);

AntPath generate_path(int start, int n, Table &adj_mat, Table &pheromones) {
    AntPath path;
    path.push_back(start);  // Random starting point
    std::list<int> unvisited = construct_unvisited_list(start, n);
    auto current = start, next_dest = -1;

    while (unvisited.size() > 0) {
        auto best = std::numeric_limits<double>::max();
        for (auto city : unvisited) {
            // calculate the probability of moving to the next city
            double prob = pheromones[current][city] * pow((1 / adj_mat[current][city]), BETA);
            if (prob < best) {
                best = prob;
                next_dest = city;
            }
        }
        path.push_back(next_dest);    // move to the next city
        current = next_dest;          // update the current city
        unvisited.remove(next_dest);  // remove the city from the unvisited list
    }
    path.push_back(start);  // comback to the start
    return path;
}

double cost(const Table &adj_mat, const AntPath &path) {
    double cost = 0.0;
    for (auto it = path.begin(); it != path.end(); it++) {
        auto next = std::next(it);
        if (next == path.end()) continue;
        cost += adj_mat[*it][*next];
    }
    return cost;
}

// link to the article
// https://sci-hub.se/https://www.sciencedirect.com/science/article/abs/pii/S0020025503004110

void update_pheromones(Table &pheromones, const Table &adj_mat, const AntPath &path, PHEROMONE_UPDATE_STRATEGY strategy) {
    switch (strategy) {
        case PHEROMONE_UPDATE_STRATEGY::LOCAL:
            local_update_strategy(pheromones, adj_mat, path);
            break;
        case PHEROMONE_UPDATE_STRATEGY::GLOBAL:
            global_update_strategy(pheromones, adj_mat, path);
            break;
        default:
            throw std::runtime_error("Invalid pheromone update strategy");
            break;
    }
}

// Helper functions

static void local_update_strategy(Table &pheromones, const Table &adj_mat, const AntPath &path) {
    auto num_cities = path.size();
    auto path_cost = cost(adj_mat, path);
    for (auto it = path.begin(); it != path.end(); it++) {
        auto next = std::next(it);
        if (next == path.end()) continue;
        pheromones[*it][*next] = (1 - RHO) * pheromones[*it][*next] + (RHO * TAU);
    }
}

static void global_update_strategy(Table &pheromones, const Table &adj_mat, const AntPath &path) {
    auto path_cost = cost(adj_mat, path);
    for (auto it = path.begin(); it != path.end(); it++) {
        auto next = std::next(it);
        if (next == path.end()) continue;
        pheromones[*it][*next] = (1 - RHO) * pheromones[*it][*next] + THETA * (Q / path_cost);
    }
}

static std::list<int> construct_unvisited_list(int start, int n) {
    std::list<int> unvisited;
    for (int i = 0; i < n; i++)
        if (i != start) unvisited.emplace_back(i);
    return unvisited;
}