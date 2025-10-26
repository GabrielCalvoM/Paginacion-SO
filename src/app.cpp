#include "app.h"

Application::Application(int argc, char *argv[]) : mWindow(argc, argv, mInstructions) {
    mWindow.initialize();
}

Application::~Application() {}

void Application::run() {
    mWindow.run();
}
