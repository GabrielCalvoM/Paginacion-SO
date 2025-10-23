#include "sim/pointer.h"

// Static Attributes
unsigned int Pointer::mIdCount = 0;

// Constructor
Pointer::Pointer() : id(++mIdCount) {}

// --- Getters ---
std::vector<Page> Pointer::getPages() const { 
    return mPages; 
}

// --- Setters ---
void Pointer::assignPages(int num) { 
    for (int i = 0; i < num; ++i) {
        mPages.push_back(Page());
    }
}