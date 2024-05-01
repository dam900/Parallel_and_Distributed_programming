#include <mpi.h>

#include <algorithm>
#include <iomanip>
#include <iostream>
#include <list>
#include <random>
#include <vector>

#include "../include/pugixml.hpp"
#include "mpi_pacs.hpp"

// IO functions

void print_table(const Matrix &table, bool like_float = false);
void parse_xml(const char *filename, Matrix &vertecies);
void parse_args(int argc, char **argv, int &n, char *&filename);
void print_path(const Path &path, int cost);

int main(int argc, char **argv) {
    int n;
    int num_procs;
    int rank;
    char *filename;

    MPI_Init(&argc, &argv);                     // Initialize the MPI environment
    MPI_Comm_size(MPI_COMM_WORLD, &num_procs);  // Get the number of processes
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);       // Get the rank of the process
    parse_args(argc, argv, n, filename);        // Parse cmd line arguments

    Matrix adj_mat = Matrix(n, std::vector<double>(n, 0.0));
    Matrix pheromones = Matrix(n, std::vector<double>(n, 1.0));  // Initialize the pheromones table

    // Initialize the necessary data
    if (rank == 0) {
        parse_xml(filename, adj_mat);  // Parse the xml file and print the adjacency matrix for verification
    }

    MPI_Barrier(MPI_COMM_WORLD);  // Wait until xml file is read
    for (int i = 0; i < adj_mat.size(); i++) {
        MPI_Bcast(adj_mat[i].data(), adj_mat[i].size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);        // Broadcast the adjacency matrix to all processes
        MPI_Bcast(pheromones[i].data(), pheromones[i].size(), MPI_DOUBLE, 0, MPI_COMM_WORLD);  // Broadcast the pheromones table to all processes
    }

    MPI_PACS pacs = MPI_PACS(-2.0, 0.1, 2.0, 100.0, 0.6);
    pacs.set_adj_mat(adj_mat);
    pacs.set_pheromones(pheromones);

    // Actual ACO algorithm
    // begin
    auto p = pacs.run(10, 100, n, 15);
    auto best_cost = p.first;
    auto best_path = p.second;

    // end
    if (rank == 0) {
        print_path(best_path, best_cost);
    }

    MPI_Finalize();  // Finalize the MPI environment
    return 0;
}

/**
 * Parse the xml file
 *
 * @param filename The name of the xml file
 * @param vertecies The adjacency matrix to be filled
 */
void parse_xml(const char *filename, Matrix &vertecies) {
    pugi::xml_document doc;
    if (!doc.load_file(filename)) {
        std::cerr << "Error: File not found" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }

    pugi::xml_node tsp = doc.child("travellingSalesmanProblemInstance");
    std::cout << "Name: " << tsp.child_value("name") << std::endl;
    std::cout << "Source: " << tsp.child_value("source") << std::endl;
    std::cout << "Description: " << tsp.child_value("description") << std::endl;

    int i = 0;
    pugi::xml_node graph = tsp.child("graph");
    for (auto node = graph.child("vertex"); node; node = node.next_sibling("vertex"), i++) {
        for (auto inner_node = node.child("edge"); inner_node; inner_node = inner_node.next_sibling("edge")) {
            vertecies[i][inner_node.text().as_int()] = inner_node.attribute("cost").as_double();
        }
    }
}

/**
 * Pretty print the table
 *
 * @param table The table to be printed
 * @param like_float If true, print the table with up to 4 numbers of precision
 */
void print_table(const Matrix &table, bool like_float) {
    for (int i = 0; i < table.size(); i++) {
        for (int j = 0; j < table[i].size(); j++) {
            if (like_float)
                std::cout << std::setw(4) << std::setprecision(1) << table[i][j] << " | ";
            if (!like_float)
                std::cout << std::setw(4) << std::setprecision(4) << table[i][j] << " | ";
        }
        std::cout << std::endl;
        for (int j = 0; j < table[i].size(); j++) {
            std::cout << "-------";
        }
        std::cout << std::endl;
    }
}

/**
 * Parse the command line arguments
 *
 * @param argc The number of arguments
 * @param argv The arguments
 * @param n The number of vertecies return variable
 * @param filename The name of the xml file return variable
 */
void parse_args(int argc, char **argv, int &n, char *&filename) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <n> <filename>" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    n = std::stoi(argv[1]);
    filename = argv[2];
}

/**
 * Pretty print the path
 *
 * @param path The path to be printed
 */
void print_path(const Path &path, int cost) {
    std::cout << "Path< ";
    for (auto it = path.begin(); it != path.end(); it++) {
        std::cout << *it;
        if (std::next(it) != path.end()) {
            std::cout << " -> ";
        }
    }
    std::cout << " >End" << std::endl;
    std::cout << "Cost: " << cost << std::endl;
}