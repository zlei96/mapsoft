#ifndef AM_JOIN_VIS_TRK
#define AM_JOIN_VIS_TRK

#include "action_mode.h"

// join visible waypoints action

class JoinVisTrk : public ActionMode{
public:
    JoinVisTrk (Mapview * mapview) :  ActionMode(mapview) { }

    std::string get_name() { return "Join visible tracks"; }
    bool is_radio() { return false; }

    void activate() {
      Gtk::TreeNodeChildren::const_iterator i; 
      boost::shared_ptr<g_track> newd(new g_track);
      i = mapview->trk_ll.store->children().begin();
      while (i != mapview->trk_ll.store->children().end()){
        if (!(*i)[mapview->trk_ll.columns.checked]) {
          i++;
          continue;
        }
        boost::shared_ptr<LayerTRK> current_layer =
          (*i)[mapview->trk_ll.columns.layer];
        g_track * curr = current_layer->get_data();
        if (!curr){
          i++;
          continue;
        }
        newd->insert(newd->end(), curr->begin(), curr->end());
        if (newd->size()) newd->comm = "JOIN";
        else newd->comm = curr->comm;
        mapview->workplane.remove_layer(
          i->get_value(mapview->trk_ll.columns.layer).get());
        i = mapview->trk_ll.store->erase(i);
      }
      if (newd->size()) mapview->add_trks(newd);
    }
};

#endif /* AM_ADD_FILE */