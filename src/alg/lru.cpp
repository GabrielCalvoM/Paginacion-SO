#include "alg/lru.h"

#include <vector>

// Constructor
Lru::Lru(std::vector<Page> &ram) : IAlgorithm(ram) {}

FrameResult Lru::execute(const std::vector<unsigned int> &accesses) {
	FrameResult res;
	res.state = InFrameE::Miss;
	res.pagePos = 0;
	return res;
}
