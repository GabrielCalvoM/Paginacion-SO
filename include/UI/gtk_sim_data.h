#ifndef GTK_SIM_DATA_H
#define GTK_SIM_DATA_H

#include <gtkmm/comboboxtext.h>
#include <gtkmm/spinbutton.h>

class GtkSimData {
private:
    Gtk::ComboBoxText *mAlgorithm;
    Gtk::SpinButton *mRandomSeed;
    Gtk::SpinButton *mNProcess; 
    Gtk::SpinButton *mNOperations; 

public:
    GtkSimData();
    ~GtkSimData();

};

#endif // GTK_SIM_DATA_H
