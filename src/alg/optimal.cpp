#include "alg/optimal.h"
#include "sim/intset.h"

#include <vector>
#include <iostream>
#include <limits>


// Constructor
Optimal::Optimal(std::vector<Page>& bufRAM, IntSet& set, const std::vector<unsigned int>& pages) 
    : IAlgorithm(bufRAM), mAccessSequence(accessSequence), mCurIndex(0)
{
    for (size_t idx = 0; idx < mAccessSequence.size(); ++idx) {
        unsigned int pid = mAccessSequence[idx];
        mOccurrences[pid].push_back(idx);
    }
}

// Foresee Future
Optimal::optForesee(unsigned int pageId) 
{
    if (mCurIndex >= mAccessSequence.size()) return;

    while (mCurIndex < mAccessSequence.size() && mAccessSequence[mCurIndex] == pageId) {
        auto &dq = mOccurrences[pageId];
        if (!dq.empty() && dq.front() == mCurIndex) dq.pop_front();
        ++mCurIndex;
    }

    for (auto &kv : mOccurrences) {
        auto &dq = kv.second;
        while (!dq.empty() && dq.front() < mCurIndex) dq.pop_front();
    }
}

// Execution
std::vector<unsigned int> Optimal::execute(const std::vector<Page> &bufRAM, unsigned int pages) 
{
    printf("\n [OPT]-Start \n");

    std::vector<unsigned int> evicted;
    if (pages == 0 || bufRAM.empty()) return evicted;

    struct Item { size_t nextPos; unsigned int frameIdx; };
    std::vector<Item> candidates;
    candidates.reserve(bufRAM.size());

    for (unsigned int i = 0; i < bufRAM.size(); ++i) {
        unsigned int pid = bufRAM[i].pid;
        size_t nextPos = numeric_limits<size_t>::max();
    }

    std::sort(candidates.begin(), candidates.end(), [](const Item &a, const Item &b) {
        if (a.nextPos == b.nextPos) return a.frameIdx < b.frameIdx;
        return a.nextPos > b.nextPos;
    });

    for (unsigned int k = 0; k < pages && k < candidates.size(); ++k) {
        evicted.push_back(candidates[k].frameIdx);
    }

    // DEBUG 
    /*/
    printf(" [OPT]-Evicting Pages: ");
    for (unsigned int idx : evicted) {  
        std::cout << idx << " ";
    }
    // */

    printf("\n [OPT]-Finish \n");
    return evicted;
}
