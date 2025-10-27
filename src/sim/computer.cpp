
#include "sim/computer.h"

#include <thread>
#include <chrono>

std::atomic<bool> Computer::mPaused{false};

Computer::Computer(std::condition_variable &cv, std::mutex &mtx) : mmtx(mtx), mcv(cv) {}

void Computer::setPaused(bool p) {
    mPaused.store(p);
}

bool Computer::isPaused() {
    return mPaused.load();
}

bool Computer::executeNext(unsigned int waitMs)
{
    std::unique_lock<std::mutex> lock(mmtx);
    if (mPaused.load()) mcv.wait(lock);
    lock.unlock();

    const Instruction *instruction;
    if (!(instruction = mISet.next())) return false;

    mmu.executeInstruction(instruction);

    if (waitMs > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(waitMs));
    }
    return true;
}
