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

    void mruUpdate(unsigned int pageId);
    void markUsage(unsigned int pageId);
    
private:
    std::deque<unsigned int> mQueue;
    std::unordered_set<unsigned int> mLookUp;
};

#endif // MRU_H
