#pragma once
#include <string>
#include <vector>
typedef std::vector<std::vector<int>> IntMatrix;

class QapDataReader {
   public:
    std::tuple<int, IntMatrix, IntMatrix> fromDataFile(std::string& filename);
};
