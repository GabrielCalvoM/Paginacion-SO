#include "alg/random.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>

// RANDOM BUILDER
Random::Random(std::vector<Page> &ram, unsigned int seed) : IAlgorithm(ram) {
    // random seed if not provided
    if (seed == 0) {
        mRng.seed( 
            static_cast<unsigned int>(std::chrono::high_resolution_clock::now().time_since_epoch().count())
        );
    }
    mRng.seed(seed);
}

// EXECUTION
std::vector<unsigned int> Random::execute(const std::vector<Page> &bufRAM, unsigned int pages) 
{
    printf("\n [RND]-Start \n");
    std::vector<unsigned int> evicted;
    if (pages == 0 || bufRAM.empty()) return evicted;

    // Define array
    std::vector<unsigned int> roulette(bufRAM.size());
    for (unsigned int i = 0; i < bufRAM.size(); ++i)
        roulette[i] = i;

    // Choose random and select the first in range [0, pages]
    std::shuffle(roulette.begin(), roulette.end(), mRng)
    evicted.insert(evicted.end(), roulette.end(), roulette.begin() + pages);

    std::cout << "\n [FIFO]-Evicting: ";
    for (unsigned int idx : evicted) std::cout << idx << " ";
    std::cout << "\n";

    printf("\n [RND]-Finish \n");
    return evicted;
}