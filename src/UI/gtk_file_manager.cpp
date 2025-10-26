#include <cmath>
#include <glibmm.h>
#include <gtkmm/button.h>
#include <gtkmm/eventbox.h>
#include <gtkmm/textview.h>
#include <random>
#include <iostream>

#include "UI/gtk_file_manager.h"

namespace {
    typedef struct {
        bool exists;
        std::vector<unsigned int> pointers;
    } process_t;
}

// Constructor
GtkFileManager::GtkFileManager(IntSet &set) : mSet(set) {}

// Destructor
GtkFileManager::~GtkFileManager() {}


void GtkFileManager::initialize(Glib::RefPtr<Gtk::Builder> builder) {
    Gtk::TextView *instructions;

    builder->get_widget("DragAndDrop", mDragAndDrop);
    builder->get_widget("DragAndDropImg", mDragAndDropImg);
    builder->get_widget("Filename", mFilename);
    builder->get_widget("InstructionsView", instructions);
    mInstructions = instructions->get_buffer();

    builder->get_widget("AskSave", mDialog);
    builder->get_widget("OpenFile", mOpenChooser);
    builder->get_widget("SaveFile", mSaveChooser);

    Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file(std::string(PROJECT_ROOT) + "/resources/images/Drag_and_drop2.png");
    mDragAndDropImg->set(pixbuf);
    mDragAndDrop->add_events(Gdk::BUTTON_PRESS_MASK);
    mDragAndDrop->drag_dest_set({Gtk::TargetEntry("text/uri-list")}, Gtk::DEST_DEFAULT_ALL, Gdk::ACTION_COPY);

    mDragAndDrop->signal_drag_begin().connect([this](const Glib::RefPtr<Gdk::DragContext>& context) {
        
    });

    mDragAndDrop->signal_drag_data_received().connect([this](const Glib::RefPtr<Gdk::DragContext>& context, int x, int y,
                                                             const Gtk::SelectionData& data, guint info, guint time) {
        std::string uri = data.get_uris()[0];
        std::string filepath = Glib::filename_from_uri(uri);
        std::cout << "filepath: " << filepath << std::endl;
        this->readFile(filepath);
    });

    mDragAndDrop->signal_button_press_event().connect([this](GdkEventButton* event) {
        if (event->type != GDK_BUTTON_PRESS) return true;

        if (this->mOpenChooser->run() == Gtk::RESPONSE_OK) {
            std::string filename = this->mOpenChooser->get_filename();
            this->readFile(filename);
        }

        this->mOpenChooser->hide();
        return true;
    });
}

void GtkFileManager::generateInstructions(unsigned int seed, unsigned int nProc, unsigned int nOp) {
    std::string buffer = mSet.generateInstructions(seed, nProc, nOp);

    mSaveChooser->set_filename(std::string(PROJECT_ROOT) + "/files");
    mSaveChooser->set_current_name("untitled");

    if (mDialog->run() == Gtk::RESPONSE_ACCEPT) {
        mDialog->hide();
        
        if (mSaveChooser->run() == Gtk::RESPONSE_OK) {
            writeFile();
        }

        mSaveChooser->hide();
    }

    mDialog->hide();


    mInstructions->set_text(buffer);
}

void GtkFileManager::readFile(const std::string filepath) {
    std::string buffer = mSet.loadSet(filepath);

    const int index = filepath.find_last_of('/') + 1;
    const std::string filename(filepath, index, filepath.size() - index);
    mFilename->set_text(filename);
    mInstructions->set_text(buffer);
}

void GtkFileManager::writeFile() {
    const std::string filepath = mSaveChooser->get_filename();
    mSet.saveSet(filepath);

    const int index = filepath.find_last_of('/') + 1;
    const std::string filename(filepath, index, filepath.size() - index);
    mFilename->set_text(filename);
}

void GtkFileManager::onResize(Gtk::Allocation& allocation) {
    Glib::RefPtr<Gdk::Pixbuf> pixbuf = mDragAndDropImg->get_pixbuf();
    int ancho = allocation.get_width();
    int alto = allocation.get_height();

    auto pixbuf_scaled = pixbuf->scale_simple(ancho, alto, Gdk::INTERP_BILINEAR);
    mDragAndDropImg->set(pixbuf_scaled);
}
