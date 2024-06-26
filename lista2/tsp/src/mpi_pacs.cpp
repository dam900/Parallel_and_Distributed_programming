
#include "mpi_pacs.hpp"

#include <math.h>
#include <mpi.h>

#include <iostream>
#include <limits>
#include <map>
#include <random>

#include "rng.hpp"

std::pair<double, Path> MPI_PACS::run(int num_ants, int num_iter, int num_cities, int comm_freq, double timeout) {
    IntRNG city_rng(0, num_cities - 1);

    auto best_cost = std::numeric_limits<double>::max();  // Start with a high cost
    auto best_path = Path();                              // Start with empty path

    double start = MPI_Wtime();

    for (int iter = 0; iter < num_iter; iter++) {  // Main loop
        if (MPI_Wtime() - start >= timeout) {
            break;
        }
        for (int i = 0; i < num_ants; i++) {
            int start = city_rng.getNext();                // generate random starting point for single ant
            auto path = generate_path(start, num_cities);  // generate path for single ant
            auto path_cost = cost(path);                   // calculate the cost of the path
            if (path_cost < best_cost) {                   // update the best path if the current path is better
                best_cost = path_cost;
                best_path = path;
            }
            update_pheromones(path, PHEROMONE_UPDATE_STRATEGY::LOCAL);  // update pheromones::local
        }
        update_pheromones(best_path, PHEROMONE_UPDATE_STRATEGY::GLOBAL);  // update pheromones::global

        if (iter % comm_freq == 0) {
            MPI_Barrier(MPI_COMM_WORLD);
            double global_best_cost;
            MPI_Allreduce(&best_cost, &global_best_cost, 1, MPI_DOUBLE, MPI_MIN, MPI_COMM_WORLD);
            int best_cost_rank = rank;
            if (global_best_cost == best_cost) {
                for (int i = 0; i < num_procs; i++) {
                    if (i != rank) {
                        MPI_Send(&rank, 1, MPI_INT, i, 0, MPI_COMM_WORLD);
                    }
                }
            } else {
                MPI_Status status;
                MPI_Recv(&best_cost_rank, 1, MPI_INT, MPI_ANY_SOURCE, MPI_ANY_TAG, MPI_COMM_WORLD, &status);
            }
            MPI_Barrier(MPI_COMM_WORLD);
            for (int i = 0; i < pheromones.size(); i++) {
                MPI_Bcast(pheromones[i].data(), pheromones[i].size(), MPI_DOUBLE, best_cost_rank, MPI_COMM_WORLD);  // Broadcast the pheromones table to all processes
            }
            best_cost = global_best_cost;
        }
    }

    return {best_cost, best_path};
}
MPI_PACS::MPI_PACS(double beta, double rho, double theta, double q, double tau) {
    BETA = beta;
    RHO = rho;
    THETA = theta;
    Q = q;
    TAU = tau;
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
}

void MPI_PACS::set_adj_mat(const Matrix &adj_mat) {
    this->adj_mat = adj_mat;
}

void MPI_PACS::set_pheromones(const Matrix &pheromones) {
    this->pheromones = pheromones;
}

Path MPI_PACS::generate_path(int start, int n) {
    Path path;
    path.push_back(start);  // Random starting point
    std::list<int> unvisited = construct_unvisited_list(start, n);
    auto current = start, next_dest = -1;

    DoubleRNG action = DoubleRNG(0.0, 1.0);

    while (unvisited.size() > 0) {
        auto best = std::numeric_limits<double>::max();
        for (auto city : unvisited) {
            if (action.getNext() < 0.5) {
                // If the random number is less than 0.5, use greedy selection
                double prob = pheromones[current][city] * pow((1 / adj_mat[current][city]), BETA);
                if (prob < best) {
                    best = prob;
                    next_dest = city;
                }
            } else {
                // Otherwise, use probabilistic selection
                double full_prob = 0.0;
                double prob = pheromones[current][city] * pow((1 / adj_mat[current][city]), BETA);  // probability of moving to the city
                for (auto city_id : unvisited) {
                    if (city_id == city) continue;
                    full_prob += pheromones[current][city_id] * pow((1 / adj_mat[current][city_id]), BETA);
                }
                if (prob / full_prob < best) {
                    best = prob / full_prob;
                    next_dest = city;
                }
            }
        }
        if (current != next_dest) {
            unvisited.remove(next_dest);  // remove the city from the unvisited list
            path.push_back(next_dest);    // move to the next city
            current = next_dest;          // update the current city
        }
    }
    path.push_back(start);  // comback to the start
    return path;
}

double MPI_PACS::cost(const Path &path) {
    double cost = 0.0;
    for (auto it = path.begin(); it != path.end(); it++) {
        auto next = std::next(it);
        if (next == path.end()) continue;
        cost += adj_mat[*it][*next];
    }
    return cost;
}

void MPI_PACS::update_pheromones(const Path &path, PHEROMONE_UPDATE_STRATEGY strategy) {
    switch (strategy) {
        case PHEROMONE_UPDATE_STRATEGY::LOCAL:
            local_update_strategy(path);
            break;
        case PHEROMONE_UPDATE_STRATEGY::GLOBAL:
            global_update_strategy(path);
            break;
        default:
            throw std::runtime_error("Invalid pheromone update strategy");
            break;
    }
}

// Helper functions

void MPI_PACS::local_update_strategy(const Path &path) {
    auto num_cities = path.size();
    auto path_cost = cost(path);
    for (auto it = path.begin(); it != path.end(); it++) {
        auto next = std::next(it);
        if (next == path.end()) continue;
        pheromones[*it][*next] = (1 - RHO) * pheromones[*it][*next] + (RHO * TAU);
    }
}

void MPI_PACS::global_update_strategy(const Path &path) {
    auto path_cost = cost(path);
    for (auto it = path.begin(); it != path.end(); it++) {
        auto next = std::next(it);
        if (next == path.end()) continue;
        pheromones[*it][*next] = (1 - RHO) * pheromones[*it][*next] + THETA * (Q / path_cost);
    }
}

Path MPI_PACS::construct_unvisited_list(int start, int n) {
    std::list<int> unvisited;
    for (int i = 0; i < n; i++)
        if (i != start) unvisited.push_back(i);
    return unvisited;
}