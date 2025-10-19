#ifndef MMU_H
#define MMU_H

#include <unordered_map>
#include <vector>

#include "sim/page.h"
#include "sim/pointer.h"
#include "sim/process.h"

class MemoryManagementUnit {
private:
    std::unordered_map<unsigned int, Process*> mProcessList;
    std::unordered_map<unsigned int, Pointer> mSimbolTable;
    std::vector<Page> mRam;
    
public:
    static const unsigned int ramSize = 4e5;

    MemoryManagementUnit();

    // --- Getters ---


    // --- Setters ---


    inline unsigned int newPtr(unsigned int pid, size_t size);
    inline void usePointer(unsigned int ptrId);
    inline void deletePointer(unsigned int ptrId);
    inline void killPointer(unsigned int pid);
    
};

#endif // MMU_H
