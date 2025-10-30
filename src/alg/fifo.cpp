#include "alg/fifo.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

// FIFO implementation optimized for large queues.
Fifo::Fifo(std::unordered_map<unsigned int, std::unique_ptr<Page>*> &ram) : IAlgorithm(ram) {}

void Fifo::onInsert(unsigned int pageId, unsigned int frameIdx) {
    // If not present, push to back
    if (mSet.find(pageId) == mSet.end()) {
        mQueue.push_back(pageId);
        mSet.insert(pageId);
    }
}

void Fifo::onAccess(unsigned int pageId) {
    // FIFO doesn't change order on access
    (void)pageId;
}

void Fifo::onEvict(unsigned int pageId, unsigned int frameIdx) {
    // remove id if present
    auto it = mSet.find(pageId);
    if (it == mSet.end()) return;
    mSet.erase(it);
    // remove from deque
    mQueue.erase(std::remove(mQueue.begin(), mQueue.end(), pageId), mQueue.end());
    (void)frameIdx;
}

std::vector<unsigned int> Fifo::execute(unsigned int pages)
{
    std::vector<unsigned int> evicted;
    if (mRam.empty() || pages == 0) return evicted;

    // Build set of current page ids in RAM and map id->index
    std::unordered_set<unsigned int> currentIds;
    currentIds.reserve(mRam.size()*2);
    std::unordered_map<unsigned int, unsigned int> idToIndex;
    idToIndex.reserve(mRam.size()*2);

    for (auto it : mRam) {
        unsigned int id = (*it.second)->id;
        currentIds.insert(id);
        idToIndex[id] = it.first;
    }

    // Filter queue: keep only ids that are still in RAM
    std::deque<unsigned int> newQueue;
    newQueue.clear();
    for (unsigned int id : mQueue) {
        if (currentIds.find(id) != currentIds.end()) {
            newQueue.push_back(id);
        }
    }
    mQueue.swap(newQueue);

    // Rebuild mSet from the filtered queue
    mSet.clear();
    mSet.reserve(mQueue.size()*2);
    for (unsigned int id : mQueue) mSet.insert(id);

    // Append any pages in mRam that are not yet in the queue
    for (const auto it : mRam) {
        unsigned int id = (*it.second)->id;
        if (mSet.find(id) == mSet.end()) {
            mQueue.push_back(id);
            mSet.insert(id);
        }
    }

    // Evict up to 'pages' from the front of the queue
    unsigned int toEvict = std::min<unsigned int>(pages, static_cast<unsigned int>(mQueue.size()));
    for (unsigned int i = 0; i < toEvict; ++i) {
        unsigned int evictId = mQueue.front();
        mQueue.pop_front();
        mSet.erase(evictId);

        auto it = idToIndex.find(evictId);
        if (it != idToIndex.end()) {
            evicted.push_back(it->second);
        }
        // if not found, skip
    }

    std::cout << "\n [FIFO]-Evicting: ";
    for (unsigned int idx : evicted) std::cout << idx << " ";
    std::cout << "\n";

    return evicted;
}