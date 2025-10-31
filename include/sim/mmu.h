#ifndef MMU_H
#define MMU_H

#include <cstddef>
#include <unordered_map>
#include <set>
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
    std::unordered_map<unsigned int, unsigned int> mPageMap;
    std::unordered_map<unsigned int, unsigned int> mPtrMap;
    
    std::vector<std::unique_ptr<Page>*> mPageLoadedTime;
    std::unordered_map<unsigned int, std::unique_ptr<Page>*> mRam;
    std::unordered_map<unsigned int, std::unique_ptr<Page>*> mDisk;
    std::unique_ptr<IAlgorithm> mAlgorithm;
    
    std::vector<std::unique_ptr<Page>*> mPagesCreated;
    std::vector<std::unique_ptr<Page>*> mPagesModified;
    std::vector<unsigned int> mPagesDeleted;
    unsigned int mLoadedPages = 0;
    unsigned int mUnloadedPages = 0;
    unsigned int thrashTime = 0;
    unsigned int algTime = 0;
    
    std::set<unsigned int> mRamAddresses;
    std::set<unsigned int> mDiskAddresses;
    unsigned int procCount = 0;
    unsigned int procIdCount = 0;
    unsigned int ptrIdCount = 0;
    unsigned int pageIdCount = 0;

    bool insertPageOnDisk(std::unique_ptr<Page>&, unsigned int index);
    bool insertPageOnRam(std::unique_ptr<Page>&, unsigned int);
    unsigned int ramAddress();
    unsigned int diskAddress();
    
public:
    static const unsigned int ramSize = Consts::MAX_RAM; // 400 KB

    MemoryManagementUnit();
    ~MemoryManagementUnit();

    // --- Getters ---
    const std::unordered_map<unsigned int, std::unique_ptr<Page>*> &ram() const { return mRam; }
    const std::unordered_map<unsigned int, std::unique_ptr<Page>*> &disk() const { return mDisk; }
    const std::vector<std::unique_ptr<Page>*> pagesCreated() const { return mPagesCreated; }
    const std::vector<std::unique_ptr<Page>*> pagesModified() const { return mPagesModified; }
    const std::vector<unsigned int> &pagesDeleted() const { return mPagesDeleted; }
    const unsigned int getPageId(unsigned int id) const { return mPtrMap.at(mPageMap.at(id)); }
    const unsigned int getProcesses() const { return mProcessList.size(); }
    const unsigned int getRamSize() const {
        int size = 0;
        for (const auto &p : mRam) size += (*p.second)->getSpace();
        return size;
    }
    const unsigned int getDiskSize() const {
        int size = 0;
        for (const auto &p : mDisk) size += (*p.second)->getSpace();
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

        return freeBytes;
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
