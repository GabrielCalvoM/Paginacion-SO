#ifndef RANDOM_H
#define RANDOM_H

#include <vector>
#include <queue>

#include "sim/page.h"
#include "alg/algorithm.h"

class Random : public IAlgorithm {
public:
    Random(std::vector<Page>&);
    std::vector<unsigned int> execute(const std::vector<Page> &bufRAM, unsigned int pages) override;
    
};

#endif // RANDOM_H
