#include <gtkmm/liststore.h>

#include "UI/gtk_sim_data.h"

namespace {
    class ModelColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        Gtk::TreeModelColumn<gint> value;
        Gtk::TreeModelColumn<Glib::ustring> name;
        ModelColumns() { add(value); add(name); }

    };
}

static inline void insertAlgorithm(Gtk::ListStore*, AlgTypeE, const std::string);

// Constructor
GtkSimData::GtkSimData() {}

// Destructor
GtkSimData::~GtkSimData() {}


// --- Getters ---
unsigned int GtkSimData::getSeed() const {
    return (unsigned int) mRandomSeed->get_value_as_int();
}

AlgTypeE GtkSimData::getAlgorithm() const{
    Gtk::TreeModel::Row row = *mAlgorithm->get_active();
    ModelColumns columns;
    return static_cast<AlgTypeE>(row.get_value(columns.value));
}

unsigned int GtkSimData::getNProcesses() const {
    return (unsigned int) mNProcesses->get_value_as_int();
}

unsigned int GtkSimData::getNOperations() const {
    return (unsigned int) mNOperations->get_value_as_int();
}

void GtkSimData::algorithmConnect(std::function<void(AlgTypeE)> func) const {
    mAlgorithm->signal_changed().connect([=]() { func(getAlgorithm()); });
}

void GtkSimData::initialize() {
    mBuilder->get_widget("SeedEntry", mRandomSeed);
    mBuilder->get_widget("AlgorithmCombo", mAlgorithm);
    mBuilder->get_widget("ProcessesEntry", mNProcesses);
    mBuilder->get_widget("OperationsEntry", mNOperations);

    Glib::RefPtr<Gtk::TreeModel> model = mAlgorithm->get_model();
    Gtk::ListStore *list = Glib::RefPtr<Gtk::ListStore>::cast_dynamic(model).get();

    insertAlgorithm(list, AlgTypeE::FIFO, "FIFO");
    insertAlgorithm(list, AlgTypeE::LRU, "LRU");
    insertAlgorithm(list, AlgTypeE::MRU, "MRU");
    insertAlgorithm(list, AlgTypeE::RND, "Random");
    insertAlgorithm(list, AlgTypeE::SC, "Second Chance");
}

static inline void insertAlgorithm(Gtk::ListStore *list, AlgTypeE value, const std::string name) {
    ModelColumns columns;
    Gtk::TreeModel::Row row = *list->append();
    row[columns.value] = (unsigned int) value;
    row[columns.name] = name;
}
