#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <vector>

#include "sim/page.h"

enum class InFrameE {
    Hit,
    Miss
};

struct FrameResult {
    InFrameE state;
    unsigned int pagePos;
};

enum class AlgTypeE {
    OPT,
    FIFO,
    SC,
    MRU,
    LRU,
    RND
};

class IAlgorithm {
protected:
    std::vector<Page> &mRam;

public:
    IAlgorithm(std::vector<Page>&);
    virtual ~IAlgorithm() = default;

    // Execute the algorithm for the given access sequence.
    // Returns the frame result for the current access.
    virtual FrameResult execute(const std::vector<unsigned int>&) = 0;

};

#endif // ALGORITHM_H
