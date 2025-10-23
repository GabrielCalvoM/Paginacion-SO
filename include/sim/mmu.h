#ifndef MMU_H
#define MMU_H

#include <cstddef>
#include <unordered_map>
#include <vector>
#include <memory>

#include "sim/page.h"
#include "sim/pointer.h"
#include "sim/process.h"
#include "alg/algorithm.h"

class MemoryManagementUnit {
private:
    std::unordered_map<unsigned int, Process*> mProcessList;
    std::unordered_map<unsigned int, Pointer> mSimbolTable;
    std::vector<Page> mRam;
    std::vector<Page> mDisk;
    std::unique_ptr<IAlgorithm> mAlgorithm;
    
public:
    static const unsigned int ramSize = 409600; // 400 KB

    MemoryManagementUnit();
    ~MemoryManagementUnit();

    // --- Getters ---

    // --- Setters ---
    void initAlgorithm(AlgType type, const std::vector<unsigned int> &accessSequence={});

    // --- Methods Process ---
    unsigned int newPtr(unsigned int pid, size_t size);  
    void usePtr(unsigned int ptrId);
    void delPtr(unsigned int ptrId);
    void kill(unsigned int pid);
    
};

#endif // MMU_H
