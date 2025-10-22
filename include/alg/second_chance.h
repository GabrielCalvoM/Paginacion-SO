#ifndef SECOND_CHANCE_H
#define SECOND_CHANCE_H

#include <vector>
#include <queue>

#include "sim/page.h"
#include "alg/algorithm.h"

class SecondChance : public IAlgorithm {
public:
    SecondChance(std::vector<Page>&);
    FrameResult execute(const std::vector<unsigned int>&) override;

};

#endif // SECOND_CHANCE_H
