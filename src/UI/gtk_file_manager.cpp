#include <cmath>
#include <gtkmm/button.h>
#include <gtkmm/textview.h>
#include <random>

#include "UI/gtk_file_manager.h"

namespace {
    typedef struct {
        bool exists;
        std::vector<unsigned int> pointers;
    } process_t;
}

static inline void insert_instruction(Glib::RefPtr<Gtk::TextBuffer> text, char instruction[], char args[]);

// Constructor
GtkFileManager::GtkFileManager() {}

// Destructor
GtkFileManager::~GtkFileManager() {}


void GtkFileManager::initialize(Glib::RefPtr<Gtk::Builder> builder) {
    Gtk::TextView *instructions;

    builder->get_widget("DragAndDrop", mDragAndDrop);
    builder->get_widget("DragAndDropImg", mDragAndDropImg);
    builder->get_widget("Filename", mFilename);
    builder->get_widget("InstructionsView", instructions);
    mInstructions = instructions->get_buffer();

    Glib::RefPtr<Gdk::Pixbuf> pixbuf = Gdk::Pixbuf::create_from_file(std::string(PROJECT_ROOT) + "/resources/images/Drag_and_drop2.png");
    mDragAndDropImg->set(pixbuf);
}

void GtkFileManager::generate_instructions(unsigned int seed, unsigned int nProc, unsigned int nOp) {
    if (nOp == 0 || nProc == 0) return;
    mInstructions->set_text("");

    std::vector<process_t> processes(nProc, (process_t){TRUE, {}});
    std::vector<bool> pointers;

    const char *instrList[] = {"new", "use", "delete", "kill"};

    std::mt19937 genSeed(seed);
    std::uniform_int_distribution<> genSpace(1, 2e4);
    std::uniform_int_distribution<> genPtr;
    std::uniform_int_distribution<> genProc(0, nProc - 1);
    std::discrete_distribution<> genInstr({0.3, 0.5, 0.15, 0.05});

    char instr[7], args[40];
    unsigned int instrI, ptrI, processI, pointersExists = 0, processesExists = nProc;

    for (unsigned int i = 0; i < nOp; ++i) {
        while ((instrI = i <= nProc / 10 || pointersExists == 0 ? 0                         // crear punteros si no hay o si no han pasado más de nProc/10 instrucciones
            : genInstr(genSeed)) == 3
            && i / (nProc - processesExists + 1) < std::ceil((double)nOp / (double)nProc)); // no hacer kill antes de nOp/nProc instrucciones

        strcpy(instr, instrList[instrI]);
        
        // generar use o delete
        if (instrI == 1 || instrI == 2) {
            while (!pointers[ptrI = genPtr(genSeed, decltype(genPtr)::param_type(0, pointers.size() - 1))]);    // solo obtener punteros válidos
            sprintf(args, "%d", ptrI);
            insert_instruction(mInstructions, instr, args);
            if (instrI == 2) { pointers[ptrI] = FALSE; --pointersExists; }                                      // invalidar si es delete
            continue;
        }

        while (!processes[processI = genProc(genSeed)].exists); // solo obtener procesos corriendo
        
        // generar kill
        if (instrI == 3) {
            sprintf(args, "%d", processI + 1);
            insert_instruction(mInstructions, instr, args);
            std::vector ptrId = processes[processI].pointers;
            for (unsigned int i = 0; i < ptrId.size(); ++i) { pointers[ptrId[i]] = FALSE; --pointersExists; }   // matar todos los punteros del proceso
            processes[processI].exists = FALSE;                                                                 // matar el proceso
            --processesExists;
            continue;
        }

        // generar new
        unsigned int size = genSpace(genSeed);
        sprintf(args, "%d, %d", processI + 1, size);
        insert_instruction(mInstructions, instr, args);
        pointers.push_back(TRUE); ++pointersExists;     // agregar puntero
    }
}

void GtkFileManager::read_file() {

}

void GtkFileManager::write_file() {

}

void GtkFileManager::on_resize(Gtk::Allocation& allocation) {
    Glib::RefPtr<Gdk::Pixbuf> pixbuf = mDragAndDropImg->get_pixbuf();
    int ancho = allocation.get_width();
    int alto = allocation.get_height();

    auto pixbuf_scaled = pixbuf->scale_simple(ancho, alto, Gdk::INTERP_BILINEAR);
    mDragAndDropImg->set(pixbuf_scaled);
}

static inline void insert_instruction(Glib::RefPtr<Gtk::TextBuffer> text, char instruction[], char args[]) {
    static Glib::ustring buffer = Glib::ustring::compose("%1(%2)\n", instruction, args);
    text->insert(text->end(), buffer);
}
