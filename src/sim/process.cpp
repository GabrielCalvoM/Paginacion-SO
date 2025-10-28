#include "sim/process.h"

// Constructor
Process::Process(unsigned int &idCount) : pId(++idCount) {}

// --- Getters ---
std::vector<unsigned int> Process::getPointers() const { return mPointers; }

// --- Setters ---
void Process::assignPtr(unsigned int ptr) { mPointers.push_back(ptr); }
void Process::deletePtr(unsigned int ptr) {
    for (int i = 0; i < mPointers.size(); ++i ) {
        if (mPointers.at(i) != ptr) continue;
        
        std::vector<unsigned int>::iterator it = mPointers.begin() + i;
        mPointers.erase(it);
    }
}
