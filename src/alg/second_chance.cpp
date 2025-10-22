#include "alg/second_chance.h"

#include <vector>

// Constructor
SecondChance::SecondChance(std::vector<Page> &ram) : IAlgorithm(ram) {}

FrameResult SecondChance::execute(const std::vector<unsigned int> &accesses) {
	FrameResult res;
	res.state = InFrameE::Miss;
	res.pagePos = 0;
	return res;
}
