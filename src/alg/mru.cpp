#include "alg/mru.h"

#include <vector>

// Constructor
Mru::Mru(std::vector<Page> &ram) : IAlgorithm(ram) {}

FrameResult Mru::execute(const std::vector<unsigned int> &accesses) {
	FrameResult res;
	res.state = InFrameE::Miss;
	res.pagePos = 0;
	return res;
}
