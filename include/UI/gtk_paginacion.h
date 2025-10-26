#ifndef GTK_PAGINACION_H
#define GTK_PAGINACION_H

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "."
#endif // PROJECT_ROOT


#include <gtkmm.h>
#include <gtkmm/window.h>

#include "UI/gtk_file_manager.h"
#include "UI/gtk_sim_data.h"
#include "UI/gtk_sim_view.h"

class GtkPaginacion {
private:
    Glib::RefPtr<Gtk::Builder> mBuilder;

    Gtk::Main kit;
    Gtk::Window *mWindow;
    GtkFileManager mFManager;
    GtkSimData mData;
    GtkSimView mView;

public:
    GtkPaginacion(int argc, char *argv[], IntSet &set);
    ~GtkPaginacion();

    void initialize();
    void run();

};

#endif // GTK_PAGINACION_H
