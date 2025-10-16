#ifndef FIFO_H
#define FIFO_H

#include <vector>
#include <queue>

#include "sim/page.h"
#include "alg/algorithm.h"

class Fifo : public IAlgorithm {
public:
    Fifo(std::vector<Page>&);
    FrameResult execute(std::vector<unsigned int>);
};

#endif // FIFO_H