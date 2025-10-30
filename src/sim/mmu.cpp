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
            auto pages = ptr.getPages();
            if (pages.empty()) {
                std::cout << " NULL \n" << std::endl;
                continue;
            }

            for (const Page &page : pages) {
                std::cout << "\t [ PageID: " << page.id 
                          << "\t | InRAM: " << (page.isInRealMem() ? "Y" : "N") 
                          << "\t | PhysDir: " << page.getPhysicalDir() << " ] \n ";
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

//////////////////////////////////////////////////////////////////////////////////////////
// --- PROC METHOD: NEW PTR ---
unsigned int MemoryManagementUnit::newPtr(unsigned int pid, size_t size) 
{
    // BOOL FAULT
    bool fault = 0;
    Page* pg = nullptr;

    // Compute Pages
    unsigned int pages = size / Page::pageSize;
    if (size % Page::pageSize != 0) ++pages;

    printf("\n [EXE: new] - Size %zu -> Pages: %u \n", size, pages);

    // Create Pointer and assign Pages
    Pointer ptr(ptrIdCount);
    ptr.assignPages(static_cast<int>(pages), size, pageIdCount);
    std::vector<Page> &newPages = ptr.getPages();


    // Fill Pages in RAM
    unsigned int placedPages = 0;
    while (placedPages < pages && mRam.size() < 100) {
        pg = &newPages[placedPages];
        pg->setInRealMem(true);
        pg->setPhysicalDir(static_cast<unsigned int>(mRam.size()));
        // push a copy of the page into RAM
        mRam.push_back(*pg);

        // Notify algorithm of insertion
        if (mAlgorithm) mAlgorithm->onInsert(pg->id, static_cast<unsigned int>(mRam.size()-1));

        ++placedPages;

        // consume Time
        printf("\n [EXE: new] - Placing direct RAM page %u (%u/%u) at %u\n",
                pg->id, placedPages, pages, pg->getPhysicalDir());
        addTime(false);
    }

    // Fill Extra in DISK FAULT
    if (placedPages < pages) {
        // FAULT
        fault = 1;
        unsigned int remaining = pages - placedPages;
        printf("\n [EXE: new] - Not enough RAM space for %u pages \n", remaining);
        printf("\n [EXE: new] - Calling Algorithm \n");

        std::vector<unsigned int> evicted = mAlgorithm->execute(mRam, remaining);

        for (unsigned int idx : evicted) {
            if (placedPages >= pages) break; // FINISH
            if (idx >= mRam.size()) continue; // GUARD

            // move evicted to disk
            Page ev = mRam[idx];
            ev.setInRealMem(false);
            ev.setPhysicalDir(static_cast<unsigned int>(mDisk.size()));
            mDisk.push_back(ev);

            printf("\n [EXE: new] - Moving page to DISK %u (%u/%u) at %u\n",
                ev.id, remaining--, evicted.size(), ev.getPhysicalDir());

            // move extra to frame (replace the evicted slot)
            Page &frame = mRam[idx];
            frame = newPages[placedPages]; // copy incoming page into frame
            frame.setInRealMem(true);
            frame.setPhysicalDir(idx);

            ++placedPages;

            printf("\n [EXE: new] - Placing direct RAM page %u (%u/%u) at %u\n",
                frame.id, placedPages, pages, frame.getPhysicalDir());
        }
    }

    // Store Pointer Data (Table + Owner)
    mSimbolTable.emplace(ptr.id, ptr);

    auto proc = mProcessList.find(pid);
    
    // NO CREAR NUEVOS PROCESOS, SOLO LOS CREADOS (?)
    // Actualmente: crea un nuevo proceso si proc no existe
    if (proc == mProcessList.end()) {
         Process* p = new Process(++procIdCount);
         mProcessList[pid] = p;
         p->assignPtr(ptr.id);
     } else {
        proc->second->assignPtr(ptr.id);
    }

    printf("\n [EXE: new] - Finish new instruction \n");

    addTime(fault);
    return ptr.id;
}

//////////////////////////////////////////////////////////////////////////////////////////
// --- PROC METHOD: USE PTR ---
void MemoryManagementUnit::usePtr(unsigned int ptrId)
{
    // VARS
    bool fault = 0;
    Page* pg = nullptr;

    auto it = mSimbolTable.find(ptrId);
    if (it == mSimbolTable.end()) { printf("\n [EXE: use] - NULL PTR \n"); return; }
    printf("\n [EXE: use] - Ptr %u \n", it->id);

    std::vector<Page> &pages = it->second.getPages();
    for (unsigned int i = 0; i < pages.size(); ++i) {
        pg = &pages[i];

        // si ya esta en RAM -> mark access for algorithms that need it
        if (pg->isInRealMem()) {
            printf("\n [EXE: use] - Page Hit ID = %u  \n", pg->id);

            // mark second chance bit
            pg->setSecondChance(true);

            // if (mAlgorithm) mAlgorithm->onAccess(pg->id);
            continue;
        }

        //si hay espacio libre en RAM, colocar al final
        if (mRam.size() < 100) {
            printf("\n [EXE: use] - Page Fault ID = %u  (but Available RAM) \n", pg->id);

            fault = true;
            pg->setInRealMem(true);
            //pg->setPhysicalDir(static_cast<unsigned int>(mRam.size()));
            mRam.push_back(*pg); // copia actualizada a RAM

            printf("\n [EXE: use] - Moving page to RAM %u at %u\n", pg->id, pg->getPhysicalDir());
            // notify algorithm of insert
            // if (mAlgorithm) mAlgorithm->onInsert(pg->id, static_cast<unsigned int>(mRam.size()-1));
            continue;
        }

        // signals
        // if (mAlgorithm) {
        //     mAlgorithm->onAccess(pg.id);   // notificar
        //     mAlgorithm->optForesee(pg.id); // consumir ocurrencia
        // }

    // NOT ENOUGH RAM SPACE
    fault = true;
    printf("\n [EXE: use] - Page Fault ID = %u \n", pg->id);
    std::vector<unsigned int> evicted = mAlgorithm->execute(mRam, 1);
    if (evicted.empty()) {  printf("\n [EXE: use] - WARNING: Empty Evicted");  continue; } // guard

    unsigned int idx = evicted[0];
    if (idx >= mRam.size()) continue;

    // Notify algorithm of eviction (resident page)
    unsigned int evictedId = mRam[idx].id;
    //if (mAlgorithm) mAlgorithm->onEvict(evictedId, ridx);

    // move evicted page to DISK (copy then remove slot)
    Page ev = mRam[ridx];
    ev.setInRealMem(false);
    ev.setPhysicalDir(static_cast<unsigned int>(mDisk.size()));
    mDisk.push_back(ev);
    // remove from RAM vector
    mRam.erase(mRam.begin() + ridx);
    printf("\n [EXE: use] - Moving page to DISK %u at %u\n", ev.id, ev.getPhysicalDir());

    // colocar la página en RAM (as a new slot at the end)
    pg->setInRealMem(true);
    pg->setPhysicalDir(static_cast<unsigned int>(mRam.size()));
    mRam.push_back(*pg);
    printf("\n [EXE: use] - Moving page to RAM %u at %u\n", pg->id, pg->getPhysicalDir());

        //actualizar la página en la tabla de simbolos para reflejar que ahora esta en RAM
        
        // notify algorithm of insert
        //if (mAlgorithm) mAlgorithm->onInsert(pg.id, idx);

        
    }

    printf("\n [EXE: use] - Finish use instruction \n");

    addTime(fault);
}


//////////////////////////////////////////////////////////////////////////////////
// --- PROC METHOD: DEL PTR ---
void MemoryManagementUnit::delPtr(unsigned int ptrId)
{
    auto it = mSimbolTable.find(ptrId);
    if (it == mSimbolTable.end()) return;

    Pointer &ptr = it->second;
    printf("\n [EXE: del] - Ptr %u \n", ptr->id);
    std::vector<Page> pages = ptr.getPages(); //copia de las paginas

    //recolectar indices en RAM y DISK
    std::vector<unsigned int> ramIdxs;
    std::vector<unsigned int> diskIdxs;

    for (const Page &pg : pages) {
        if (pg.isInRealMem()) {
            printf("\n [EXE: del] - Deleting RAM page %u at %u\n", pg.id, pg.getPhysicalDir());
            unsigned int idx = pg.getPhysicalDir();
            //if index does not match -> try to find by id
            if (!(idx < mRam.size() && mRam[idx].id == pg.id)) {
                bool found = false;
                for (unsigned int k = 0; k < mRam.size(); ++k) {
                    if (mRam[k].id == pg.id) { idx = k; found = true; break; }
                }
                if (!found) continue;
            }
            printf("\n [EXE: del] - Deletion Mark for RAM page %u = %u\n", pg.id, idx);
            ramIdxs.push_back(idx);
        } else {
            printf("\n [EXE: del] - Deleting DISK page %u at %u\n", pg.id, pg.getPhysicalDir());
            unsigned int idx = pg.getPhysicalDir();
            if (!(idx < mDisk.size() && mDisk[idx].id == pg.id)) {
                bool found = false;
                for (unsigned int k = 0; k < mDisk.size(); ++k) {
                    if (mDisk[k].id == pg.id) { idx = k; found = true; break; }
                }
                if (!found) continue;
            }
            printf("\n [EXE: del] - Deletion Mark for DISK page %u = %u\n", pg.id, idx);
            diskIdxs.push_back(idx);
        }
    }

    //borrar duplicados y ordernar descendente
    auto uniq_desc = [](std::vector<unsigned int> &v){
        std::sort(v.begin(), v.end());
        v.erase(std::unique(v.begin(), v.end()), v.end());
        std::sort(v.rbegin(), v.rend());
    };

    uniq_desc(ramIdxs);
    uniq_desc(diskIdxs);

    // --- Reconstruir mRam y mDisk evitando operator= de Page ---
    // Usar conjuntos de ids a borrar para filtrar
    std::set<unsigned int> ramDelIds;
    std::set<unsigned int> diskDelIds;
    for (unsigned int idx : ramIdxs) {
        if (idx < mRam.size()) ramDelIds.insert(mRam[idx].id);
    }
    for (unsigned int idx : diskIdxs) {
        if (idx < mDisk.size()) diskDelIds.insert(mDisk[idx].id);
    }

    // Notify algorithm of evictions for RAM indices to be removed
    for (unsigned int idx : ramIdxs) {
        if (idx < mRam.size()) {
            unsigned int pid = mRam[idx].id;
            if (mAlgorithm) mAlgorithm->onEvict(pid, idx);
        }
    }

    // Reconstruir RAM
    std::vector<Page> newRam;
    newRam.reserve(mRam.size() - ramDelIds.size());
    for (const Page &p : mRam) {
        if (ramDelIds.find(p.id) == ramDelIds.end()) newRam.push_back(p);
    }
    mRam.swap(newRam); 

    // Reconstruir DISK
    std::vector<Page> newDisk;
    newDisk.reserve(mDisk.size() - diskDelIds.size());
    for (const Page &p : mDisk) {
        if (diskDelIds.find(p.id) == diskDelIds.end()) newDisk.push_back(p);
    }
    mDisk.swap(newDisk);


    //reconstruir physicalDir values para las remaining RAM pages
    for (unsigned int i = 0; i < mRam.size(); ++i) {
        mRam[i].setPhysicalDir(i);
        mRam[i].setInRealMem(true);
    }

    //reconstruir physicalDir values para las remaining DISK pages
    for (unsigned int i = 0; i < mDisk.size(); ++i) {
        mDisk[i].setPhysicalDir(i);
        mDisk[i].setInRealMem(false);
    }

    //actualizar todos los punteros en la tabla de simbolos para que sus valores physicalDir de Page se queden correctos
    for (auto &kv : mSimbolTable) {
        Pointer &otherPtr = kv.second;
        std::vector<Page> &otherPages = otherPtr.getPages();
        for (Page &opg : otherPages) {
            if (opg.isInRealMem()) {
                // find in RAM by id
                bool found = false;
                for (unsigned int i = 0; i < mRam.size(); ++i) {
                    if (mRam[i].id == opg.id) { opg.setPhysicalDir(i); found = true; break; }
                }
                if (!found) {
                    // page no longer in RAM -> try disk
                    for (unsigned int i = 0; i < mDisk.size(); ++i) {
                        if (mDisk[i].id == opg.id) { opg.setInRealMem(false); opg.setPhysicalDir(i); break; }
                    }
                }
            } else {
                // find in DISK by id
                for (unsigned int i = 0; i < mDisk.size(); ++i) {
                    if (mDisk[i].id == opg.id) { opg.setPhysicalDir(i); break; }
                }
            }
        }
    }

    //remueve pointer from simbol table
    mSimbolTable.erase(it);

    // count time for the delete instruction (no fault)
    printf("\n [EXE: del] - Finish del instruction \n");
    addTime(false);

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
    std::vector<Page> emptyVec1;
    std::vector<Page> emptyVec2;
    mRam.swap(emptyVec1);
    mDisk.swap(emptyVec2);

    // restaurar la capacidad reservada de RAM como en el constructor
    mRam.reserve(ramSize / Page::pageSize);

    // reiniciar contador de id
    procCount = 0;
    ptrIdCount = 0;
    pageIdCount = 0;

    // se puede meter un alg que mantenga estado interno 
}
