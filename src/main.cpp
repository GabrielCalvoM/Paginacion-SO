#include "app.h"
#include "stdlib.h"
#include "stdio.h"

#include "sim/computer.h"
#include "alg/algorithm.h" 
#include "sim/page.h"      

int main(int argc, char *argv[]) {
    printf("[OS]-[Proyecto 2]\n");

    // Build Simulation
    Computer computer;
    std::vector<unsigned int> futureSequence; 
    computer.mmu.initAlgorithm(AlgType::OPT, futureSequence);

    // Sequence

    unsigned int pid = 1;
    size_t bytes = 100 * Page::pageSize;
    computer.mmu.newPtr(pid, bytes);

    pid = 2;
    bytes = 4 * Page::pageSize;
    computer.mmu.newPtr(pid, bytes);

    pid = 3;
    bytes = 4 * Page::pageSize;
    computer.mmu.newPtr(pid, bytes);

    computer.mmu.printState();

    return 0;
}
