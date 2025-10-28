#ifndef MRU_H
#define MRU_H

#include <vector>
#include <queue>

#include "sim/page.h"
#include "alg/algorithm.h"
#include <unordered_set>
#include <unordered_map>

class Mru : public IAlgorithm {
public:
    Mru(std::vector<Page>&);
    std::vector<unsigned int> execute(const std::vector<Page> &bufRAM, unsigned int pages) override;

    void mruUpdate(unsigned int pageId);
    void markUsage(unsigned int pageId);
    void onInsert(unsigned int pageId, unsigned int frameIdx) override;
    void onAccess(unsigned int pageId) override;
    void onEvict(unsigned int pageId, unsigned int frameIdx) override;
    
private:
    std::deque<unsigned int> mQueue;
    std::unordered_set<unsigned int> mSet;
};

#endif // MRU_H
