#include <iostream>
#include "include/pugixml.hpp"
#include <mpi.h>
#include <vector>
#include <random>
#include <algorithm>
#include <iomanip>

#define DEBUG 1

typedef std::vector<std::vector<double>> Table;

void pretty_print(const Table& table, bool like_float = false);
void parse_xml(const char* filename, Table& vertecies);
void parse_args(int argc, char** argv, int& n, char* &filename);

int main(int argc, char** argv) {
    MPI_Init(&argc, &argv); // Initialize the MPI environment
    int num_procs; MPI_Comm_size(MPI_COMM_WORLD, &num_procs); // Get the number of processes
    int rank; MPI_Comm_rank(MPI_COMM_WORLD, &rank); // Get the rank of the process
    int n; char* filename; parse_args(argc, argv, n, filename); // Parse cmd line arguments

    std::uniform_real_distribution<double> dist(0, 1); // distribution in range [0, 1]
    std::random_device rd;
    std::mt19937 gen(rd());

    Table adj_mat = Table(n, std::vector<double>(n, 0.0)); 
    Table pheromones = Table(n, std::vector<double>(n, 0.0));

    if (rank == 0) { 
        for (auto &row : pheromones) {
            std::generate(row.begin(), row.end(), [&]() { return dist(gen); });
        }
        parse_xml(filename, adj_mat); // Parse the xml file and print the adjacency matrix for verification
        if (DEBUG) pretty_print(adj_mat);
        if (DEBUG) pretty_print(pheromones, true);
    } 
    MPI_Barrier(MPI_COMM_WORLD); // Wait until xml file is read
    for (int i = 0; i < adj_mat.size(); i++) { 
        MPI_Bcast(adj_mat[i].data(), adj_mat[i].size(), MPI_DOUBLE, 0, MPI_COMM_WORLD); // Broadcast the adjacency matrix to all processes
        MPI_Bcast(pheromones[i].data(), pheromones[i].size(), MPI_DOUBLE, 0, MPI_COMM_WORLD); // Broadcast the pheromones table to all processes
    }


    MPI_Finalize(); // Finalize the MPI environment 
    return 0;
}

void parse_xml(const char* filename, Table& vertecies) {
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

void pretty_print(const Table& table, bool like_float) {
    for (int i = 0; i < table.size(); i++) {
        for (int j = 0; j < table[i].size(); j++) {
            if (like_float) std::cout << std::setw(4) << std::setprecision(1) << table[i][j] << " | ";
            if (!like_float) std::cout << std::setw(4) << table[i][j] << " | ";
        }
        std::cout << std::endl;
        for (int j = 0; j < table[i].size(); j++) {
            std::cout << "-------";
        }
        std::cout << std::endl;
    }
}

void parse_args(int argc, char** argv, int& n, char* &filename) {
    if (argc != 3) {
        std::cerr << "Usage: " << argv[0] << " <n> <filename>" << std::endl;
        MPI_Abort(MPI_COMM_WORLD, 1);
    }
    n = std::stoi(argv[1]);
    filename = argv[2];
}