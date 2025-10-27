#ifndef FIFO_H
#define FIFO_H

#include <vector>
#include <deque>
#include <unordered_set>

#include "sim/page.h"
#include "alg/algorithm.h"

class Fifo : public IAlgorithm {
public:
    Fifo(std::vector<Page>&);
    std::vector<unsigned int> execute(const std::vector<Page> &bufRAM, unsigned int pages) override;
private:
    // FIFO queue of page ids present in RAM, front = oldest
    
    std::deque<unsigned int> mQueue;
    std::unordered_set<unsigned int> mSet;
};

#endif // FIFO_H
