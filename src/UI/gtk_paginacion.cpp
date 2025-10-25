#include <gtkmm.h>

#include "UI/gtk_paginacion.h"

// Constructor
GtkPaginacion::GtkPaginacion() {}

// Destructor
GtkPaginacion::~GtkPaginacion() {}

void GtkPaginacion::run(int argc, char *argv[]) {
    Gtk::Main kit(argc, argv);
    mBuilder = Gtk::Builder::create();
    mBuilder->add_from_file(std::string(PROJECT_ROOT) + "/resources/glade/Paginacion.glade");
    mBuilder->get_widget("MainWindow", mWindow);
    mData.initialize(mBuilder);
    mFManager.initialize(mBuilder);

    Gtk::Button *genButton;
    mBuilder->get_widget("GenerateButton", genButton);
    genButton->signal_clicked().connect([this]() {
        mFManager.generate_instructions(this->mData.get_seed(), this->mData.get_n_processes(), this->mData.get_n_operations());
    });
    
    mWindow->set_default_size(962, 509);
    Gtk::Main::run(*mWindow);
}
