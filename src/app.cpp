#include "app.h"

#include <glibmm/main.h>
#include <iostream>

std::vector<MMUModel> getMMU(Computer &sim);
InfoModel getInfo(Computer &sim);

Application::Application(int argc, char *argv[]) : mWindow(argc, argv, mInstructions) {
    mWindow.initialize();
    mWindow.fileManager().intSetConnect([this]() {
        this->mOptSimulation.copyInstructions(mInstructions);
        this->mAlgSimulation.copyInstructions(mInstructions);

        // Debug: verify instructions were copied and access-sequence built
        auto seq = mInstructions.getAccessSequence();
        std::cout << "[DBG] App: IntSet access sequence size = " << seq.size() << std::endl;
        if (!seq.empty()) {
            std::cout << "[DBG] App: first entries:";
            for (size_t i = 0; i < seq.size() && i < 10; ++i) std::cout << " " << seq[i];
            std::cout << std::endl;
        }

        this->mOptSimulation.mmu.initAlgorithm(AlgTypeE::OPT, mInstructions.getAccessSequence());
    });
    mWindow.simulationData().algorithmConnect([this](AlgTypeE type) { this->mAlgSimulation.mmu.initAlgorithm(type); });
    mWindow.simulationView().ralentizationConnect([this](unsigned int ms) { Computer::setWaitMs(ms); });
    mWindow.simulationView().playConnect([this]() {
        if (mFinishedCount.load() >= 2) return;
        std::unique_lock<std::mutex> lock(mmtx);
        if (!mIsRunning.load()) mSimThread = std::thread([=]{ runSimulation(); });
        mcv.wait(lock, [this]{ return mIsRunning.load(); });
        lock.unlock();

        printf("\t[OPT SIM]\n");
        mOptSimulation.mmu.printState();
        printf("\t[ALG SIM]\n");
        mAlgSimulation.mmu.printState();

        Computer::setPaused(false);
        mcv.notify_all();
    });
    mWindow.simulationView().pauseConnect([this]() {
        if (mFinishedCount.load() >= 2) return;
        std::unique_lock<std::mutex> lock(mmtx);
        Computer::setPaused(true);
        mcv.notify_all();
        
    });
    mWindow.simulationView().resetConnect([this]() {
        mResetRequest.store(true);
        std::unique_lock<std::mutex> lock(mmtx);
        Computer::setPaused(false);
        mcv.notify_all();
        lock.unlock();
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

    mOptThread = std::thread([&]{ runComputer(mOptSimulation); });
    mAlgThread = std::thread([&]{ runComputer(mAlgSimulation); });

    mcv.notify_all();
    int i = 0;

    while (!mResetRequest.load() && mIsRunning.load() && mFinishedCount.load() < 2) {
        std::unique_lock<std::mutex> lock(mmtx);
        std::cout << "Simulación: " << ++i << std::endl;
        mcv.wait(lock, [&]{ return mWaitCount.load() >= 2 || mFinishedCount.load() >= 2 || mResetRequest.load(); });
        if (mResetRequest.load()) break;
        mWaitCount.store(0);

        lock.unlock();

        mIsPrinting.store(true);

        auto mmuOpt = getMMU(mOptSimulation);
        auto infoOpt = getInfo(mOptSimulation);
        auto mmuAlg = getMMU(mAlgSimulation);
        auto infoAlg = getInfo(mAlgSimulation);

        Glib::MainContext::get_default()->invoke([&, this]() {
            mWindow.simulationView().setOptMMU(mmuOpt);
            mWindow.simulationView().setOptInfo(infoOpt);
            mWindow.simulationView().setAlgMMU(mmuAlg);
            mWindow.simulationView().setAlgInfo(infoAlg);

            mWindow.simulationView().showState();

            mIsPrinting.store(false);
            mcv.notify_all();
            return false;
        });
        
        std::unique_lock<std::mutex> lock2(mmtx);
        mcv.wait(lock2, [&]{ return !mIsPrinting.load(); });
        mOptSimulation.setWaitThread(false);
        mAlgSimulation.setWaitThread(false);
        mcv.notify_all();
    }
    
    mOptThread.join();
    mAlgThread.join();

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
    mSimThread.join();

    resetComputer(mOptSimulation);
    resetComputer(mAlgSimulation);

    mWindow.simulationView().resetState();

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
    return {
        sim.mmu.getProcesses(), 
        sim.mmu.getAlgTime(),
        sim.mmu.getRamSize(),
        sim.mmu.getDiskSize(),
        sim.mmu.getLoadedPages(),
        sim.mmu.getUnloadedPages(),

        // swap order
        sim.mmu.getThrashTime(),
        sim.mmu.getFragmentation()};
}
