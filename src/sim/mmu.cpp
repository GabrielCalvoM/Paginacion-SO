#include "sim/mmu.h"

#include <algorithm>
#include <iostream>
#include <new>
#include <set>
#include <mutex>

#include "alg/optimal.h"
#include "alg/fifo.h"
#include "alg/second_chance.h"
#include "alg/lru.h"
#include "alg/mru.h"
#include "alg/random.h"

//////////////////////////////////////////////////////////////////////////////////////////
// --- CPP IMPLEMENTATION ---
// Constructor
MemoryManagementUnit::MemoryManagementUnit() {
    mRam.reserve(ramSize / Page::pageSize);     // mRam.capacity() = tamaño máxmimo de RAM
    for (int i = 1; i <= 100; ++i) mRamAddresses.insert(i);
}

// Destructor
MemoryManagementUnit::~MemoryManagementUnit() {
    mRam.clear();
    mSimbolTable.clear();
    while (mProcessList.size() > 0) {
        delete mProcessList.begin()->second;
        mProcessList.erase(mProcessList.begin());
    }
}

//////////////////////////////////////////////////////////////////////////////////////////////
// --- SETTERS ---
void MemoryManagementUnit::initAlgorithm(AlgType type, const std::vector<unsigned int> &accessSequence, unsigned int seed) 
{
    // Init mAlgorithm
    if (type == AlgType::OPT) mAlgorithm = std::make_unique<Optimal>(mRam, accessSequence);
    // fifo
    else if (type == AlgType::FIFO) mAlgorithm = std::make_unique<Fifo>(mRam);
    // second chance
    else if (type == AlgType::SC) mAlgorithm = std::make_unique<SecondChance>(mRam);
    // MRU
    else if (type == AlgType::MRU) mAlgorithm = std::make_unique<Mru>(mRam);
    // LRU
    else if (type == AlgType::LRU) mAlgorithm = std::make_unique<Lru>(mRam);
    // Random
    else if (type == AlgType::RND) mAlgorithm = std::make_unique<Random>(mRam, seed);
    // Fallback OPT
    else mAlgorithm = std::make_unique<Optimal>(mRam, accessSequence);
}

void MemoryManagementUnit::setProcCount(unsigned int n) {
    // clear previous procs
    reset();
    procCount = n;
}

//////////////////////////////////////////////////////////////////////////////////////////////
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
            auto &pages = ptr.getPages();
            if (pages.empty()) {
                std::cout << " NULL \n" << std::endl;
                continue;
            }

            for (const std::unique_ptr<Page> &page : pages) {
                std::cout << "\t [ PageID: " << page->id 
                          << "\t | InRAM: " << (page->isInRealMem() ? "Y" : "N") 
                          << "\t | PhysDir: " << page->getPhysicalDir() << " ] \n ";
            }
        }
    }
    std::cout << "\n =================== \n" << std::endl;
    std::cout << std::flush;
}

//////////////////////////////////////////////////////////////////////////////////////////
// --- EXEC INT SET  ---
void MemoryManagementUnit::executeIntSet(const IntSet &iset) 
{
    std::cout << "[DBG] MMU: executeIntSet called\n";
    const Instruction* i;
    unsigned int executed = 0;
    while ((i = iset.next())) {
        if (!i) continue;

        executeInstruction(i);
        ++executed;
    }

    std::cout << "[DBG] MMU: executeIntSet finished, executed = " << executed << std::endl;
}

// --- EXEC INSTRUCTION  ---
void MemoryManagementUnit::executeInstruction(const Instruction *i) 
{
    unsigned int p = i->param1;
    mPagesCreated.clear();
    mPagesModified.clear();
    mPagesDeleted.clear();
    mLoadedPages = mUnloadedPages = 0;

    switch (i->type) {
        // Call New
        case newI: {
            size_t bytes = i->param2;
            newPtr(p, bytes);
            break;
        }

        // Call Use
        case useI: {
            usePtr(p);
            break;
        }

        // Call Del
        case delI: {
            delPtr(p);
            break;
        }

        // Call Kill
        case killI: {
            kill(p);
            break;
        }

        // Fall Back
        default: {
            printf("[ERROR] = Instruction Not Valid ");
        }
    }
}

