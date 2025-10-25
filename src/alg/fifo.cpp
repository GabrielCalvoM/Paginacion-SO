#include "alg/fifo.h"
#include <vector>
#include <iostream>
#include <algorithm>

// implementacion inicial de fifo simple sin cola de page ids
Fifo::Fifo(std::vector<Page> &ram) : IAlgorithm(ram), mNext(0) {}

std::vector<unsigned int> Fifo::execute(const std::vector<Page> &bufRAM, unsigned int pages)
{
    std::vector<unsigned int> evicted;
    if (bufRAM.empty() || pages == 0) return evicted;

    unsigned int frameCount = static_cast<unsigned int>(bufRAM.size());
    unsigned int count = std::min<unsigned int>(pages, frameCount);

    for (unsigned int i = 0; i < count; ++i) {
        unsigned int idx = (mNext + i) % frameCount;
        evicted.push_back(idx);
    }

    mNext = (mNext + count) % (frameCount == 0 ? 1 : frameCount);

    std::cout << "\n [FIFO]-Evicting: ";
    for (unsigned int idx : evicted) std::cout << idx << " ";
    std::cout << "\n";

    return evicted;
}