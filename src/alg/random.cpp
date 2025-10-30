#include "alg/random.h"
#include <vector>
#include <iostream>
#include <algorithm>
#include <random>
#include <chrono>

// RANDOM BUILDER
Random::Random(std::unordered_map<unsigned int, std::unique_ptr<Page>*> &ram, unsigned int seed) : IAlgorithm(ram) {
    // random seed if not provided
    if (seed == 0) {
        mRng.seed(static_cast<unsigned int>(
        std::chrono::high_resolution_clock::now().time_since_epoch().count()
       ));
    } else {
        mRng.seed(seed);
    }
}

// EXECUTION
std::vector<unsigned int> Random::execute(unsigned int pages) 
{
    printf("\n [RND]-Start \n");
    std::vector<unsigned int> evicted;
    if (pages == 0 || mRam.empty()) return evicted;

    // Define array
    std::vector<unsigned int> roulette;
    for (auto i : mRam)
        roulette.push_back(i.first);

    // Choose random and select the first in range [0, pages]
    std::shuffle(roulette.begin(), roulette.end(), mRng);
    // clamp pages to roulette size to avoid out-of-range
    if (pages > roulette.size()) pages = static_cast<unsigned int>(roulette.size());
    // correct range: begin -> begin + pages
    evicted.insert(evicted.end(), roulette.begin(), roulette.begin() + pages);

    std::cout << "\n [RND]-Evicting: ";
    for (unsigned int idx : evicted) std::cout << idx << " ";
    std::cout << "\n";

    printf("\n [RND]-Finish \n");
    return evicted;
}
