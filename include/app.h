#ifndef APP_H
#define APP_H

#include <atomic>
#include <condition_variable>
#include <mutex>
#include <thread>

#include "sim/computer.h"
#include "UI/gtk_paginacion.h"

class Application {
private:
    std::mutex mmtx;
    std::condition_variable mcv;
    
    std::thread mSimThread;
    std::thread mOptThread;
    std::thread mAlgThread;

    Computer mOptSimulation = Computer(mcv, mmtx); // MMU 1
    Computer mAlgSimulation = Computer(mcv, mmtx); // MMU 2
    
    GtkPaginacion mWindow;

    IntSet mInstructions;

    std::atomic<bool> mIsRunning = false;
    std::atomic<bool> mResetRequest = false;
    std::atomic<bool> mIsPrinting = false;
    std::atomic<unsigned int> mWaitCount = 0;
    std::atomic<unsigned int> mFinishedCount = 0;

    void runSimulation();
    void runComputer(Computer &sim);
    void resetSimulation();
    void resetComputer(Computer &sim);

public:
    Application(int argc, char *argv[]);
    ~Application();

    void run();

};

#endif // APP_H
