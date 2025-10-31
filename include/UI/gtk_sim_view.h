#ifndef GTK_SIM_VIEW_H
#define GTK_SIM_VIEW_H

#include <gtkmm/builder.h>
#include <gtkmm/liststore.h>
#include <gtkmm/treeview.h>
#include <gtkmm/scale.h>
#include <gtkmm/togglebutton.h>
#include <gtkmm/drawingarea.h>
#include <gdkmm/rgba.h>
#include <cairomm/context.h>
#include <glibmm/ustring.h>
#include <vector>

typedef enum class PageAction {
    createPage,
    modifyPage,
    deletePage
} PageActionE;

typedef struct MMUModel {
    guint id;
    guint pid;
    bool loaded;
    guint lAddr;
    guint mAddr;
    guint dAddr;
    guint loadedTime;
    bool mark;
} MMUModel;

typedef struct InfoModel {
    guint processes;
    guint time;
    guint ram;
    guint vRam;
    guint loadedPages;
    guint unloadedPages;
    guint thrashing;
    guint fragmentation;
} InfoModel;

class GtkSimView {
private:
    Glib::RefPtr<Gtk::Builder> mBuilder;
    
    Gtk::Scale *mRalenScale;
    Gtk::TreeView *mAlgMmu;
    Gtk::TreeView *mOptMmu;
    Gtk::TreeView *mAlgMain;
    Gtk::TreeView *mOptMain;
    Gtk::TreeView *mAlgRam;
    Gtk::TreeView *mOptRam;
    Gtk::DrawingArea *mRamAlgBar;
    Gtk::DrawingArea *mRamOptBar;
    Gtk::TreeView *mAlgPages;
    Gtk::TreeView *mOptPages;
    Gtk::TreeView *mAlgThrashing;
    Gtk::TreeView *mOptThrashing;
    Gtk::TreeView *mAlgFragmentation;
    Gtk::TreeView *mOptFragmentation;

    std::vector<Gdk::RGBA> mRamOptColors;
    std::vector<Gdk::RGBA> mRamAlgColors;
    std::vector<Glib::ustring> mRamOptLabels;
    std::vector<Glib::ustring> mRamAlgLabels;

    Gtk::Button *mReset;
    Gtk::ToggleButton *mPlay;
    Gtk::ToggleButton *mPause;
    
    void setMMU(Glib::RefPtr<Gtk::ListStore> model, const std::vector<MMUModel> mmu) const;
    void setInfo(Glib::RefPtr<Gtk::ListStore> model, const InfoModel info) const;
    void updateRamBar(Gtk::DrawingArea *area,
                      std::vector<Gdk::RGBA> &colors,
                      std::vector<Glib::ustring> &labels,
                      const std::vector<MMUModel> &mmu) const;
    bool onRamDraw(const Cairo::RefPtr<Cairo::Context> &cr,
                   const std::vector<Gdk::RGBA> &colors,
                   const std::vector<Glib::ustring> &labels,
                   Gtk::DrawingArea *area) const;
    
public:
    GtkSimView();
    ~GtkSimView();
    
    void setBuilder(Glib::RefPtr<Gtk::Builder> builder) { mBuilder = builder; }
    void setOptMMU(const std::vector<MMUModel> mmu) const;
    void setAlgMMU(const std::vector<MMUModel> mmu) const;
    void setOptInfo(const InfoModel info) const { setInfo(Glib::RefPtr<Gtk::ListStore>::cast_dynamic(mOptMain->get_model()), info); }
    void setAlgInfo(const InfoModel info) const { setInfo(Glib::RefPtr<Gtk::ListStore>::cast_dynamic(mAlgMain->get_model()), info); }

    void ralentizationConnect(std::function<void(unsigned int)> func) const;
    void playConnect(std::function<void()> func) const;
    void pauseConnect(std::function<void()> func) const;
    void resetConnect(std::function<void()> func) const;

    void initialize();
    void showState() const;
    void resetState() const;

};

#endif // GTK_SIM_VIEW_H
