#include "qap_data_reader.hpp"

#include <fstream>
#include <iostream>

std::tuple<int, IntMatrix, IntMatrix> QapDataReader::fromDataFile(std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }
    int n;  // Dimension of the Matricies (n by n)
    file >> n;
    IntMatrix flowMatrix(n, std::vector<int>(n));
    IntMatrix distanceMatrix(n, std::vector<int>(n));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file >> flowMatrix[i][j];
        }
    }
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file >> distanceMatrix[i][j];
        }
    }

    return std::tuple(n, flowMatrix, distanceMatrix);
}
