#ifndef MMU_H
#define MMU_H

#include <cstddef>
#include <unordered_map>
#include <vector>
#include <memory>
#include <mutex>

#include "sim/intset.h"
#include "sim/page.h"
#include "sim/pointer.h"
#include "sim/process.h"
#include "alg/algorithm.h"
#include "constants.h"


class MemoryManagementUnit {
private:
    std::unordered_map<unsigned int, Process*> mProcessList;
    std::unordered_map<unsigned int, Pointer> mSimbolTable;
    std::vector<Page> mRam;
    std::vector<Page> mDisk;
    std::unique_ptr<IAlgorithm> mAlgorithm;
    
    unsigned int thrashTime = 0;
    unsigned int algTime = 0;

    unsigned int procCount = 0;
    unsigned int procIdCount = 0;
    unsigned int ptrIdCount = 0;
    unsigned int pageIdCount = 0;
    
public:
    static const unsigned int ramSize = Consts::MAX_RAM; // 400 KB

    MemoryManagementUnit();
    ~MemoryManagementUnit();

    // --- Getters ---
    const std::vector<Page> &ram() const { return mRam; }
    const unsigned int getProcesses() const { return mProcessList.size(); }
    const unsigned int getRamSize() const {
        int size = 0; for (const auto p : mRam) size += p.getSpace();
        return size;
    }
    const unsigned int getDiskSize() const {
        int size = 0; for (const auto p : mDisk) size += p.getSpace();
        return size;
    }
    const unsigned int getLoadedPages() const { return mRam.size(); }
    const unsigned int getUnloadedPages() const { return mDisk.size(); }
    const unsigned int getAlgTime() const { return algTime; }
    const unsigned int getThrashTime() const { return thrashTime; }

    const unsigned int getFragmentation() const {
        // track storage
        size_t totalPages = mRam.size() + mDisk.size();
        size_t totalBytes = totalPages * Page::pageSize;

        // calculate usage
        size_t usedBytes = getRamSize() + getDiskSize();
        size_t freeBytes = totalBytes - usedBytes;

        // percentage
        return static_cast<unsigned int>((freeBytes * 100) / totalBytes);
    }


    // --- Setters ---
    void initAlgorithm(AlgType type, const std::vector<unsigned int> &accessSequence={}, unsigned int seed=0);
    void setProcCount(unsigned int n);
    
    // --- Debug ---
    void printState() const;

    // --- IntSet ---
    void executeIntSet(const IntSet &iset);
    void addTime(bool fault);

    // --- Instruction ---
    void executeInstruction(const Instruction *i);

    // --- Methods Process ---
    unsigned int newPtr(unsigned int pid, size_t size);  
    void usePtr(unsigned int ptrId);
    void delPtr(unsigned int ptrId);
    void kill(unsigned int pid);

    // --- reset ---
    void reset();

private:
    // --- mutex aux ---
    std::mutex mStateMutex;
    
};

#endif // MMU_H
