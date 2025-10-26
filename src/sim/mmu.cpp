#include "sim/mmu.h"

#include <algorithm>
#include <iostream>
#include <new>
#include <set>

#include "alg/optimal.h"
#include "alg/fifo.h"

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
    // fifo
    else if (type == AlgType::FIFO) mAlgorithm = std::make_unique<Fifo>(mRam);
    // Fallback OPT
    else mAlgorithm = std::make_unique<Optimal>(mRam, accessSequence); 
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
                          << "\t | InRAM: " << (page.isInRealMem() ? "Y" : "N") 
                          << "\t | PhysDir: " << page.getPhysicalDir() << " ] \n ";
            }
        }
    }
    std::cout << "\n =================== \n" << std::endl;
    std::cout << std::flush;
}

//////////////////////////////////////////////////////////////////////////////////////////
// --- EXEC INSTRUCTION  ---
void MemoryManagementUnit::executeIntSet(const IntSet &iset) 
{
    Instruction* i;
    while ((i = iset.next())) {
        if (!i) continue;

        unsigned int p = i->param1;
        size_t bytes = i->param2;

        switch (i->type) {
            // Call New
            case newI: {
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
void MemoryManagementUnit::usePtr(unsigned int ptrId)
{
    auto it = mSimbolTable.find(ptrId);
    if (it == mSimbolTable.end()) return;

    std::vector<Page> &pages = it->second.getPages();
    for (unsigned int i = 0; i < pages.size(); ++i) {
        Page &pg = pages[i];

        //si ya esta en RAM no hacer nada (lo del pass de Carlos)
        if (pg.isInRealMem()) continue;

        //si hay espacio libre en RAM, colocar al final
        if (mRam.size() < mRam.capacity()) {
            pg.setInRealMem(true);
            pg.setPhysicalDir(static_cast<unsigned int>(mRam.size()));
            mRam.push_back(pg); // copia actualizada a RAM
            continue;
        }

        //no hay espacio -> pedir al algoritmo indices a desalojar 
        std::vector<unsigned int> evictIdx = mAlgorithm->execute(mRam, 1);
        if (evictIdx.empty()) continue; // guard

        unsigned int idx = evictIdx[0];
        if (idx >= mRam.size()) continue; // guard

        //mover la página evictada a DISCO
        Page ev = mRam[idx];
        ev.setInRealMem(false);
        ev.setPhysicalDir(static_cast<unsigned int>(mDisk.size()));
        mDisk.push_back(ev);
        //colocar la página solicitada en el frame liberado
        Page &frame = mRam[idx];
        //reconstruye la pagina in-place
        frame.~Page();
        new (&frame) Page(pg);
        frame.setInRealMem(true);
        frame.setPhysicalDir(idx);
        frame.setPhysicalDir(idx);

        //actualizar la página en la tabla de simbolos para reflejar que ahora esta en RAM
        pg.setInRealMem(true);
        pg.setPhysicalDir(idx);
    }
}


//////////////////////////////////////////////////////////////////////////////////
// --- PROC METHOD: DEL PTR ---
void MemoryManagementUnit::delPtr(unsigned int ptrId)
{
    auto it = mSimbolTable.find(ptrId);
    if (it == mSimbolTable.end()) return;

    Pointer &ptr = it->second;
    std::vector<Page> pages = ptr.getPages(); //copia de las paginas

    //recolectar indices en RAM y DISK
    std::vector<unsigned int> ramIdxs;
    std::vector<unsigned int> diskIdxs;

    for (const Page &pg : pages) {
        if (pg.isInRealMem()) {
            unsigned int idx = pg.getPhysicalDir();
            //if index does not match -> try to find by id
            if (!(idx < mRam.size() && mRam[idx].id == pg.id)) {
                bool found = false;
                for (unsigned int k = 0; k < mRam.size(); ++k) {
                    if (mRam[k].id == pg.id) { idx = k; found = true; break; }
                }
                if (!found) continue;
            }
            ramIdxs.push_back(idx);
        } else {
            unsigned int idx = pg.getPhysicalDir();
            if (!(idx < mDisk.size() && mDisk[idx].id == pg.id)) {
                bool found = false;
                for (unsigned int k = 0; k < mDisk.size(); ++k) {
                    if (mDisk[k].id == pg.id) { idx = k; found = true; break; }
                }
                if (!found) continue;
            }
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

    //NOTA: No se elimina el puntero de la lista de punteros del proceso propietario!!!!
    // OJOOOO
}

void MemoryManagementUnit::kill(unsigned int ptrId)
{
    // TODO: replace placeholder with real logic
    (void)ptrId;
}
