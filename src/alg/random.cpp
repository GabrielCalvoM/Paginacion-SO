#include "alg/random.h"

#include <vector>

// Constructor
Random::Random(std::vector<Page> &ram) : IAlgorithm(ram) {}

// Execute: stub implementation to satisfy linker
FrameResult Random::execute(const std::vector<unsigned int> &accesses) {
	FrameResult res;
	res.state = InFrameE::Miss;
	res.pagePos = 0;
	return res;
}
