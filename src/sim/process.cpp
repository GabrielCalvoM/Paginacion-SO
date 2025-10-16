#include "sim/process.h"

// Static Attributes
unsigned int Process::mIdCount = 0;

// Constructor
Process::Process() : pId(++mIdCount) {}

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