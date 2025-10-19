#ifndef GTK_PAGINACION_H
#define GTK_PAGINACION_H

#include <gtkmm/window.h>

#include "UI/gtk_file_manager.h"
#include "UI/gtk_sim_data.h"
#include "UI/gtk_sim_view.h"

class GtkPaginacion {
private:
    Gtk::Window *mWindow;
    GtkFileManager mFManager;
    GtkSimData mData;
    GtkSimView mView;

public:
    GtkPaginacion();
    ~GtkPaginacion();

};

#endif // GTK_PAGINACION_H
