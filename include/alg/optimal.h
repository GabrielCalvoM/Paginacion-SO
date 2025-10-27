#ifndef OPTIMAL_H
#define OPTIMAL_H

#include <vector>
#include <unordered_map>
#include <deque>
#include <cstddef>

#include "sim/page.h"
#include "alg/algorithm.h"

class Optimal : public IAlgorithm {
    private:
        std::vector<unsigned int> mAccessSequence;
        std::unordered_map<unsigned int, std::deque<size_t>> mOccurrences;
        size_t mCurIndex = 0;

    public:
        Optimal(std::vector<Page>& bufRAM, const std::vector<unsigned int>& pages);
        std::vector<unsigned int> execute(const std::vector<Page> &bufRAM, unsigned int pages);
        
        void optForesee(unsigned int pageId) override;
}

#endif // OPTIMAL_H
