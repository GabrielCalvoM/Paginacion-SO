#ifndef APP_H
#define APP_H

#include <condition_variable>
#include <mutex>

#include "sim/computer.h"
#include "UI/gtk_paginacion.h"

class Application {
private:
    std::mutex mmtx;
    std::condition_variable mcv;

    Computer mOptSimulation = Computer(mcv, mmtx);
    Computer mAlgSimulation = Computer(mcv, mmtx);
    
    GtkPaginacion mWindow;

    IntSet mInstructions;

public:
    Application(int argc, char *argv[]);
    ~Application();

    void run();

};

#endif // APP_H
