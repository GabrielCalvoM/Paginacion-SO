#ifndef SECOND_CHANCE_H
#define SECOND_CHANCE_H

#include <vector>
#include <queue>

#include "sim/page.h"
#include "alg/algorithm.h"

class SecondChance : public IAlgorithm {
public:
    SecondChance(std::vector<Page>&);
    std::vector<unsigned int> execute(const std::vector<Page> &bufRAM, unsigned int pages) override;

private:
    // Clock hand index into RAM
    unsigned int mHand = 0;
};

#endif // SECOND_CHANCE_H
