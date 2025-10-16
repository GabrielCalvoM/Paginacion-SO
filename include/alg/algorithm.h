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
    std::vector<Page> &const mRam;

public:
    IAlgorithm(std::vector<Page>&);
    virtual FrameResult execute(std::vector<unsigned int>);
};

#endif // ALGORITHM_H