// AUX ADD TIME
void MemoryManagementUnit::addTime(bool fault) {    
    if (fault) {
        algTime += 5;
        thrashTime += 5;
    } else {
        algTime += 1;
    }
}

unsigned int MemoryManagementUnit::ramAddress() {
    unsigned int addr = *mRamAddresses.begin();
    mRamAddresses.erase(mRamAddresses.begin());
    return addr;
}

unsigned int MemoryManagementUnit::diskAddress() {
    if (mDiskAddresses.empty()) return mDisk.size() + 1;
    unsigned int addr = *mDiskAddresses.begin();
    mDiskAddresses.erase(mDiskAddresses.begin());
    return addr;
}

bool MemoryManagementUnit::insertPageOnDisk(std::unique_ptr<Page> &page, unsigned int index) {
    if (!page->isInRealMem()) return false;

    auto it = std::find_if(mRam.begin(), mRam.end(), [&](const std::pair<const unsigned int, std::unique_ptr<Page>*> &p){ return (*p.second)->id == page->id; });
    if (it != mRam.end()) { mRamAddresses.insert((*it->second)->getPhysicalDir()); mRam.erase(it); }

    page->setInRealMem(false);
    page->setPhysicalDir(static_cast<unsigned int>(mDisk.size()));
    mDisk[index] = &page;
    if (mAlgorithm) mAlgorithm->onEvict(page->id, index);
    return true;
}

bool MemoryManagementUnit::insertPageOnRam(std::unique_ptr<Page> &page, unsigned int index) {
    if (page->isInRealMem()) {
        if (mAlgorithm->type == AlgTypeE::SC) page->setSecondChance(true);
        if (mAlgorithm) mAlgorithm->onAccess(page->id);
        addTime(false);
        return false;
    }

    auto it = std::find_if(mDisk.begin(), mDisk.end(), [&](const std::pair<const unsigned int, std::unique_ptr<Page>*> &p){ return (*p.second)->id == page->id; });
    if (it != mDisk.end()) { mDiskAddresses.insert((*it->second)->getPhysicalDir()); mDisk.erase(it); }

    page->setInRealMem(true);
    page->setPhysicalDir(static_cast<unsigned int>(mRam.size()));
    mRam[index] = &page;
    if (mAlgorithm) mAlgorithm->onInsert(page->id, index);
    addTime(true);
    mLoadedPages++;
    return true;
}


//////////////////////////////////////////////////////////////////////////////////////////
// --- PROC METHOD: NEW PTR ---
unsigned int MemoryManagementUnit::newPtr(unsigned int pid, size_t size) 
{
    // Create Pointer
    unsigned int id = ptrIdCount + 1;
    mSimbolTable.try_emplace(id, ptrIdCount);  
    mPtrMap.emplace(id, pid);
    Pointer &ptr = mSimbolTable.at(id);

    // Añadir puntero al proceso
    auto proc = mProcessList.find(pid);
    
    if (proc == mProcessList.end()) {
        Process* p = new Process(pid);
        mProcessList[pid] = p;
        p->assignPtr(ptr.id);
    } else {
        proc->second->assignPtr(ptr.id);
    }

    // Assign Pages
    ptr.assignPages(size, pageIdCount);
    std::vector<std::unique_ptr<Page>> &newPages = ptr.getPages();

    for (auto &p : newPages) { mPageMap.emplace(p->id, ptr.id); mPagesCreated.insert(p->id); }    

    usePtr(ptr.id);

    return ptr.id;
}

