#ifndef APP_H
#define APP_H

#include "sim/computer.h"
#include "UI/gtk_paginacion.h"

class Application {
private:
    Computer mOptSimulation;
    Computer mAlgSimulation;
    Computer mFifoSimulation;
    
    GtkPaginacion mWindow;

    IntSet mInstructions;

public:
    Application(int argc, char *argv[]);
    ~Application();

    void run();

};

#endif // APP_H
