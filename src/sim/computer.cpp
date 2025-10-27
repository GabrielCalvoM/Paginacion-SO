
#include "sim/computer.h"

#include <thread>
#include <chrono>

// vector estático de instrucciones (vacío por ahora)
std::vector<Instruction> Computer::mInstr = {};

void Computer::setPaused(bool p) {
    mPaused.store(p);
}

bool Computer::isPaused() const {
    return mPaused.load();
}


bool Computer::executeNext(unsigned int waitMs)
{
    if (mPaused.load()) return false;
    if (mInstrIndex >= mInstr.size()) return false;

    IntSet iset;
    iset.pushInstruction(mInstr[mInstrIndex]);
    mmu.executeIntSet(iset);

    ++mInstrIndex;

    if (waitMs > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(waitMs));
    }
    return true;
}
