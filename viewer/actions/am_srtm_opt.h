#ifndef AM_SRTM_OPT_H
#define AM_SRTM_OPT_H

#include "action_mode.h"
#include "../dialogs/srtm_opt.h"

class SrtmOpt : public ActionMode {
public:
    SrtmOpt (Mapview * mapview) : ActionMode(mapview) {

      dlg.signal_response().connect(
        sigc::mem_fun (this, &SrtmOpt::on_response));
      dlg.signal_changed().connect(
        sigc::bind(sigc::mem_fun (this, &SrtmOpt::on_response),1));

      dlg.set_title(get_name());
      dlg.set_opt(mapview->layer_options);
    }

    std::string get_name() { return "SRTM layer"; }
//    Gtk::StockID get_stockid() { return Gtk::StockID(""); }

    bool is_radio() { return false; }

    void activate() {
      if (!mapview->have_reference){
        mapview->statusbar.push("No geo-reference", 0);
        return;
      }
      dlg.set_opt(mapview->layer_srtm.get_opt());
      dlg.show_all();
      mapview->show_srtm();
    }

    void on_response(int r){

      if (!mapview->have_reference){
        mapview->statusbar.push("No geo-reference", 0);
        return;
      }

      if (r<0){
        if (r==Gtk::RESPONSE_CANCEL) mapview->show_srtm(false);
        dlg.hide_all();
      }
      else if (mapview->workplane.exists(&mapview->layer_srtm)){
         mapview->layer_srtm.set_opt(dlg.get_opt());
         mapview->workplane.refresh_layer(&mapview->layer_srtm);
      }

    }

private:
    DlgSrtmOpt dlg;
};

#endif