#include "alg/mru.h"
#include <vector>
#include <iostream>
#include <algorithm>

// Constructor
Mru::Mru(std::vector<Page> &ram) : IAlgorithm(ram) {}

unsigned int Mru::findMax(INT) {
    
}

Mru::execute(const std::vector<Page>& bufRAM, unsigned int pages) {
    std::vector<unsigned int> evicted;

    // Add the pages with the highest usage rate
    for (int i = 0; i < pages; i++) {
        evicted.push_back(findMax());
    }

    return evicted;
}
