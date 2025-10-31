#include "UI/gtk_sim_view.h"

#include "constants.h"
#include <gtkmm/main.h>
#include <gtkmm/cellrenderertext.h>
#include <gdkmm/rgba.h>
#include <algorithm>
#include <cmath>
#include <iomanip>
#include <string>
#include <vector>

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

    static void hsvToRgb(double h, double s, double v, double &r, double &g, double &b) {
        if (s <= 0.0) {
            r = g = b = v;
            return;
        }

        h = std::fmod(h, 1.0);
        if (h < 0.0) h += 1.0;
        double sector = h * 6.0;
        int i = static_cast<int>(std::floor(sector));
        double f = sector - i;
        double p = v * (1.0 - s);
        double q = v * (1.0 - s * f);
        double t = v * (1.0 - s * (1.0 - f));

        switch (i % 6) {
            case 0: r = v; g = t; b = p; break;
            case 1: r = q; g = v; b = p; break;
            case 2: r = p; g = v; b = t; break;
            case 3: r = p; g = q; b = v; break;
            case 4: r = t; g = p; b = v; break;
            case 5: default: r = v; g = p; b = q; break;
        }
    }

    // Deterministic palette: each PID maps to a unique HSV tuple.
    Gdk::RGBA pidToColor(unsigned int pid) {
        Gdk::RGBA color;
        if (pid == 0) {
            color.set_rgba(0.96, 0.96, 0.96, 1.0);
            return color;
        }

        // Spread hues using the golden ratio to avoid clustering.
        constexpr double goldenConjugate = 0.61803398875;
        double hue = std::fmod(static_cast<double>(pid) * goldenConjugate, 1.0);
        double saturation = 0.55 + 0.25 * std::fmod(static_cast<double>((pid * 37) % 100) / 100.0, 1.0);
        double value = 0.85 + 0.10 * std::fmod(static_cast<double>((pid * 53) % 100) / 100.0, 1.0);

        double r, g, b;
        hsvToRgb(hue, std::min(saturation, 0.9), std::min(value, 0.98), r, g, b);
        color.set_rgba(r, g, b, 1.0);
        return color;
    }
}

static void setRendererText(Gtk::TreeViewColumn *column, bool colorByPid = false);
static void setRendererText(Gtk::TreeViewColumn *column, std::function<std::string(Gtk::TreeRow&)> transform, bool colorByPid = false);
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

void GtkSimView::setOptMMU(const std::vector<MMUModel> mmu) const {
    setMMU(Glib::RefPtr<Gtk::ListStore>::cast_dynamic(mOptMmu->get_model()), mmu);
    updateRamBar(mRamOptBar,
                 const_cast<std::vector<Gdk::RGBA>&>(mRamOptColors),
                 const_cast<std::vector<Glib::ustring>&>(mRamOptLabels),
                 mmu);
}

void GtkSimView::setAlgMMU(const std::vector<MMUModel> mmu) const {
    setMMU(Glib::RefPtr<Gtk::ListStore>::cast_dynamic(mAlgMmu->get_model()), mmu);
    updateRamBar(mRamAlgBar,
                 const_cast<std::vector<Gdk::RGBA>&>(mRamAlgColors),
                 const_cast<std::vector<Glib::ustring>&>(mRamAlgLabels),
                 mmu);
}

