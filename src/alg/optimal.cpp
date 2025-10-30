#include <algorithm>
#include "alg/optimal.h"
#include "sim/intset.h"

#include <vector>
#include <iostream>
#include <limits>
#include <unordered_set>


// Constructor
Optimal::Optimal(std::unordered_map<unsigned int, std::unique_ptr<Page>*>& mRam, const std::vector<unsigned int>& accessSequence)
    : IAlgorithm(mRam), mAccessSequence(accessSequence), mCurIndex(0)
{
    // Ocurrence Map Build
    for (size_t idx = 0; idx < mAccessSequence.size(); ++idx) {
        unsigned int pid = mAccessSequence[idx];
        mOccurrences[pid].push_back(idx);
    }
}

// Foresee Future (advance internal pointer / consume occurrence)
void Optimal::optForesee(unsigned int pageId)
{
    if (mCurIndex < mAccessSequence.size() && mAccessSequence[mCurIndex] == pageId) {
        ++mCurIndex;
    }
}

void Optimal::onAccess(unsigned int pageId) {
    optForesee(pageId);
}

void Optimal::onInsert(unsigned int pageId, unsigned int idx) {
    optForesee(pageId);
}


// Execution: evict using Belady (farthest next use or never)
std::vector<unsigned int> Optimal::execute(unsigned int pages) 
{
    printf("\n [OPT]-Start \n");

    std::vector<unsigned int> evicted;
    if (pages == 0 || mRam.empty()) return evicted;

    // track Frames
    std::vector<unsigned int> frames;
    for (auto it : mRam) {
        frames.push_back(it.first);
    }

    // Scan access sequence
    for (size_t i = mCurIndex; i < mAccessSequence.size(); ++i) {
        unsigned int futurePageId = mAccessSequence[i];

        // Remove frames when their page is found
        for (size_t j = 0; j < frames.size(); ++j) {
            unsigned int f = frames[j];
            if ((*mRam[f])->id == futurePageId) {
                frames.erase(frames.begin() + j);
                break;  // Only remove first match
            }
        }

        // exit once frame equals the requested pages
        if (frames.size() <= pages) break;
    }

    // Evict from frame those who will no longer be used
    for (unsigned int k = 0; k < pages && k < frames.size(); ++k) {
        evicted.push_back(frames[k]);
    }

    // Safeguard to ensure perfect Evict
    if (evicted.size() < pages) {
        // This shouldn't normally happen, but as a safeguard:
        std::unordered_set<unsigned int> alreadyEvicted(evicted.begin(), evicted.end());
        for (unsigned int i = 0; i < mRam.size() && evicted.size() < pages; ++i) {
            if (alreadyEvicted.find(i) == alreadyEvicted.end()) {
                evicted.push_back(i);
            }
        }
    }

    std::cout << "\n [OPT]-Evicting: ";
    for (unsigned int idx : evicted) std::cout << idx << " ";
    std::cout << "\n";

    printf("\n [OPT]-Finish \n");
    return evicted;
}
