#include <algorithm>
#include "alg/mru.h"
#include <vector>
#include <iostream>
#include <unordered_set>
#include <unordered_map>

////////////////////////////////////////////////////////////////////////
// Constructor
Mru::Mru(std::vector<Page> &ram) : IAlgorithm(ram) {}

////////////////////////////////////////////////////////////////////////
// AUX MRU UPDATE - place a page to the front
void Mru::mruUpdate(unsigned int pageId) {
    // existing pages moved
    auto it = mSet.find(pageId);
    if (it != mSet.end()) {
        mQueue.erase(std::remove(mQueue.begin(), mQueue.end(), pageId),
                     mQueue.end());
    }
    // new pages directly
    else {
        mSet.insert(pageId); 
    }

    mQueue.push_front(pageId);
}

// AUX MARK USAGE - explicitly mark a page for the front
void Mru::markUsage(unsigned int pageId) {
    mruUpdate(pageId);
}

////////////////////////////////////////////////////////////////////////
// EXECUTION
std::vector<unsigned int> Mru::execute(const std::vector<Page>& bufRAM, unsigned int pages) {
    printf("\n [MRU]-Start \n");
    std::vector<unsigned int> evicted;
    if (bufRAM.empty() || pages == 0) return evicted;

    // Data Structure Implementation
    std::unordered_set<unsigned int> currentIds;
    currentIds.reserve(bufRAM.size() * 2);
    std::unordered_map<unsigned int, unsigned int> idToIndex;
    idToIndex.reserve(bufRAM.size() * 2);

    for (unsigned int idx = 0; idx < bufRAM.size(); ++idx) {
        unsigned int id = bufRAM[idx].id;
        currentIds.insert(id);
        idToIndex[id] = idx;
    }

    // Filter Queue to keep only RAM
    std::deque<unsigned int> newQueue;
    newQueue.clear();
    for (unsigned int id : mQueue) {
        if (currentIds.find(id) != currentIds.end()) {
            newQueue.push_back(id);
        }
    }
    mQueue.swap(newQueue);

    // Rebuild Set
    mSet.clear();
    mSet.reserve(mQueue.size() * 2);
    for (const Page &p : bufRAM) {
        unsigned int id = p.id;
        if (mSet.find(id) == mSet.end()) {
            mQueue.push_back(id);
            mSet.insert(id);
        }
    }

    // Evict from Front of the Queue (stack despichado)
    unsigned int ev = std::min<unsigned int>(pages, static_cast<unsigned int>(mQueue.size()));

    for (unsigned int i = 0; i < ev; ++i) {
        unsigned int evId = mQueue.front();
        mQueue.pop_front();
        mSet.erase(evId);

        auto it = idToIndex.find(ev);
        if (it != idToIndex.end()) {
            evicted.push_back(it->second);
        }
    }

    std::cout << "\n [MRU]-Evicting: ";
    for (unsigned int idx : evicted) std::cout << idx << " ";
    std::cout << "\n";

    printf("\n [MRU]-Finish \n");
    return evicted;
}
