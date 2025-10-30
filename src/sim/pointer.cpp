#include "sim/pointer.h"

// Constructor
Pointer::Pointer(unsigned int &idCount) : id(++idCount) {}

// Destructor
Pointer::~Pointer() { mPages.clear(); }

// --- Getters ---
std::vector<std::unique_ptr<Page>>& Pointer::getPages() const { 
    return mPages; 
}

// --- Setters ---
void Pointer::assignPages(int size, unsigned int &idCount) { 
    while (size >= 4096) {
        mPages.push_back(std::make_unique<Page>(4096, idCount));
        size -= 4096;
    }
    
    mPages.push_back(std::make_unique<Page>(4096, idCount));
}