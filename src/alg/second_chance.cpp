#include "alg/second_chance.h"

#include <vector>
#include <iostream>
#include <algorithm>

// Maintains a hand (index) into mRam and gives a "second chance" to pages
// that have their second-chance bit set.

SecondChance::SecondChance(std::vector<Page> &ram) : IAlgorithm(ram), mHand(0) {}

std::vector<unsigned int> SecondChance::execute(unsigned int pages)
{
    std::vector<unsigned int> evicted;
    if (mRam.empty() || pages == 0) return evicted;
    unsigned int frameCount = static_cast<unsigned int>(mRam.size());
    unsigned int need = std::min<unsigned int>(pages, frameCount);

    // Ensure hand is in range
    if (mHand >= frameCount) mHand = 0;

    // Loop until we collected required number of frames to evict
    while (evicted.size() < need && frameCount > 0) {
        if (mHand >= mRam.size()) mHand = 0;
        Page &pg = mRam[mHand];

        if (pg.hasSecondChance()) {
            // second chance clear the bit and advance
            pg.setSecondChance(false);
            mHand = (mHand + 1) % frameCount;
            continue;
        }

        // No second chance
        evicted.push_back(mHand);

        // Advance hand to next frame
        mHand = (mHand + 1) % frameCount;
    }

    std::cout << "\n [SC]-Evicting: ";
    for (unsigned int idx : evicted) std::cout << idx << " ";
    std::cout << "\n";

    return evicted;
}


void SecondChance::onInsert(unsigned int pageId, unsigned int frameIdx) {
    // nothing special required if insert appends at end
    (void)pageId; (void)frameIdx;
}

void SecondChance::onAccess(unsigned int pageId) {
    // mark second chance bit for the page in mRam (if present)
    for (Page &p : mRam) {
        if (p.id == pageId) {
            p.setSecondChance(true);
            return;
        }
    }
}

void SecondChance::onEvict(unsigned int pageId, unsigned int frameIdx) {
    // adjust hand if eviction removes frames before current hand index
    if (mRam.empty()) { mHand = 0; return; }
    unsigned int n = static_cast<unsigned int>(mRam.size());
    if (frameIdx < n) {
        if (frameIdx < mHand) {
            // indices shift left by one for frames after frameIdx
            if (mHand > 0) --mHand;
        } else if (frameIdx == mHand) {
            // if current hand frame is removed, hand should now point to same index
            // which becomes the next element, so no change beyond bounds check
        }
    }
    // keep hand in valid range
    if (!mRam.empty()) mHand %= static_cast<unsigned int>(mRam.size());
    else mHand = 0;
}