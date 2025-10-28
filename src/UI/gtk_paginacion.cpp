#include "UI/gtk_paginacion.h"

// Constructor
GtkPaginacion::GtkPaginacion(int argc, char *argv[], IntSet &set) : kit(argc, argv), mFManager(set) {}

// Destructor
GtkPaginacion::~GtkPaginacion() {}

void GtkPaginacion::initialize() {
    mBuilder = Gtk::Builder::create();
    mBuilder->add_from_file(std::string(PROJECT_ROOT) + "/resources/glade/Paginacion.glade");
    mBuilder->get_widget("MainWindow", mWindow);

    mFManager.setBuilder(mBuilder);
    mData.setBuilder(mBuilder);
    mView.setBuilder(mBuilder);

    mFManager.initialize();
    mData.initialize();
    mView.initialize();

    Gtk::Button *genButton;
    mBuilder->get_widget("GenerateButton", genButton);
    genButton->signal_clicked().connect([this]{
        mFManager.generateInstructions(this->mData.getSeed(), this->mData.getNProcesses(), this->mData.getNOperations());
    });
    
    mWindow->set_default_size(962, 509);
    mWindow->signal_delete_event().connect([&](GdkEventAny*){ mWindow->hide(); return true; });
}

void GtkPaginacion::run() {
    Gtk::Main::run(*mWindow);
}
