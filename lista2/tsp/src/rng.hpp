#include <random>

class IntRNG {
   public:
    IntRNG(int min, int max) : min(min), max(max) {
        std::random_device rd;
        gen = std::mt19937(rd());
        dist = std::uniform_int_distribution<int>(min, max);
    }

    int getNext() {
        return dist(gen);
    }

   private:
    int min;
    int max;
    std::mt19937 gen;
    std::uniform_int_distribution<int> dist;
};

class DoubleRNG {
   public:
    DoubleRNG(double min, double max) : min(min), max(max) {
        std::random_device rd;
        gen = std::mt19937(rd());
        dist = std::uniform_real_distribution<double>(min, max);
    }

    double getNext() {
        return dist(gen);
    }

   private:
    double min;
    double max;
    std::mt19937 gen;
    std::uniform_real_distribution<double> dist;
};