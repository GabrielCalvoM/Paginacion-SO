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


void GtkFileManager::initialize() {
    Gtk::TextView *instructions;

    mBuilder->get_widget("DragAndDrop", mDragAndDrop);
    mBuilder->get_widget("DragAndDropImg", mDragAndDropImg);
    mBuilder->get_widget("Filename", mFilename);
    mBuilder->get_widget("InstructionsView", instructions);
    mInstructions = instructions->get_buffer();

    mBuilder->get_widget("AskSave", mDialog);
    mBuilder->get_widget("OpenFile", mOpenChooser);
    mBuilder->get_widget("SaveFile", mSaveChooser);

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

void GtkFileManager::executeSetConnection() const {
    for (auto func : mSetConnection) {
        func();
    }
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

    executeSetConnection();
    mInstructions->set_text(buffer);
}

void GtkFileManager::readFile(const std::string filepath) {
    std::string buffer = mSet.loadSet(filepath);

    const int index = filepath.find_last_of('/') + 1;
    const std::string filename(filepath, index, filepath.size() - index);
    mFilename->set_text(filename);
    executeSetConnection();
    mInstructions->set_text(buffer);
}

void GtkFileManager::writeFile() {
    const std::string filepath = mSaveChooser->get_filename();
    mSet.saveSet(filepath);

    const int index = filepath.find_last_of('/') + 1;
    const std::string filename(filepath, index, filepath.size() - index);
    mFilename->set_text(filename);
}
