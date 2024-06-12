#pragma once
#include <string>
#include <vector>
typedef std::vector<std::vector<int>> IntMatrix;

class NehDataReader {
   public:
    IntMatrix fromDataFile(std::string& filename);
};
