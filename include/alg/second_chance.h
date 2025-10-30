#ifndef SECOND_CHANCE_H
#define SECOND_CHANCE_H

#include <vector>
#include <queue>

#include "sim/page.h"
#include "alg/algorithm.h"

class SecondChance : public IAlgorithm {
public:
    SecondChance(std::vector<Page>&);
    std::vector<unsigned int> execute(unsigned int pages) override;

    // Declarar las callbacks aquí, implementar en el .cpp
    void onInsert(unsigned int pageId, unsigned int frameIdx) override;
    void onAccess(unsigned int pageId) override;
    void onEvict(unsigned int pageId, unsigned int frameIdx) override;

private:
    // Clock hand index into RAM
    unsigned int mHand = 0;
};

#endif // SECOND_CHANCE_H