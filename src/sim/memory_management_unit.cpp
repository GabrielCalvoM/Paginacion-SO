#include "sim/memory_management_unit.h"

// Constructor
MemoryManagementUnit::MemoryManagementUnit() {
    mRam.reserve(ramSize / Page::pageSize);     // mRam.capacity() = tamaño máxmimo de RAM
}

// Destructor
MemoryManagementUnit::~MemoryManagementUnit() {
    while (mRam.size() > 0) { mRam.pop_back(); }
    while (mSimbolTable.size() > 0) { mSimbolTable.erase(mSimbolTable.begin()); }
    while (mProcessList.size() > 0) {
        delete mProcessList.begin()->second;
        mProcessList.erase(mProcessList.begin());
    }
    
}

// Test: create an Optimal algorithm and run a single execute to trigger debug output
void MemoryManagementUnit::runOptimalTest() {
    // small access sequence to test Optimal
    std::vector<unsigned int> seq = {1, 2, 3, 4, 1, 2, 5};
    // instantiate Optimal with a reference to mRam
    Optimal opt(mRam, seq);
    // call execute which prints debug output
    opt.execute(seq);
}
