#ifndef LRU_H
#define LRU_H

#include <vector>
#include <queue>
#include <unordered_map>

#include "sim/page.h"
#include "alg/algorithm.h"

class Lru : public IAlgorithm {
public:
    Lru(std::vector<Page>&);
    std::vector<unsigned int> execute(const std::vector<Page> &bufRAM, unsigned int pages) override;
    void optForesee(unsigned int pageId) override;
    
private:
    unsigned long long mClock = 0;
    std::unordered_map<unsigned int, unsigned long long> mLastAccess;
        
};

#endif // LRU_H
