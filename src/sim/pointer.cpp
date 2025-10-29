#include "sim/pointer.h"

// Constructor
Pointer::Pointer(unsigned int &idCount) : id(++idCount) {}

// --- Getters ---
std::vector<Page>& Pointer::getPages() { 
    return mPages; 
}

const std::vector<Page>& Pointer::getPages() const { 
    return mPages; 
}

// --- Setters ---
void Pointer::assignPages(int num, int size, unsigned int &idCount) { 
    while (size >= 4096) {
        mPages.push_back(Page(4096, idCount));
        size -= 4096;
    }
    
    mPages.push_back(Page(size, idCount));
}