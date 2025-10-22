#include "sim/memory_management_unit.h"
#include "alg/optimal.h"

//////////////////////////////////////////////////////////////////////////////////////////
// --- CPP IMPLEMENTATION ---
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

//////////////////////////////////////////////////////////////////////////////////////////
// --- AUX METHODS ---

void addPage(unsigned int pid) {
    // Check if mRam <= 100 pages
    if (mRam.size() < 100) {
    // Replace Page if needed using the algorithm
    } else {
    // Page fault handling
    }
}


// --- PROC METHODS ---

unsigned int newPtr(unsigned int pid, size_t size) {

    return pid;
}


/*
    // Test: create an Optimal algorithm and run a single execute to trigger debug output
    void MemoryManagementUnit::runOptimalTest() {
    // small access sequence to test Optimal
    std::vector<unsigned int> seq = {0, 1, 2, 1, 3, 5, 8, 20, 13, 1, 2, 4, 5, 15, 0, 15};
    // instantiate Optimal with a reference to mRam
    Optimal opt(mRam, seq);
    // call execute which prints debug output
    opt.execute(seq);
*/