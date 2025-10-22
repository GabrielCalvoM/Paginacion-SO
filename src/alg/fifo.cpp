#include "alg/fifo.h"

#include <vector>

// Constructor
Fifo::Fifo(std::vector<Page> &ram) : IAlgorithm(ram) {}

FrameResult Fifo::execute(const std::vector<unsigned int> &accesses) {
	FrameResult res;
	res.state = InFrameE::Miss;
	res.pagePos = 0;
	return res;
}
