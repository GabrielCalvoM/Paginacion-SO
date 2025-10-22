#ifndef RANDOM_H
#define RANDOM_H

#include <vector>
#include <queue>

#include "sim/page.h"
#include "alg/algorithm.h"

class Random : public IAlgorithm {
public:
    Random(std::vector<Page>&);
    FrameResult execute(const std::vector<unsigned int>&) override;

};

#endif // RANDOM_H
