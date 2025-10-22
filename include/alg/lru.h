#ifndef LRU_H
#define LRU_H

#include <vector>
#include <queue>

#include "sim/page.h"
#include "alg/algorithm.h"

class Lru : public IAlgorithm {
public:
    Lru(std::vector<Page>&);
    FrameResult execute(const std::vector<unsigned int>&) override;
    
};

#endif // LRU_H
