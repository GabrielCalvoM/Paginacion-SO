#include "alg/lru.h"

#include <vector>

// Constructor
Lru::Lru(std::vector<Page> &ram) : IAlgorithm(ram) {}