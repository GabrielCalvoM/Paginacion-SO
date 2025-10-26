#ifndef GTK_SIM_DATA_H
#define GTK_SIM_DATA_H

#include <gtkmm/builder.h>
#include <gtkmm/combobox.h>
#include <gtkmm/spinbutton.h>

#include "alg/algorithm.h"

class GtkSimData {
private:
    Gtk::SpinButton *mRandomSeed;
    Gtk::ComboBox *mAlgorithm;
    Gtk::SpinButton *mNProcesses; 
    Gtk::SpinButton *mNOperations; 

public:
    GtkSimData();
    ~GtkSimData();

    // --- Getters ---
    unsigned int getSeed();
    AlgTypeE getAlgorithm();
    unsigned int getNProcesses();
    unsigned int getNOperations();

    void initialize(Glib::RefPtr<Gtk::Builder> builder);

};

#endif // GTK_SIM_DATA_H
