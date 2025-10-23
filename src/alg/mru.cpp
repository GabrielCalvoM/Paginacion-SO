#include "alg/mru.h"

#include <vector>

// Constructor
Mru::Mru(std::vector<Page> &ram) : IAlgorithm(ram) {}

std::vector<unsigned int> execute(const std::vector<Page> &bufRAM, unsigned int pages)
{
	FrameResult res;
	res.state = InFrameE::Miss;
	res.pagePos = 0;
	return NULL;
}
