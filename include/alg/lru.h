#ifndef LRU_H
#define LRU_H

#include <vector>
#include <queue>

#include "sim/page.h"
#include "alg/algorithm.h"

class Lru : public IAlgorithm {
public:
    Lru(std::vector<Page>&);
    FrameResult execute(std::vector<unsigned int>);
};

#endif // LRU_H