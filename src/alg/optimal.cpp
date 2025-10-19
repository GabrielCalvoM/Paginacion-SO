#include "alg/optimal.h"

#include <vector>

// Constructor
Optimal::Optimal(std::vector<Page> &ram, std::vector<unsigned int> sequence) : IAlgorithm(ram), mAccessSequence(sequence) {}
