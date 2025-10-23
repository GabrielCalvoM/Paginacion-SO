#include "alg/fifo.h"

#include <vector>

// Constructor
Fifo::Fifo(std::vector<Page> &ram) : IAlgorithm(ram) {}

std::vector<unsigned int> execute(const std::vector<Page> &bufRAM, unsigned int pages)
{
	FrameResult res;
	res.state = InFrameE::Miss;
	res.pagePos = 0;
	return NULL;
}
