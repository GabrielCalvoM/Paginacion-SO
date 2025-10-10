#ifndef COMPUTER_H
#define COMPUTER_H

#include "memory_management_unit.h"

class Computer {
    public:
        const int cores = 1;
        const int instrPerSec = 1;
        const int diskAccessTime = 5; // 5s
        const int ramSize = 4e5;
        const int pageSize = 4e3;

    
};

#endif // COMPUTER_H
