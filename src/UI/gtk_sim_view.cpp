#include "UI/gtk_sim_view.h"

#include "constants.h"
#include <gtkmm/main.h>
#include <iomanip>

namespace {
    class MMUColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        static const MMUColumns columns;
        Gtk::TreeModelColumn<guint> id;
        Gtk::TreeModelColumn<guint> pid;
        Gtk::TreeModelColumn<bool> loaded;
        Gtk::TreeModelColumn<guint> lAddr;
        Gtk::TreeModelColumn<guint> mAddr;
        Gtk::TreeModelColumn<guint> dAddr;
        Gtk::TreeModelColumn<guint> loadedTime;
        Gtk::TreeModelColumn<bool> mark;
        MMUColumns() { add(id); add(pid); add(loaded); add(lAddr); add(mAddr); add(dAddr); add(loadedTime); add(mark); }

    };
    const MMUColumns MMUColumns::columns;

    class InfoColumns : public Gtk::TreeModel::ColumnRecord {
    public:
        static const InfoColumns columns;
        Gtk::TreeModelColumn<guint> processes;
        Gtk::TreeModelColumn<guint> time;
        Gtk::TreeModelColumn<guint> ram;
        Gtk::TreeModelColumn<guint> vRam;
        Gtk::TreeModelColumn<guint> loadedPages;
        Gtk::TreeModelColumn<guint> unloadedPages;
        Gtk::TreeModelColumn<guint> thrashing;
        Gtk::TreeModelColumn<guint> fragmentation;
        InfoColumns() { add(processes); add(time); add(ram); add(vRam); add(loadedPages); add(unloadedPages); add(thrashing); add(fragmentation); }

    };
    const InfoColumns InfoColumns::columns;

}

static void setRendererText(Gtk::TreeViewColumn *column);
static void setRendererText(Gtk::TreeViewColumn *column, std::function<std::string(Gtk::TreeRow&)> transform);

// Constructor
GtkSimView::GtkSimView() {}

// Destructor
GtkSimView::~GtkSimView() {}

void GtkSimView::setMMU(Glib::RefPtr<Gtk::ListStore> model, const std::vector<MMUModel> mmu) const {
    model->clear();
    
    for (const auto p : mmu) {
        auto row = *model->append();
        
        row[MMUColumns::columns.id] = p.id;
        row[MMUColumns::columns.pid] = p.pid;
        row[MMUColumns::columns.loaded] = p.loaded;
        row[MMUColumns::columns.lAddr] = p.lAddr;
        row[MMUColumns::columns.mAddr] = p.mAddr;
        row[MMUColumns::columns.dAddr] = p.dAddr;
        row[MMUColumns::columns.loadedTime] = p.loadedTime;
        row[MMUColumns::columns.mark] = p.mark;
    }
}

void GtkSimView::setInfo(Glib::RefPtr<Gtk::ListStore> model, const InfoModel info) const {
    Gtk::TreeModel::Row row = *model->get_iter("0");
    row[InfoColumns::columns.processes] = info.processes;
    row[InfoColumns::columns.time] = info.time;
    row[InfoColumns::columns.ram] = info.ram;
    row[InfoColumns::columns.vRam] = info.vRam;
    row[InfoColumns::columns.loadedPages] = info.loadedPages;
    row[InfoColumns::columns.unloadedPages] = info.unloadedPages;
    row[InfoColumns::columns.thrashing] = info.thrashing;
    row[InfoColumns::columns.fragmentation] = info.fragmentation;
}

