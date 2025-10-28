
#include "sim/computer.h"

#include <thread>
#include <chrono>

std::atomic<bool> Computer::mPaused{false};
std::atomic<unsigned int> Computer::mWaitMs{0};

Computer::Computer(std::condition_variable &cv, std::mutex &mtx) : mmtx(mtx), mcv(cv) {}

void Computer::reset() {
    mmu.reset();
    setPaused(false);
    setWaitThread(false);
    mISet.reset();
}

void Computer::setWaitMs(unsigned int ms) {
    mWaitMs.store(ms);
}

void Computer::setPaused(bool p) {
    mPaused.store(p);
}

bool Computer::isPaused() {
    return mPaused.load();
}

void Computer::setWaitThread(bool w) {
    mWaitThread.store(w);
}

bool Computer::isWaiting() {
    return mWaitThread.load();
}

bool Computer::executeNext()
{
    std::unique_lock<std::mutex> lock(mmtx);
    mcv.wait(lock, [&]{ return !mPaused.load(); });
    lock.unlock();

    const Instruction *instruction;
    if (!(instruction = mISet.next())) return false;

    mmu.executeInstruction(instruction);

    if (mWaitMs.load() > 0) {
        std::this_thread::sleep_for(std::chrono::milliseconds(mWaitMs));
    }
    return true;
}
