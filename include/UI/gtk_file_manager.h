#ifndef GTK_FILE_MANAGER_H
#define GTK_FILE_MANAGER_H

#ifndef PROJECT_ROOT
#define PROJECT_ROOT "."
#endif // PROJECT_ROOT


#include <gtkmm/builder.h>
#include <gtkmm/dialog.h>
#include <gtkmm/filechooserdialog.h>
#include <gtkmm/image.h>
#include <gtkmm/label.h>
#include <gtkmm/textbuffer.h>
#include <string>

#include "sim/intset.h"

class GtkFileManager {
private:
    Gtk::Widget *mDragAndDrop;
    Gtk::Image *mDragAndDropImg;
    Gtk::Label *mFilename;
    Glib::RefPtr<Gtk::TextBuffer> mInstructions;

    Gtk::Dialog *mDialog;
    Gtk::FileChooserDialog *mOpenChooser;
    Gtk::FileChooserDialog *mSaveChooser;

    IntSet &mSet;

public:
    GtkFileManager(IntSet &set);
    ~GtkFileManager();

    void initialize(Glib::RefPtr<Gtk::Builder> builder);
    void generateInstructions(unsigned int seed, unsigned int nProc, unsigned int nOp);
    void readFile(const std::string filename);
    void writeFile();

    void onResize(Gtk::Allocation& allocation);

};

#endif // GTK_FILE_MANAGER_H
