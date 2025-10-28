#include "app.h"

#include <thread>
#include <iostream>

std::vector<MMUModel> getMMU(Computer &sim);
InfoModel getInfo(Computer &sim);

Application::Application(int argc, char *argv[]) : mWindow(argc, argv, mInstructions) {
    mWindow.initialize();
    mWindow.fileManager().intSetConnect([this]() {
        this->mOptSimulation.copyInstructions(mInstructions);
        this->mAlgSimulation.copyInstructions(mInstructions);

        this->mOptSimulation.mmu.initAlgorithm(AlgTypeE::OPT, mInstructions.getAccessSequence());
    });
    mWindow.simulationData().algorithmConnect([this](AlgTypeE type) { this->mAlgSimulation.mmu.initAlgorithm(type); });
    mWindow.simulationView().ralentizationConnect([this](unsigned int ms) { Computer::setWaitMs(ms); });
    mWindow.simulationView().playConnect([this]() {
        std::unique_lock<std::mutex> lock(mmtx);
        if (!mIsRunning.load()) std::thread([=]{ runSimulation(); }).detach();
        mcv.wait(lock, [this]{ return mIsRunning.load(); });
        lock.unlock();

        Computer::setPaused(false);
    });
    mWindow.simulationView().pauseConnect([this]() {
        std::unique_lock<std::mutex> lock(mmtx);
        Computer::setPaused(true);
        mcv.notify_all();
    });
    mWindow.simulationView().resetConnect([this]() {
        mResetRequest.store(true);
        std::unique_lock<std::mutex> lock(mmtx);
        mcv.notify_all();
        resetSimulation();
        mResetRequest.store(false);
    });
}

Application::~Application() {}

void Application::run() {
    mWindow.run();
}

void Application::runSimulation() {
    mIsRunning.store(true);

    std::thread tOpt([&]{ runComputer(mOptSimulation); });
    std::thread tAlg([&]{ runComputer(mAlgSimulation); });

    mcv.notify_all();
    int i = 0;

    while (!mResetRequest.load() && mIsRunning.load() && mFinishedCount < 2) {
        std::unique_lock<std::mutex> lock(mmtx);
        std::cout << "Simulación: " << ++i << std::endl;
        mcv.wait(lock, [&] { return mWaitCount.load() >= 2 || mFinishedCount.load() >= 2 || mResetRequest.load(); });
        mWaitCount.store(0);

        lock.unlock();

        //mWindow.simulationView().setOptMMU(getMMU(mOptSimulation));
        mWindow.simulationView().setOptInfo(getInfo(mOptSimulation));
        //mWindow.simulationView().setAlgMMU(getMMU(mAlgSimulation));
        mWindow.simulationView().setAlgInfo(getInfo(mAlgSimulation));
        
        std::unique_lock<std::mutex> lock2(mmtx);
        mOptSimulation.setWaitThread(false);
        mAlgSimulation.setWaitThread(false);
        mcv.notify_all();
    }
    
    tOpt.join();
    tAlg.join();

    mIsRunning.store(false);
    
    std::cout << "Fin de Simulación" << std::endl;
}

void Application::runComputer(Computer &sim) {
    int i = 0;
    while (!mResetRequest.load()
        && sim.executeNext()) {
        mWaitCount.fetch_add(1);
        std::cout << "  Computador: " << ++i << std::endl;
        mcv.notify_all();
        sim.setWaitThread(true);
        std::unique_lock<std::mutex> lock(mmtx);
        mcv.wait(lock, [&]{ return !sim.isWaiting() || mResetRequest.load(); });
    }
    
    mFinishedCount.fetch_add(1);
    mcv.notify_all();

    std::cout << "Fin de Computador" << std::endl;
}

void Application::resetSimulation() {
    resetComputer(mOptSimulation);
    resetComputer(mAlgSimulation);

    mIsRunning.store(false);
    mWaitCount.store(0);
    mFinishedCount.store(0);
}

void Application::resetComputer(Computer &sim) {
    sim.reset();
}

std::vector<MMUModel> getMMU(Computer &sim) {
    const auto ram = sim.mmu.ram();
    std::vector<MMUModel> vec;

    for (const auto p : ram) {
        vec.push_back({p.id, 0, p.isInRealMem(), p.id, p.isInRealMem() ? p.getPhysicalDir() : 0, 0, 0, p.hasSecondChance()});
    }

    return vec;
}

InfoModel getInfo(Computer &sim) {
    return {sim.mmu.getProcesses(), 0, sim.mmu.getRamSize(), sim.mmu.getDiskSize(), sim.mmu.getLoadedPages(), sim.mmu.getUnloadedPages(), 0, 0};

}
