
#include "aco_utils.hpp"

#include <iostream>
#include <limits>

AntPath generate_path(int start, int n, Table &adj_mat) {
    AntPath path;
    path.push_back(start);  // Generate a random starting point
    std::list<int> unvisited;
    for (int i = 0; i < n; i++)
        if (i != start)
            unvisited.emplace_back(i);

    auto current = start, next_dest = -1;
    while (unvisited.size() > 0) {
        auto best = std::numeric_limits<double>::max();
        for (auto city : unvisited) {
            if (adj_mat[current][city] < best) {
                best = adj_mat[current][city];
                next_dest = city;
            }
        }
        path.push_back(next_dest);
        current = next_dest;
        unvisited.remove(next_dest);
    }
    path.push_back(start);
    return path;
}

double cost(const Table &adj_mat, const Table &pheromones, const AntPath &path) {
    double cost = 0.0;
    for (auto it = path.begin(); it != path.end(); it++) {
        auto next = std::next(it);
        if (next == path.end()) continue;
        cost += adj_mat[*it][*next] * pheromones[*it][*next];
    }
    return cost;
}