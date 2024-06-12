#include "neh_data_reader.hpp"

#include <fstream>
#include <iostream>

IntMatrix NehDataReader::fromDataFile(std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Could not open file");
    }

    int n;
    file >> n;
    int m;
    file >> m;
    IntMatrix tasks(n, std::vector<int>(m));
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < m; j++) {
            file >> tasks[i][j];
        }
    }
    return tasks;
}
