#ifndef COMPUTER_H
#define COMPUTER_H

#include <vector>
#include <string>
#include <atomic>
#include <condition_variable>
#include <mutex>

#include "sim/intset.h"
#include "sim/mmu.h"

class Computer {
private:
    static std::atomic<bool> mPaused;
    static std::atomic<unsigned int> mWaitMs;

    IntSet mISet;
    std::atomic<bool> mWaitThread{false};

    std::mutex &mmtx;
    std::condition_variable &mcv;

public:
    Computer(std::condition_variable &cv, std::mutex &mtx);

    void reset();

    static void setPaused(bool p);
    static bool isPaused();
    static void setWaitMs(unsigned int ms);

    void setWaitThread(bool w);
    bool isWaiting();

    // Ejecuta la siguiente instrucción si no está en pausa.
    // waitMs: tiempo de espera en milisegundos después de ejecutar.
    // Retorna true si se ejecutó una instrucción, false si estaba en pausa o no había instrucciones.
    bool executeNext();
    void copyInstructions(IntSet iset) { mISet.copyInstructions(iset.getVec()); };

    MemoryManagementUnit mmu;
    const unsigned int cores = 1;
    const unsigned int instrPerSec = 1;
    const unsigned int diskAccessTime = 5; // 5s
    
};

#endif // COMPUTER_H
