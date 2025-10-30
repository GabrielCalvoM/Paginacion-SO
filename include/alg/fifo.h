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
    std::vector<unsigned int> execute(unsigned int pages) override;
    void onInsert(unsigned int pageId, unsigned int frameIdx) override;
    void onAccess(unsigned int pageId) override;
    void onEvict(unsigned int pageId, unsigned int frameIdx) override;
private:
    // FIFO queue of page ids present in RAM, front = oldest
    
    std::deque<unsigned int> mQueue;
    std::unordered_set<unsigned int> mSet;
};

#endif // FIFO_H
