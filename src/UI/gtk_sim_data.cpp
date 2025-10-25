#include <gtkmm/liststore.h>

#include "UI/gtk_sim_data.h"

namespace {
    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        Gtk::TreeModelColumn<guint> value;
        Gtk::TreeModelColumn<Glib::ustring> name;
        ModelColumns() { add(value); add(name); }

    };
}

static inline void insert_algorithm(Gtk::ListStore*, AlgTypeE, const std::string);

// Constructor
GtkSimData::GtkSimData() {}

// Destructor
GtkSimData::~GtkSimData() {}


// --- Getters ---
unsigned int GtkSimData::get_seed() {
    return (unsigned int) mRandomSeed->get_value_as_int();
}

AlgTypeE GtkSimData::get_algorithm(){
    Gtk::TreeModel::Row row = *mAlgorithm->get_active();
    ModelColumns columns;
    return static_cast<AlgTypeE>(row.get_value(columns.value));
}

unsigned int GtkSimData::get_n_processes() {
    return (unsigned int) mNProcesses->get_value_as_int();
}

unsigned int GtkSimData::get_n_operations() {
    return (unsigned int) mNOperations->get_value_as_int();
}


void GtkSimData::initialize(Glib::RefPtr<Gtk::Builder> builder) {
    builder->get_widget("SeedEntry", mRandomSeed);
    builder->get_widget("AlgorithmCombo", mAlgorithm);
    builder->get_widget("ProcessesEntry", mNProcesses);
    builder->get_widget("OperationsEntry", mNOperations);

    Glib::RefPtr<Gtk::TreeModel> model = mAlgorithm->get_model();
    Gtk::ListStore *list = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(model).get();

    insert_algorithm(list, AlgTypeE::FIFO, "FIFO");
    insert_algorithm(list, AlgTypeE::LRU, "LRU");
    insert_algorithm(list, AlgTypeE::MRU, "MRU");
    insert_algorithm(list, AlgTypeE::RND, "Random");
    insert_algorithm(list, AlgTypeE::SC, "Second Chance");
}

static inline void insert_algorithm(Gtk::ListStore *list, AlgTypeE value, const std::string name) {
    ModelColumns columns;
    Gtk::TreeModel::Row row = *list->append();
    row[columns.value] = (unsigned int) value;
    row[columns.name] = name;
}
