#include "alg/optimal.h"

#include <vector>

// Constructor
Optimal::Optimal(std::vector<Page> &ram, const std::vector<unsigned int> &sequence) : IAlgorithm(ram), mAccessSequence(sequence) {}

FrameResult Optimal::execute(const std::vector<unsigned int> &accesses) {
	FrameResult res;
	res.state = InFrameE::Miss;
	res.pagePos = 0;
	return res;
}
