#ifndef OPTIMAL_H
#define OPTIMAL_H

#include <vector>

#include "sim/page.h"
#include "alg/algorithm.h"

class Optimal : public IAlgorithm {
private:
    const std::vector<unsigned int> mAccessSequence;
    unsigned int mActualAccess = 0;

public:
    Optimal(std::vector<Page>&, std::vector<unsigned int>);
    FrameResult execute(std::vector<unsigned int>);

};

#endif // OPTIMAL_H
