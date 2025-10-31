#include "alg/algorithm.h"

#include <vector>

// Constructor
IAlgorithm::IAlgorithm(std::unordered_map<unsigned int, std::unique_ptr<Page>*> &ram) : mRam(ram) {}
