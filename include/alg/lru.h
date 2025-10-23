/////////////////// ESTO CREO QUE NO HAY QUE IMPLEMENTARLO ///////////////////
/////////////////// ESTO CREO QUE NO HAY QUE IMPLEMENTARLO ///////////////////
/////////////////// ESTO CREO QUE NO HAY QUE IMPLEMENTARLO ///////////////////
/////////////////// ESTO CREO QUE NO HAY QUE IMPLEMENTARLO ///////////////////

#ifndef LRU_H
#define LRU_H

#include <vector>
#include <queue>

#include "sim/page.h"
#include "alg/algorithm.h"

class Lru : public IAlgorithm {
public:
    Lru(std::vector<Page>&);
    std::vector<unsigned int> execute(const std::vector<Page> &bufRAM, unsigned int pages) override;
        
};

#endif // LRU_H

/////////////////// ESTO CREO QUE NO HAY QUE IMPLEMENTARLO ///////////////////
/////////////////// ESTO CREO QUE NO HAY QUE IMPLEMENTARLO ///////////////////
/////////////////// ESTO CREO QUE NO HAY QUE IMPLEMENTARLO ///////////////////
/////////////////// ESTO CREO QUE NO HAY QUE IMPLEMENTARLO ///////////////////