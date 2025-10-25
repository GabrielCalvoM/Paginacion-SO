#ifndef ALGORITHM_H
#define ALGORITHM_H

#include <vector>

#include "sim/page.h"

typedef enum class InFrame {
    Hit,
    Miss
} InFrameE;

struct FrameResult {
    InFrameE state;
    unsigned int pagePos;
};

typedef enum class AlgType {
    OPT,
    FIFO,
    SC,
    MRU,
    LRU,
    RND
} AlgTypeE;

class IAlgorithm {
protected:
    std::vector<Page> &mRam;

public:
    IAlgorithm(std::vector<Page>&);
    virtual ~IAlgorithm() = default;

    // Execute the algorithm to free 'pages' frames.
    // Returns a list of indices in mRam that should be evicted (ordered by eviction preference).
    virtual std::vector<unsigned int> execute(const std::vector<Page>& bufRAM, unsigned int pages) = 0;

};

#endif // ALGORITHM_H
