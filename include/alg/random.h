#ifndef RANDOM_H
#define RANDOM_H

#include <vector>
#include <queue>
#include <random>

#include "sim/page.h"
#include "alg/algorithm.h"

class Random : public IAlgorithm {
private:
    // mersenne twister thing
    std::mt19937 mRng;
public:
    Random(std::vector<Page>&, unsigned int seed = 0);
    std::vector<unsigned int> execute(const std::vector<Page> &bufRAM, unsigned int pages) override;
    
};

#endif // RANDOM_H
