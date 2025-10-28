#include "app.h"
#include "stdlib.h"
#include "stdio.h"

#include "sim/computer.h"
#include "alg/algorithm.h" 
#include "sim/page.h"      

int main(int argc, char *argv[]) {
    printf("\n[OS]-[Proyecto 2]\n");
    
/*
    // Build Simulation
    std::mutex mtx;
    std::condition_variable cv;
    Computer computer(cv, mtx);
    
    // Sequence
    std::vector<unsigned int> futureSequence;


    // // Pruebas de algoritmos
    
    // // Opt
    // //computer.mmu.initAlgorithm(AlgType::OPT, futureSequence);
    
    // // FIFO
    // //computer.mmu.initAlgorithm(AlgType::FIFO);
    
    // // Second Chance
    // //computer.mmu.initAlgorithm(AlgType::SC);

    // // LRU
    // computer.mmu.initAlgorithm(AlgType::LRU);


    // unsigned int pid = 1;
    // size_t bytes = 100 * Page::pageSize;
    // unsigned int ptr1 = computer.mmu.newPtr(pid, bytes);

    // pid = 2;
    // bytes = 4 * Page::pageSize;
    // unsigned int ptr2 = computer.mmu.newPtr(pid, bytes);

    // pid = 3;
    // bytes = 4 * Page::pageSize;
    // unsigned int ptr3 = computer.mmu.newPtr(pid, bytes);

    // computer.mmu.printState();

    // //PRUEBA USEPTR==============
    // //prueba de uso de puntero
    // //probar usePtr en los punteros creados (forzar carga/posible reemplazo)
    // printf("\n-- Usando ptr2 --\n");
    // computer.mmu.usePtr(ptr2);
    // computer.mmu.printState();

    // printf("\n-- Usando ptr3 --\n");
    // computer.mmu.usePtr(ptr3);
    // computer.mmu.printState();

    // printf("\n-- Usando ptr1 --\n");
    // computer.mmu.usePtr(ptr1);
    // computer.mmu.printState();

    // //PRUEBA DELPTR==============
    // //prueba de borrado de puntero
    // printf("\n-- Borrando ptr2 --\n");
    // computer.mmu.delPtr(ptr2);
    // computer.mmu.printState();
    // printf("\n-- Borrando ptr3 --\n");
    // computer.mmu.delPtr(ptr3);
       // computer.mmu.printState();
*/

    Application app(argc, argv);
    app.run();

    return 0;
}
