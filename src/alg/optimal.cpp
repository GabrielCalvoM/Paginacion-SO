#include "alg/optimal.h"
#include "sim/intset.h"

#include <vector>
#include <iostream>
#include <limits>
#include <algorithm>

// Constructor
Optimal::Optimal(std::vector<Page>& bufRAM, const std::vector<unsigned int>& accessSequence)
    : IAlgorithm(bufRAM), mAccessSequence(accessSequence), mCurIndex(0)
{
    for (size_t idx = 0; idx < mAccessSequence.size(); ++idx) {
        unsigned int pid = mAccessSequence[idx];
        mOccurrences[pid].push_back(idx);
    }
}

// Foresee Future (advance internal pointer / consume occurrence)
void Optimal::optForesee(unsigned int pageId)
{
    if (mCurIndex >= mAccessSequence.size()) return;

    // consume occurrences while current index matches pageId
    while (mCurIndex < mAccessSequence.size() && mAccessSequence[mCurIndex] == pageId) {
        auto &dq = mOccurrences[pageId];
        if (!dq.empty() && dq.front() == mCurIndex) dq.pop_front();
        ++mCurIndex;
    }

    // clean up any stale entries < mCurIndex
    for (auto &kv : mOccurrences) {
        auto &dq = kv.second;
        while (!dq.empty() && dq.front() < mCurIndex) dq.pop_front();
    }
}

// Execution: choose 'pages' frames to evict using Belady (farthest next use or never)
std::vector<unsigned int> Optimal::execute(const std::vector<Page> &bufRAM, unsigned int pages) 
{
    printf("\n [OPT]-Start \n");

    std::vector<unsigned int> evicted;
    if (pages == 0 || bufRAM.empty()) return evicted;

    struct Item { size_t nextPos; unsigned int frameIdx; };
    std::vector<Item> candidates;
    candidates.reserve(bufRAM.size());

    const size_t INF = std::numeric_limits<size_t>::max();

    for (unsigned int i = 0; i < bufRAM.size(); ++i) {
        unsigned int pageId = bufRAM[i].id; // use Page::id used elsewhere in code
        size_t nextPos = INF;
        auto it = mOccurrences.find(pageId);
        if (it != mOccurrences.end() && !it->second.empty()) {
            // front is the next future occurrence (>= mCurIndex)
            nextPos = it->second.front();
        }
        candidates.push_back({ nextPos, i });
    }

    std::sort(candidates.begin(), candidates.end(), [](const Item &a, const Item &b) {
        if (a.nextPos == b.nextPos) return a.frameIdx < b.frameIdx;
        return a.nextPos > b.nextPos; // farthest (or INF) first
    });

    for (unsigned int k = 0; k < pages && k < candidates.size(); ++k) {
        evicted.push_back(candidates[k].frameIdx);
    }

    printf("\n [OPT]-Finish \n");
    return evicted;
}