void GtkSimView::initialize() {
    mBuilder->get_widget("RalentizationScale", mRalenScale);

    mBuilder->get_widget("MmuAlgView", mAlgMmu);
    mBuilder->get_widget("MmuOptView", mOptMmu);

    mBuilder->get_widget("ProcessTimeAlgView", mAlgMain);
    mBuilder->get_widget("ProcessTimeOptView", mOptMain);
    mBuilder->get_widget("RamInfoAlgView", mAlgRam);
    mBuilder->get_widget("RamInfoOptView", mOptRam);
    mBuilder->get_widget("LoadedPagesAlgView", mAlgPages);
    mBuilder->get_widget("LoadedPagesOptView", mOptPages);
    mBuilder->get_widget("ThrashingAlgView", mAlgThrashing);
    mBuilder->get_widget("ThrashingOptView", mOptThrashing);
    mBuilder->get_widget("FragmentationAlgView", mAlgFragmentation);
    mBuilder->get_widget("FragmentationOptView", mOptFragmentation);

    mBuilder->get_widget("ResetButton", mReset);
    mBuilder->get_widget("PlayButton", mPlay);
    mBuilder->get_widget("PauseButton", mPause);

    auto floatToString = [=](float value, int n) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(n) << value;
        return out.str();
    };

    auto simMmuCellRender = [=](Gtk::TreeView *tree) {
        setRendererText(tree->get_column(0));
        setRendererText(tree->get_column(1));
        setRendererText(tree->get_column(2), [=](Gtk::TreeRow &row) { return row[MMUColumns::columns.loaded] ? "X" : ""; });
        setRendererText(tree->get_column(3));
        setRendererText(tree->get_column(4), [=](Gtk::TreeRow &row) { return row[MMUColumns::columns.mAddr] > 0 ?
                                                                        std::to_string(row[MMUColumns::columns.mAddr]) : ""; });
        setRendererText(tree->get_column(5), [=](Gtk::TreeRow &row) { return row[MMUColumns::columns.dAddr] > 0 ?
                                                                        std::to_string(row[MMUColumns::columns.dAddr]) : ""; });
        setRendererText(tree->get_column(6), [=](Gtk::TreeRow &row) { return row[MMUColumns::columns.mAddr] > 0 ?
                                                                        std::to_string(row[MMUColumns::columns.loadedTime]) : ""; });
        setRendererText(tree->get_column(7), [=](Gtk::TreeRow &row) { return row[MMUColumns::columns.mark] ? "X" : ""; });
    };

    simMmuCellRender(mAlgMmu);
    simMmuCellRender(mOptMmu);
    
    auto simTimeCellRender = [=](Gtk::TreeView *tree) {
        setRendererText(tree->get_column(0));
        setRendererText(tree->get_column(1), [=](Gtk::TreeRow &row) { return std::to_string(row[InfoColumns::columns.time]) + "s"; });
    };

    simTimeCellRender(mAlgMain);
    simTimeCellRender(mOptMain);

    auto ramCellRender = [=](Gtk::TreeView *tree) {
        setRendererText(tree->get_column(0), [=](Gtk::TreeRow &row) { return floatToString(row[InfoColumns::columns.ram] / 1024.0, 2) + "KB"; });
        setRendererText(tree->get_column(1), [=](Gtk::TreeRow &row) { return floatToString(row[InfoColumns::columns.ram] * 100.0 / Consts::MAX_RAM, 2) + "%"; });
        setRendererText(tree->get_column(2), [=](Gtk::TreeRow &row) { return floatToString(row[InfoColumns::columns.vRam] / 1024.0, 2) + "KB"; });
        setRendererText(tree->get_column(3), [=](Gtk::TreeRow &row) { return floatToString(row[InfoColumns::columns.vRam] * 100.0 / Consts::MAX_RAM, 2) + "%"; });
    };
    
    ramCellRender(mAlgRam);
    ramCellRender(mOptRam);
    
    auto pagesCellRender = [=](Gtk::TreeView *tree) {
        setRendererText(tree->get_column(0));
        setRendererText(tree->get_column(1));
    };
    
    pagesCellRender(mAlgPages);
    pagesCellRender(mOptPages);

    auto thrashingCellRender = [=](Gtk::TreeView *tree) {
        setRendererText(tree->get_column(0), [=](Gtk::TreeRow &row) { return floatToString(row[InfoColumns::columns.thrashing] / 1024.0, 2) + "s"; });
        setRendererText(tree->get_column(1), [=](Gtk::TreeRow &row) 
        { return floatToString(row[InfoColumns::columns.thrashing] == 0 ? 0 : row[InfoColumns::columns.thrashing] * 100.0 / row[InfoColumns::columns.time], 2) + "%"; });
    };
    
    thrashingCellRender(mAlgThrashing);
    thrashingCellRender(mOptThrashing);

    auto fragmentationCellRender = [=](Gtk::TreeView *tree) {
        setRendererText(tree->get_column(0), [=](Gtk::TreeRow &row) { return std::to_string(row[InfoColumns::columns.fragmentation]) + "KB"; });
    };

    fragmentationCellRender(mAlgFragmentation);
    fragmentationCellRender(mOptFragmentation);

    mOptMmu->get_model()->freeze_notify();
    mAlgMmu->get_model()->freeze_notify();
    mOptMain->get_model()->freeze_notify();
    mAlgMain->get_model()->freeze_notify();

    mReset->signal_clicked().connect([this]() {
        if (mPlay->get_active()) mPlay->set_active(false);
        if (mPause->get_visible()) mPause->set_visible(false);
        if (!mPlay->get_visible()) mPlay->set_visible(true);
        mReset->set_visible(false);
    });
    mPlay->signal_clicked().connect([this]() {
        if (!mPlay->get_active()) return;
        if (!mReset->get_visible()) mReset->set_visible(true);
        mPlay->set_visible(false);
        mPause->set_visible(true);
        mPause->set_active(false);
    });
    mPause->signal_clicked().connect([this]() {
        if (!mPause->get_active()) return;
        mPause->set_visible(false);
        mPlay->set_visible(true);
        mPlay->set_active(false);
    });
}

