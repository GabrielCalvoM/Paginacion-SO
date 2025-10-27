#ifndef COMPUTER_H
#define COMPUTER_H

#include <vector>
#include <string>
#include <atomic>

#include "sim/intset.h"
#include "sim/mmu.h"

class Computer {
private:
    static std::vector<Instruction> mInstr;
    unsigned int mInstrIndex = 0;
    std::atomic<bool> mPaused{false};

public:
    void setPaused(bool p);
    bool isPaused() const;

    // Ejecuta la siguiente instrucción si no está en pausa.
    // waitMs: tiempo de espera en milisegundos después de ejecutar.
    // Retorna true si se ejecutó una instrucción, false si estaba en pausa o no había instrucciones.
    bool executeNext(unsigned int waitMs = 0);
    MemoryManagementUnit mmu;
    const unsigned int cores = 1;
    const unsigned int instrPerSec = 1;
    const unsigned int diskAccessTime = 5; // 5s
    
    
};

#endif // COMPUTER_H
