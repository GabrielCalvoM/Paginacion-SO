#ifndef GTK_SIM_DATA_H
#define GTK_SIM_DATA_H

#include <gtkmm/builder.h>
#include <gtkmm/combobox.h>
#include <gtkmm/spinbutton.h>

#include "alg/algorithm.h"

class GtkSimData {
private:
    Glib::RefPtr<Gtk::Builder> mBuilder;

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

    void setBuilder(Glib::RefPtr<Gtk::Builder> builder) { mBuilder = builder; }

    void initialize();

};

#endif // GTK_SIM_DATA_H
