#include "alg/second_chance.h"

#include <set>
#include <vector>
#include <iostream>

// Maintains a hand (index) into mRam and gives a "second chance" to pages
// that have their second-chance bit set.

SecondChance::SecondChance(std::unordered_map<unsigned int, std::unique_ptr<Page>*> &ram) : IAlgorithm(ram), mHand(0) {}

std::vector<unsigned int> SecondChance::execute(unsigned int pages)
{
    std::vector<unsigned int> evicted;
    if (mRam.empty() || pages == 0) return evicted;

    // Build a stable vector of frame indices (keys)
    std::vector<unsigned int> frames;
    frames.reserve(mRam.size());
    for (const auto &entry : mRam) frames.push_back(entry.first);

    if (frames.empty()) return evicted;

    // Normalize hand to range
    if (mHand >= frames.size()) mHand = 0;

    unsigned int need = std::min<unsigned int>(pages, static_cast<unsigned int>(frames.size()));

    // Walk the circular list until we collect 'need' frames to evict
    while (evicted.size() < need && !frames.empty()) {
        // ensure hand in range after removals
        if (mHand >= frames.size()) mHand = 0;

        unsigned int frameIdx = frames[mHand];
        auto itMap = mRam.find(frameIdx);
        if (itMap == mRam.end() || itMap->second == nullptr) {
            // If map entry vanished or pointer null, remove this frame from list
            frames.erase(frames.begin() + mHand);
            continue;
        }

        std::unique_ptr<Page>* uptr = itMap->second;
        if (!uptr || !(*uptr)) {
            // defensive: remove invalid entries
            frames.erase(frames.begin() + mHand);
            continue;
        }

        Page *page = uptr->get();
        if (page->hasSecondChance()) {
            // give second chance: clear bit and advance hand
            page->setSecondChance(false);
            mHand = (mHand + 1) % frames.size();
        } else {
            // select this frame for eviction; remove from frames so we don't revisit
            evicted.push_back(frameIdx);
            frames.erase(frames.begin() + mHand);
            // mHand stays at same index which now points to the next element;
            if (!frames.empty()) mHand %= frames.size();
        }
    }


    return evicted;
}