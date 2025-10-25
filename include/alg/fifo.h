#ifndef FIFO_H
#define FIFO_H

#include <vector>
#include <queue>

#include "sim/page.h"
#include "alg/algorithm.h"

class Fifo : public IAlgorithm {
public:
    Fifo(std::vector<Page>&);
    std::vector<unsigned int> execute(const std::vector<Page> &bufRAM, unsigned int pages) override;
private:
    unsigned int mNext = 0; // indice circular del siguiente frame a evictar
};

#endif // FIFO_H
