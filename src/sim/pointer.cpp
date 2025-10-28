#include "sim/pointer.h"

// Static Attributes
unsigned int Pointer::mIdCount = 0;

// Constructor
Pointer::Pointer() : id(++mIdCount) {}

// --- Getters ---
std::vector<Page>& Pointer::getPages() { 
    return mPages; 
}

const std::vector<Page>& Pointer::getPages() const { 
    return mPages; 
}

// --- Setters ---
void Pointer::assignPages(int num, int size) { 
    for (int i = 0; i < num; ++i) {
        if (size >= 4096) {
            mPages.push_back(Page(4096));
            size -= 4096;
        }
        else {
            mPages.push_back(Page(size));
        }
    }
}