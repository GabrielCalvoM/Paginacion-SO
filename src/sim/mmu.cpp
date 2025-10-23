#include "sim/mmu.h"
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

// --- SETTERS ---
void MemoryManagementUnit::initAlgorithm(AlgType type, const std::vector<unsigned int> &accessSequence) 
{
    // Init mAlgorithm
    if (type == AlgType::OPT) mAlgorithm = std::make_unique<Optimal>(mRam, accessSequence);

    // Fallback OPT
    else mAlgorithm = std::make_unique<Optimal>(mRam, accessSequence); 
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
            if (idx >= mRam.size()) continue; // GUARD

            // move evicted to disk
            Page ev = mRam[idx];
            ev.setInRealMem(false);
            ev.setPhysicalDir(static_cast<unsigned int>(mDisk.size()));
            mDisk.push_back(ev);

            // move extra to frame
            Page &frame = mRam[idx];
            frame.setInRealMem(true);
            frame.setPhysicalDir(idx);
            
            // Copy Mut Attributes
            // const Page &src  = newPages[placedPages];

            ++placedPages;
        }

    }

    // Store Pointer Data (Table + Owner)
    mSimbolTable[ptr.id].emplace(ptr.id, ptr);

    auto proc = mProcessList.find(pid);
    
    if (proc == mProcessList.end()) {
        Process* p = new Process();
        mProcessList[pid] = p;
        p->assignPtr(ptr.id);
    } else {
        proc->second->assignPtr(ptr.id);
    }

    return ptr.id;
}

//////////////////////////////////////////////////////////////////////////////////////////
// --- PROC METHOD: USE PTR ---