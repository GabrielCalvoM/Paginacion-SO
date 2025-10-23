#ifndef MRU_H
#define MRU_H

#include <vector>
#include <queue>

#include "sim/page.h"
#include "alg/algorithm.h"

class Mru : public IAlgorithm {
public:
    Mru(std::vector<Page>&);
    std::vector<unsigned int> execute(const std::vector<Page> &bufRAM, unsigned int pages) override;
    
};

#endif // MRU_H