void GtkSimView::ralentizationConnect(std::function<void(unsigned int)> func) const {
    mRalenScale->signal_value_changed().connect([=]() { func((unsigned int) mRalenScale->get_value()); });
}

void GtkSimView::playConnect(std::function<void()> func) const {
    mPlay->signal_clicked().connect(func);
}

void GtkSimView::pauseConnect(std::function<void()> func) const {
    mPause->signal_clicked().connect(func);
}

void GtkSimView::resetConnect(std::function<void()> func) const {
    mReset->signal_clicked().connect(func);
}

void GtkSimView::showState() const {
    auto modelOptMmu = mOptMmu->get_model();
    auto modelAlgMmu = mAlgMmu->get_model();
    auto modelOptMain = mOptMain->get_model();
    auto modelAlgMain = mAlgMain->get_model();
    
    modelOptMmu->thaw_notify();
    modelAlgMmu->thaw_notify();
    modelOptMain->thaw_notify();
    modelAlgMain->thaw_notify();

    while (Gtk::Main::events_pending())
        Gtk::Main::iteration();

    modelOptMmu->freeze_notify();
    modelAlgMmu->freeze_notify();
    modelOptMain->freeze_notify();
    modelAlgMain->freeze_notify();
}

void GtkSimView::resetState() const {
    setOptMMU({});
    setAlgMMU({});
    setOptInfo({0, 0, 0, 0, 0, 0, 0, 0});
    setAlgInfo({0, 0, 0, 0, 0, 0, 0, 0});

    showState();
}

static void setRendererText(Gtk::TreeViewColumn *column) {
    column->set_alignment(0.5);
    column->set_cell_data_func(*column->get_first_cell(), [=](Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& iter) {
        auto row = *iter;
        auto renderer_text = dynamic_cast<Gtk::CellRendererText*>(cell);
        if (renderer_text) {
            renderer_text->property_xalign() = 0.5;
        }
    });
}

static void setRendererText(Gtk::TreeViewColumn *column, std::function<std::string(Gtk::TreeRow&)> transform) {
    column->set_alignment(0.5);
    column->set_cell_data_func(*column->get_first_cell(), [=](Gtk::CellRenderer* cell, const Gtk::TreeModel::iterator& iter) {
        auto row = *iter;
        auto renderer_text = dynamic_cast<Gtk::CellRendererText*>(cell);
        if (renderer_text) {
            renderer_text->property_text() = transform(row);
            renderer_text->property_xalign() = 0.5;
        }
    });
}
