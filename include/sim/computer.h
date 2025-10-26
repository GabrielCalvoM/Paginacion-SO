#ifndef COMPUTER_H
#define COMPUTER_H

#include <vector>
#include <string>

#include "sim/intset.h"
#include "sim/mmu.h"

class Computer {
private:
    static const std::vector<Instruction> mInstr;
    unsigned int mInstrIndex = 0;

public:
    MemoryManagementUnit mmu;
    const unsigned int cores = 1;
    const unsigned int instrPerSec = 1;
    const unsigned int diskAccessTime = 5; // 5s
    
    
};

#endif // COMPUTER_H
