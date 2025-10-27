#include "alg/random.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <unordered_set>
#include <unordered_map>

// RANDOM BUILDER
Random::Random(std::vector<Page> &ram) : IAlgorithm(ram) { }

// EXECUTION
std::vector<unsigned int> Random::execute(const std::vector<Page> &bufRAM, unsigned int pages) 
{
    printf("\n [RND]-Start \n");
    std::vector<unsigned int> evicted;
    if (pages == 0 || bufRAM.empty()) return evicted;

    // clone contents
    std::vector<Page> roulette;
    for (const Page &p : bufRAM) {
        roulette.push_back(p);
    }

    // Choose random
    int ev_count = 0;
    while (ev_count != pages) {
        int idx = rand() % roulette.size();
        const Page& p = roulette.pop(idx);
        evicted.push_back(p.id);
        ev_count++; 
    }

    std::cout << "\n [FIFO]-Evicting: ";
    for (unsigned int idx : evicted) std::cout << idx << " ";
    std::cout << "\n";

    printf("\n [RND]-Finish \n");
    return evicted;
}