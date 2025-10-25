#ifndef GTK_FILE_MANAGER_H
#define GTK_FILE_MANAGER_H

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "."
#endif // PROJECT_ROOT


#include <gtkmm/builder.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/textbuffer.h>
#include <string>

class GtkFileManager {
private:
    Gtk::Widget *mDragAndDrop;
    Gtk::Image *mDragAndDropImg;
    Gtk::Label *mFilename;
    Glib::RefPtr<Gtk::TextBuffer> mInstructions;

public:
    GtkFileManager();
    ~GtkFileManager();

    void initialize(Glib::RefPtr<Gtk::Builder> builder);
    void generate_instructions(unsigned int seed, unsigned int nProc, unsigned int nOp);
    void read_file();
    void write_file();

    void on_resize(Gtk::Allocation& allocation);

};

#endif // GTK_FILE_MANAGER_H
