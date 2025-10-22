// includes

#include "app.h"
#include "stdlib.h"
#include "stdio.h"

int main(int argc, char *argv[]) {
    printf("Proyecto 2 \n");
    // create an MMU and run the Optimal test
    MemoryManagementUnit mmu;
    mmu.runOptimalTest();

    return 0;
}
