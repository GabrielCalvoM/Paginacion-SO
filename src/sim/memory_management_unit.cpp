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
// --- PROC METHOD: NEW PTR ---
unsigned int MemoryManagementUnit::newPtr(unsigned int pid, size_t size) 
{
    // Compute Pages
    unsigned int pages = size / Page::pageSize;
    if (size % Page::pageSize != 0) ++pages;

    // Create Pointer and assign Pages
    Pointer ptr;
    ptr.assignPages(static_cast<int>(pages));
    std::vector<Page> newPages = ptr.getPages();

    // Fill Pages in RAM
    unsigned int placedPages = 0;
    while (placedPages < pages && mRam.size() < mRam.capacity()) {
        Page p = newPages[placedPages];
        p.setInRealMem(true);
        p.setPhysicalDir(static_cast<unsigned int>(mRam.size()));
        mRam.push_back(p);
        ++placedPages;
    }

    // Fill Extra in DISK
    if (placedPages < pages) {
        unsigned int remaining = pages - placedPages;
        std::vector<unsigned int> evictIndex = mAlgorithm->execute(mRam, remaining);

        for (unsigned int idx : evictIndex) {
            if (placedPages >= pages) break; // FINISH

            // move evicted to disk
            Page ev = mRam[idx];
            ev.setInRealMem(false);
            ev.setPhysicalDir(static_cast<unsigned int>(mDisk.size()));
            mDisk.push_back(ev);

            // move extra to frame
            Page ex = newPages[placedPages];
            ex.setInRealMem(true);
            ex.setPhysicalDir(idx);
            mRam[idx] = ex;

            ++placedPages;
        }

    }

    // Store Pointer Data (Table + Owner)
    // This part may need fixes
    mSimbolTable[ptr.id] = ptr;
    auto proc = mProcessList.find(pid);
    
    if (procIt == mProcessList.end()) {
        // create and insert a new process if missing
        Process* p = new Process();
        mProcessList[pid] = p;
        p->assignPtr(ptr.id);
    } else {
        procIt->second->assignPtr(ptr.id);
    }

    return ptr.id;
}

//////////////////////////////////////////////////////////////////////////////////////////
// --- PROC METHOD: USE PTR ---