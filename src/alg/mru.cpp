#include <algorithm>
#include "alg/mru.h"
#include <vector>
#include <iostream>
#include <unordered_set>
#include <unordered_map>

////////////////////////////////////////////////////////////////////////
// Constructor
Mru::Mru(std::unordered_map<unsigned int, std::unique_ptr<Page>*> &ram) : IAlgorithm(ram) {}

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

void Mru::onInsert(unsigned int pageId, unsigned int frameIdx) {
    (void)frameIdx;
    mruUpdate(pageId);
}

void Mru::onAccess(unsigned int pageId) {
    mruUpdate(pageId);
}

void Mru::onEvict(unsigned int pageId, unsigned int frameIdx) {
    (void)frameIdx;
    auto it = mSet.find(pageId);
    if (it == mSet.end()) return;
    mSet.erase(it);
    mQueue.erase(std::remove(mQueue.begin(), mQueue.end(), pageId), mQueue.end());
}

////////////////////////////////////////////////////////////////////////
// EXECUTION
std::vector<unsigned int> Mru::execute(unsigned int pages) {
    
    std::vector<unsigned int> evicted;
    if (mRam.empty() || pages == 0) return evicted;

    // Data Structure Implementation
    std::unordered_set<unsigned int> currentIds;
    currentIds.reserve(mRam.size() * 2);
    std::unordered_map<unsigned int, unsigned int> idToIndex;
    idToIndex.reserve(mRam.size() * 2);

    for (auto it : mRam) {
        unsigned int id = (*it.second)->id;
        currentIds.insert(id);
        idToIndex[id] = it.first;
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
    for (const auto &it : mRam) {
        unsigned int id = (*it.second)->id;
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

        // Buscar por el id de página
        auto it = idToIndex.find(evId);
        if (it != idToIndex.end()) {
            evicted.push_back(it->second);
        }
    }

    
    return evicted;
}
