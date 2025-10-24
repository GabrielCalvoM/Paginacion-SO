#include "sim/mmu.h"
#include "alg/optimal.h"
#include <iostream>

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

// --- DEBUG ---
void MemoryManagementUnit::printState() const
{
    std::cout << "===[ MMU STATE ]===" << std::endl;
    for (const auto &[pid, process] : mProcessList) {
        std::cout << "\n PID : " << pid << std::endl;

        if (!process) {
            std::cout << " NULL \n" << std::endl;
            continue;
        }

        auto ptrs = process->getPointers();
        if (ptrs.empty()) {
            std::cout << " NULL \n" << std::endl;
            continue;
        }

        for (unsigned int ptrId : ptrs) {
            std::cout << " Ptr(" << ptrId << ") --> \n";

            auto it = mSimbolTable.find(ptrId);
            if (it == mSimbolTable.end()) {
                std::cout << " NULL \n" << std::endl;
                continue;
            }

            const Pointer &ptr = it->second;
            auto pages = ptr.getPages();
            if (pages.empty()) {
                std::cout << " NULL \n" << std::endl;
                continue;
            }

            for (const Page &page : pages) {
                std::cout << "\t [ PageID: " << page.id 
                          << " | InRAM: " << (page.isInRealMem() ? "Y" : "N") 
                          << " | PhysDir: " << page.getPhysicalDir() << " ] \n ";
            }
        }
    }
    std::cout << "\n =================== \n" << std::endl;
    std::cout << std::flush;
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
    std::vector<Page> &newPages = ptr.getPages();

    // Fill Pages in RAM
    unsigned int placedPages = 0;
    while (placedPages < pages && mRam.size() < mRam.capacity()) {
        
        Page &pg = newPages[placedPages];
        pg.setInRealMem(true);
        pg.setPhysicalDir(static_cast<unsigned int>(mRam.size()));
        mRam.push_back(pg);

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
    mSimbolTable.emplace(ptr.id, ptr);

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
