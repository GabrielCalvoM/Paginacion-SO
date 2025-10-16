#ifndef COMPUTER_H
#define COMPUTER_H

#include <vector>
#include <string>

#include "sim/instruction.h"
#include "sim/memory_management_unit.h"

class Computer {
private:
    static const std::vector<Instruction> instr;
    unsigned int mInstrIndex = 0;

public:
    const MemoryManagementUnit mmu;
    const unsigned int cores = 1;
    const unsigned int instrPerSec = 1;
    const unsigned int diskAccessTime = 5; // 5s
    
};

#endif // COMPUTER_H
