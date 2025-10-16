#ifndef MRU_H
#define MRU_H

#include <vector>
#include <queue>

#include "sim/page.h"
#include "alg/algorithm.h"

class Mru : public IAlgorithm {
public:
    Mru(std::vector<Page>&);
    FrameResult execute(std::vector<unsigned int>);
};

#endif // MRU_H