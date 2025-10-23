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
    computer.mmu.initAlgorithm(AlgTypeE::OPT, futureSequence);

    // Sequence

    unsigned int pid = 1;
    size_t bytes = 20 * Page::pageSize; // request 20 pages worth
    unsigned int ptrId = computer.mmu.newPtr(pid, bytes);

    printf("Allocated pointer id = %u for pid = %u\n", ptrId, pid);

    return 0;
}