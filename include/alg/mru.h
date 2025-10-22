#ifndef MRU_H
#define MRU_H

#include <vector>
#include <queue>

#include "sim/page.h"
#include "alg/algorithm.h"

class Mru : public IAlgorithm {
public:
    Mru(std::vector<Page>&);
    FrameResult execute(const std::vector<unsigned int>&) override;

};

#endif // MRU_H
