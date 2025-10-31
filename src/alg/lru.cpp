#include "alg/lru.h"

#include <vector>
#include <algorithm>
#include <iostream>
#include <unordered_map>

// LRU implementation using an incrementing timestamp updated on page accesses.
// optForesee(pageId) is used by MMU to notify the algorithm of a page access.

Lru::Lru(std::unordered_map<unsigned int, std::unique_ptr<Page>*> &ram) : IAlgorithm(ram), mClock(0) {}

void Lru::optForesee(unsigned int pageId) {
    // record the access time for this page id
    mClock++;
    mLastAccess[pageId] = mClock;
}

void Lru::onInsert(unsigned int pageId, unsigned int frameIdx) {
    (void)frameIdx;
    mClock++;
    mLastAccess[pageId] = mClock;
}

void Lru::onAccess(unsigned int pageId) {
    // treat access the same as optForesee
    optForesee(pageId);
}

void Lru::onEvict(unsigned int pageId, unsigned int frameIdx) {
    (void)frameIdx;
    auto it = mLastAccess.find(pageId);
    if (it != mLastAccess.end()) mLastAccess.erase(it);
}

std::vector<unsigned int> Lru::execute(unsigned int pages) {
    std::vector<unsigned int> evicted;
    if (mRam.empty() || pages == 0) return evicted;

    unsigned int frameCount = static_cast<unsigned int>(mRam.size());
    unsigned int need = std::min<unsigned int>(pages, frameCount);

    // Build vector of (lastAccess, frameIdx)
    std::vector<std::pair<unsigned long long, unsigned int>> items;
    items.reserve(frameCount);
    for (unsigned int i = 0; i < frameCount; ++i) {
        unsigned int pid = (*mRam[i])->id;
        unsigned long long ts = 0;
        auto it = mLastAccess.find(pid);
        if (it != mLastAccess.end()) ts = it->second;
        items.emplace_back(ts, i);
    }

    // Sort by timestamp ascending (oldest first). Tie-break by frame index.
    std::sort(items.begin(), items.end(), [](const auto &a, const auto &b){
        if (a.first == b.first) return a.second < b.second;
        return a.first < b.first;
    });

    for (unsigned int k = 0; k < need && k < items.size(); ++k) {
        evicted.push_back(items[k].second);
    }

    std::cout << "\n [LRU]-Evicting: ";
    for (unsigned int idx : evicted) std::cout << idx << " ";
    std::cout << "\n";

    return evicted;
}