bool GtkSimView::onRamDraw(const Cairo::RefPtr<Cairo::Context> &cr,
                           const std::vector<Gdk::RGBA> &colors,
                           const std::vector<Glib::ustring> &labels,
                           Gtk::DrawingArea *area) const {
    if (!area) return false;

    const double width = area->get_allocated_width();
    const double height = area->get_allocated_height();
    if (width <= 0.0 || height <= 0.0) return false;

    const unsigned int totalFrames = colors.size();
    if (totalFrames == 0) return false;

    const double barHeight = height;
    const double barWidth = width;

    const double padding = 4.0;
    const double innerHeight = barHeight - padding * 2.0;
    const double innerWidth = barWidth - padding * 2.0;
    if (innerHeight <= 0.0 || innerWidth <= 0.0) return false;
    const double frameWidth = innerWidth / static_cast<double>(totalFrames);

    // Draw background border
    cr->set_source_rgb(0.85, 0.85, 0.85);
    cr->rectangle(0.0, 0.0, barWidth, barHeight);
    cr->fill();

    cr->set_source_rgb(0.2, 0.2, 0.2);
    cr->set_line_width(1.0);
    cr->rectangle(0.5, 0.5, barWidth - 1.0, barHeight - 1.0);
    cr->stroke();

    // Draw frames as segments from left to right
    for (unsigned int i = 0; i < totalFrames; ++i) {
        const double x = padding + i * frameWidth;
        const double y = padding;

        const Gdk::RGBA &color = colors[i];
    const double segmentWidth = std::max(frameWidth, 1.0);
    cr->set_source_rgba(color.get_red(), color.get_green(), color.get_blue(), 1.0);
    cr->rectangle(x, y, segmentWidth, innerHeight);
        cr->fill();

        // Draw optional PID label when there's room
        const Glib::ustring &label = labels[i];
        if (!label.empty() && segmentWidth >= 12.0) {
            cr->set_source_rgb(0.1, 0.1, 0.1);
            Cairo::TextExtents extents;
            cr->select_font_face("Sans", Cairo::FONT_SLANT_NORMAL, Cairo::FONT_WEIGHT_NORMAL);
            cr->set_font_size(std::min(innerHeight - 4.0, segmentWidth - 2.0));
            cr->get_text_extents(label, extents);
            const double textX = x + (segmentWidth - extents.width) * 0.5 - extents.x_bearing;
            const double textY = y + innerHeight * 0.5 + extents.height * 0.5 - extents.y_bearing;
            cr->move_to(textX, textY);
            cr->show_text(label);
        }
    }

    return true;
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

void GtkSimView::updateRamBar(Gtk::DrawingArea *area,
                              std::vector<Gdk::RGBA> &colors,
                              std::vector<Glib::ustring> &labels,
                              const std::vector<MMUModel> &mmu) const {
    if (!area) return;

    const unsigned int kTotalFrames = Consts::MAX_RAM / Consts::PAGE_SIZE;
    if (colors.size() != kTotalFrames) colors.assign(kTotalFrames, pidToColor(0));
    if (labels.size() != kTotalFrames) labels.assign(kTotalFrames, Glib::ustring(""));

    std::fill(colors.begin(), colors.end(), pidToColor(0));
    std::fill(labels.begin(), labels.end(), Glib::ustring(""));

    for (const auto &entry : mmu) {
        if (!entry.loaded || entry.mAddr == 0) continue;
        unsigned int idx = entry.mAddr - 1;
        if (idx >= kTotalFrames) continue;
        colors[idx] = pidToColor(entry.pid);
        labels[idx] = entry.pid == 0 ? Glib::ustring("") : Glib::ustring(std::to_string(entry.pid));
    }

    area->queue_draw();
}

void GtkSimView::initialize() {
    mBuilder->get_widget("RalentizationScale", mRalenScale);

    mBuilder->get_widget("MmuAlgView", mAlgMmu);
    mBuilder->get_widget("MmuOptView", mOptMmu);

    mBuilder->get_widget("RamAlgView", mRamAlgBar);
    mBuilder->get_widget("RamOptView", mRamOptBar);

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

    auto setupRamArea = [&](Gtk::DrawingArea *area,
                            std::vector<Gdk::RGBA> &colors,
                            std::vector<Glib::ustring> &labels) {
        if (!area) return;
        colors.assign(Consts::MAX_RAM / Consts::PAGE_SIZE, pidToColor(0));
        labels.assign(colors.size(), Glib::ustring(""));

        area->set_size_request(-1, 60);
        area->set_hexpand(true);
        area->set_vexpand(false);

        area->signal_draw().connect([=, &colors, &labels](const Cairo::RefPtr<Cairo::Context> &cr) {
            return onRamDraw(cr, colors, labels, area);
        });

        area->queue_draw();
    };

    setupRamArea(mRamOptBar, mRamOptColors, mRamOptLabels);
    setupRamArea(mRamAlgBar, mRamAlgColors, mRamAlgLabels);

    mBuilder->get_widget("ResetButton", mReset);
    mBuilder->get_widget("PlayButton", mPlay);
    mBuilder->get_widget("PauseButton", mPause);

    auto floatToString = [=](double value, int n) {
        std::ostringstream out;
        out << std::fixed << std::setprecision(n) << value;
        return out.str();
    };

    auto simMmuCellRender = [=](Gtk::TreeView *tree) {
        setRendererText(tree->get_column(0), true);
        setRendererText(tree->get_column(1), true);
        setRendererText(tree->get_column(2), [=](Gtk::TreeRow &row) { return row[MMUColumns::columns.loaded] ? "X" : ""; }, true);
        setRendererText(tree->get_column(3), true);
        setRendererText(tree->get_column(4), [=](Gtk::TreeRow &row)
            { return row[MMUColumns::columns.mAddr] > 0 ?
                std::to_string(row[MMUColumns::columns.mAddr]) : ""; }, true);
        setRendererText(tree->get_column(5), [=](Gtk::TreeRow &row)
            { return row[MMUColumns::columns.dAddr] > 0 ?
                std::to_string(row[MMUColumns::columns.dAddr]) : ""; }, true);
        setRendererText(tree->get_column(6), [=](Gtk::TreeRow &row)
            { return row[MMUColumns::columns.mAddr] > 0 ?
                std::to_string(row[MMUColumns::columns.loadedTime]) + "s" : ""; }, true);
        setRendererText(tree->get_column(7), [=](Gtk::TreeRow &row) { return row[MMUColumns::columns.mark] ? "X" : ""; }, true);
        Glib::RefPtr<Gtk::ListStore>::cast_dynamic(tree->get_model())
            ->set_sort_column(MMUColumns::columns.id, Gtk::SORT_ASCENDING);
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
        // column 0 -> total thrashing time in seconds
        if (auto col0 = tree->get_column(0)) {
            col0->set_alignment(0.5);
            auto first = col0->get_first_cell();
            if (first) {
                col0->set_cell_data_func(*first, [=](Gtk::CellRenderer *cell, const Gtk::TreeModel::iterator &iter) {
                    auto row = *iter;
                    auto renderer_text = dynamic_cast<Gtk::CellRendererText *>(cell);
                    if (!renderer_text) return;

                    const guint thrash = row[InfoColumns::columns.thrashing];
                    const guint time = row[InfoColumns::columns.time];
                    const double percent = (thrash == 0 || time == 0) ? 0.0 : thrash * 100.0 / time;

                    renderer_text->property_text() = std::to_string(thrash) + "s";
                    renderer_text->property_xalign() = 0.5;
                   
                    renderer_text->property_foreground() = "#000000";
                    renderer_text->property_foreground_set() = true;

                    // background red if percent >= 50
                    if (percent >= 50.0) {
                        renderer_text->property_cell_background() = "#FF4D4D"; // soft red background
                        renderer_text->property_cell_background_set() = true;
                    } else {
                        renderer_text->property_cell_background_set() = false;
                    }
                });
            }
        }

        // Column 1 -> percentage
        if (auto col1 = tree->get_column(1)) {
            col1->set_alignment(0.5);
            auto first = col1->get_first_cell();
            if (first) {
                col1->set_cell_data_func(*first, [=](Gtk::CellRenderer *cell, const Gtk::TreeModel::iterator &iter) {
                    auto row = *iter;
                    auto renderer_text = dynamic_cast<Gtk::CellRendererText *>(cell);
                    if (!renderer_text) return;

                    const guint thrash = row[InfoColumns::columns.thrashing];
                    const guint time = row[InfoColumns::columns.time];
                    const double percent = (thrash == 0 || time == 0) ? 0.0 : thrash * 100.0 / time;

                    // 2 decimales
                    std::ostringstream out;
                    out << std::fixed << std::setprecision(2) << percent;
                    renderer_text->property_text() = out.str() + "%";
                    renderer_text->property_xalign() = 0.5;
                    
                    renderer_text->property_foreground() = "#000000";
                    renderer_text->property_foreground_set() = true;

                    // background red if percent >= 50
                    if (percent >= 50.0) {
                        renderer_text->property_cell_background() = "#FF4D4D"; 
                        renderer_text->property_cell_background_set() = true;
                    } else {
                        renderer_text->property_cell_background_set() = false;
                    }
                });
            }
        }
    };
    
    thrashingCellRender(mAlgThrashing);
    thrashingCellRender(mOptThrashing);

    auto fragmentationCellRender = [=](Gtk::TreeView *tree) {
        setRendererText(tree->get_column(0), [=](Gtk::TreeRow &row) { return floatToString(row[InfoColumns::columns.fragmentation] / 1024.0, 2) + "KB"; });
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

static void setRendererText(Gtk::TreeViewColumn *column, bool colorByPid) {
    column->set_alignment(0.5);
    column->set_cell_data_func(*column->get_first_cell(), [=](Gtk::CellRenderer *cell, const Gtk::TreeModel::iterator &iter) {
        auto row = *iter;
        auto renderer_text = dynamic_cast<Gtk::CellRendererText *>(cell);
        if (!renderer_text) return;

        renderer_text->property_xalign() = 0.5;

        if (colorByPid) {
            const auto color = pidToColor(row[MMUColumns::columns.pid]);
            renderer_text->property_cell_background_rgba() = color;
            renderer_text->property_cell_background_set() = true;
            renderer_text->property_foreground() = "#000000";
            renderer_text->property_foreground_set() = true;
        } else {
            renderer_text->property_cell_background_set() = false;
            renderer_text->property_foreground_set() = false;
        }
    });
}

static void setRendererText(Gtk::TreeViewColumn *column, std::function<std::string(Gtk::TreeRow &)> transform, bool colorByPid) {
    column->set_alignment(0.5);
    column->set_cell_data_func(*column->get_first_cell(), [=](Gtk::CellRenderer *cell, const Gtk::TreeModel::iterator &iter) {
        auto row = *iter;
        auto renderer_text = dynamic_cast<Gtk::CellRendererText *>(cell);
        if (!renderer_text) return;

        renderer_text->property_text() = transform(row);
        renderer_text->property_xalign() = 0.5;

        if (colorByPid) {
            const auto color = pidToColor(row[MMUColumns::columns.pid]);
            renderer_text->property_cell_background_rgba() = color;
            renderer_text->property_cell_background_set() = true;
            renderer_text->property_foreground() = "#000000";
            renderer_text->property_foreground_set() = true;
        } else {
            renderer_text->property_cell_background_set() = false;
            renderer_text->property_foreground_set() = false;
        }
    });
}
