#include "alg/random.h"

#include <vector>

// Constructor
Random::Random(std::vector<Page> &ram) : IAlgorithm(ram) {}

// Execute: stub implementation to satisfy linker
std::vector<unsigned int> execute(const std::vector<Page> &bufRAM, unsigned int pages)
{
	FrameResult res;
	res.state = InFrameE::Miss;
	res.pagePos = 0;
	return 0;
}
