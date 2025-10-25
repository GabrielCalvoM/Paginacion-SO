#ifndef GTK_PAGINACION_H
#define GTK_PAGINACION_H

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "."
#endif // PROJECT_ROOT


#include <gtkmm/window.h>

#include "UI/gtk_file_manager.h"
#include "UI/gtk_sim_data.h"
#include "UI/gtk_sim_view.h"

class GtkPaginacion {
private:
    Glib::RefPtr<Gtk::Builder> mBuilder;

    Gtk::Window *mWindow;
    GtkFileManager mFManager;
    GtkSimData mData;
    GtkSimView mView;

public:
    GtkPaginacion();
    ~GtkPaginacion();

    void run(int argc, char *argv[]);

};

#endif // GTK_PAGINACION_H
