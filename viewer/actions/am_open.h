#ifndef AM_OPEN
#define AM_OPEN

#include "action_mode.h"

class Open : public ActionMode, public Gtk::FileSelection{
public:
    Open (Mapview * mapview) :
           ActionMode(mapview),
           Gtk::FileSelection("Open"),
           warn_dlg("Only mapview xml files can be opened. Use Add/Import to"
                    " load other geodata formats.", false,
                    Gtk::MESSAGE_WARNING, Gtk::BUTTONS_CLOSE){

      warn_dlg.signal_response().connect(
        sigc::hide(sigc::mem_fun(warn_dlg, &Gtk::MessageDialog::hide_all)));

      Glib::RefPtr<Gtk::FileFilter> filter(new Gtk::FileFilter);
      filter->add_pattern("*.xml");
//      set_filter(filter);

      get_ok_button()->signal_clicked().connect(
          sigc::mem_fun (this, &Open::on_ok));
      get_cancel_button()->signal_clicked().connect(
          sigc::mem_fun(this, &Gtk::Window::hide));
    }

    std::string get_name() { return "Open"; }
    Gtk::StockID get_stockid() { return Gtk::Stock::OPEN; }
    Gtk::AccelKey get_acckey() { return Gtk::AccelKey("<control>o"); }

    bool is_radio() { return false; }

    void activate() { show(); }

    void on_ok(){
      std::string f = get_filename();
      if (!io::testext(f, ".xml")){
        warn_dlg.show_all();
        return;
      }
      mapview->load_file(f);
      hide();
    }
    Gtk::MessageDialog warn_dlg;
};

#endif