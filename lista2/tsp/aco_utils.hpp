#pragma once
#include <list>
#include <vector>

#define ALPHA 1.0
#define BETA 2.0
#define RHO 0.5
#define Q 1.0
#define MAX_ITER 2
#define NUM_ANTS 10

typedef std::vector<std::vector<double>> Table;
typedef std::list<int> AntPath;

AntPath generate_path(int start, int n, Table &adj_mat);
double cost(const Table &adj_mat, const Table &pheromones, const AntPath &path);