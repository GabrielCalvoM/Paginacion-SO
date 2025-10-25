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

public:
    Application();
    ~Application();

};

#endif // APP_H