//////////////////////////////////////////////////////////////////////////////////////////
// --- PROC METHOD: USE PTR ---
void MemoryManagementUnit::usePtr(unsigned int ptrId)
{
    auto ptr = mSimbolTable.find(ptrId);
    if (ptr == mSimbolTable.end()) return;

    std::vector<std::unique_ptr<Page>> &pages = ptr->second.getPages();
    std::vector<unsigned int> evictIndex = mAlgorithm->execute(pages.size());
    auto it = evictIndex.begin();

    for (auto &p : pages) {
        unsigned int index = mRam.size();

        if (mRam.size() >= 100) {
            while (std::find_if(pages.begin(), pages.end(), [&](const std::unique_ptr<Page> &page){ return (*mRam[*it])->id == page->id; }) != pages.end()) ++it;
            std::unique_ptr<Page> &ev = *mRam[*it];
            insertPageOnDisk(ev, *it);
            index = *it;
        }

        insertPageOnRam(p, index);
    }

}


//////////////////////////////////////////////////////////////////////////////////
// --- PROC METHOD: DEL PTR ---
void MemoryManagementUnit::delPtr(unsigned int ptrId)
{
    auto it = mSimbolTable.find(ptrId);
    if (it == mSimbolTable.end()) return;

    Pointer &ptr = it->second;
    std::vector<std::unique_ptr<Page>> &pages = ptr.getPages(); //copia de las paginas

    //recolectar indices en RAM y DISK
    std::set<unsigned int> ramIdxs;
    std::set<unsigned int> diskIdxs;

    for (const auto &pg : pages) {
        if (pg->isInRealMem()) {
            ramIdxs.insert(pg->getPhysicalDir());
        } else {
            ramIdxs.insert(pg->getPhysicalDir());
        }
    }

    // Usar conjuntos de ids a borrar para filtrar
    for (unsigned int idx : ramIdxs) {
        mRam.erase(mRam.find(idx));
    }
    for (unsigned int idx : diskIdxs) {
        mDisk.erase(mDisk.find(idx));
    }

    // Notify algorithm of evictions for RAM indices to be removed
    for (unsigned int idx : ramIdxs) {
        if (idx < mRam.size()) {
            unsigned int pid = (*mRam[idx])->id;
            if (mAlgorithm) mAlgorithm->onEvict(pid, idx);
        }
    }

    //remueve pointer from simbol table
    mSimbolTable.erase(it);

    //NOTA: No se elimina el puntero de la lista de punteros del proceso propietario!!!!
    // OJOOOO
}

//////////////////////////////////////////////////////////////////////////////////
// --- PROC METHOD: KILL PTR ---
void MemoryManagementUnit::kill(unsigned int pid)
{
    auto it = mProcessList.find(pid);
    if (it == mProcessList.end()) return;

    Process* proc = it->second;
    if (proc) {
        std::vector<unsigned int> ptrs = proc->getPointers();
        for (unsigned int ptr : ptrs) {
            delPtr(ptr);
        }
        delete proc;
    }

    mProcessList.erase(it);

    // DEC Counter of PROCS
    procCount--;
}

/////////////////////////////////////////////////////////////////////////////////
// --- RESET SAFE ---
void MemoryManagementUnit::reset()
{
    // bloquear para evitar carreras con otras operaciones que modifiquen el estado
    std::lock_guard<std::mutex> lock(mStateMutex);

    // vaciar tabla de símbolos y lista de procesos
    for (auto pid : mProcessList) { delete mProcessList[pid.first]; }
    mSimbolTable.clear();
    mProcessList.clear();

    // reconstruir mRam y mDisk mediante swap con vectores vacíos
    mRam.clear();
    mDisk.clear();

    // restaurar la capacidad reservada de RAM como en el constructor
    mRam.reserve(ramSize / Page::pageSize);

    // reiniciar tiempos
    for (int i = 1; i <= 100; ++i) mRamAddresses.insert(i);
    thrashTime = 0;
    algTime = 0;

    // reiniciar contador de id
    procCount = 0;
    procIdCount = 0;
    ptrIdCount = 0;
    pageIdCount = 0;

    // se puede meter un alg que mantenga estado interno 
}
