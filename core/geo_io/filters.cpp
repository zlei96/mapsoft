#include "io.h"
#include "geo/geo_convs.h"
#include "geo/geo_nom.h"
#include "2d/line_utils.h"

#include <boost/spirit/include/classic_core.hpp>
#include <boost/spirit/include/classic_assign_actor.hpp>


namespace io{

using namespace std;

// set map border from its nomenclature name
void map_nom_brd(geo_data & world, const Options & O){
  if (O.exists("verbose")) cerr << "map_nom_brd filter" << endl;
  vector<g_map_list>::iterator ml_i;
  vector<g_map>::iterator i;
  for (ml_i = world.maps.begin(); ml_i!=world.maps.end(); ml_i++){
    for (i = ml_i->begin(); i!=ml_i->end(); i++){

      dRect r = convs::nom_to_range(i->comm);
      if (r.empty()) r=convs::nom_to_range(i->file);

      if (O.exists("verbose")){
        if (!r.empty()) cerr << " - setting border for the nom map " << i->comm << endl;
        else cerr << " - skipping non-nomenclature map " << i->comm << endl;
      }

      if (r.empty()) return;
      double lon1 = r.x;
      double lat1 = r.y;
      double lon2 = lon1 + r.w;
      double lat2 = lat1 + r.h;

      convs::map2pt conv(*i, Datum("pulk42"), Proj("lonlat"));

      dLine brd;
      brd.push_back(dPoint(lon1,lat2));
      brd.push_back(dPoint(lon2,lat2));
      brd.push_back(dPoint(lon2,lat1));
      brd.push_back(dPoint(lon1,lat1));
      brd.push_back(dPoint(lon1,lat2));
      i->border = conv.line_bck(brd);
    }
  }
}

// set map border from its reference points
void map_ref_brd(geo_data & world, const Options & O){
  if (O.exists("verbose")) cerr << "map_ref_brd filter" << endl;
  vector<g_map_list>::iterator ml_i;
  vector<g_map>::iterator i;
  for (ml_i = world.maps.begin(); ml_i!=world.maps.end(); ml_i++){
    for (i = ml_i->begin(); i!=ml_i->end(); i++){
      g_map::const_iterator p;
      dLine points;
      for (p=i->begin(); p!=i->end(); p++)
        points.push_back(dPoint(p->xr, p->yr));
      i->border = convex_border(points);
    }
  }
}


void
generalize(g_track * line, double e, int np, const Options & O){
  // какие точки мы хотим исключить:
  std::vector<bool> skip(line->size(), false);

  Options o;
  o.put("lon0", convs::lon2lon0((*line)[0].x));
  convs::pt2wgs cnv(Datum("wgs84"), Proj("tmerc"), o);

  dLine gk_line=(dLine)(*line);
  cnv.line_bck_p2p(gk_line);

  if (O.exists("verbose"))
    cerr << "generalize track: <" << line->comm <<  ">,  points: " << line->size();

  np-=2; // end points are not counted
  double err = 0.0;
  while (1){
    // для каждой точки найдем расстояние от нее до
    // прямой, соединяющей две соседние (не пропущенные) точки.
    // найдем минимум этой величины
    double min=-1;
    int mini; // index of point with minimal deviation
    int n=0;
    for (int i=1; i<int(gk_line.size())-1; i++){
      if (skip[i]) continue;
      n++; // count point we doesn't plan to skip
      int ip, in; // previous and next indexes
      // skip[0] and skip[line.size()-1] are always false
      for (ip=i-1; ip>=0; ip--)             if (!skip[ip]) break;
      for (in=i+1; in<gk_line.size(); in++) if (!skip[in]) break;
      Point<double> p1 = gk_line[ip];
      Point<double> p2 = gk_line[i];
      Point<double> p3 = gk_line[in];
      double ll = pdist(p3-p1);
      dPoint v = (p3-p1)/ll;
      double prj = pscal(v, p2-p1);
      double dp;
      if      (prj<=0)  dp = pdist(p2,p1);
      else if (prj>=ll) dp = pdist(p2,p3);
      else              dp = pdist(p2-p1-v*prj);
      if ((min<0) || (min>dp)) {min = dp; mini=i;}
    }
    if (n==0) break;
    // если этот минимум меньше e или точек в линии больше np - выкинем точку
    if ( ((e>0) && (min<e)) ||
         ((np>0) && (n>np))) { skip[mini]=true; err=min; }
    else break;
  }
  g_track::iterator i = line->begin();
  int j=0;
  while (i != line->end()){
    if (skip[j]) i = line->erase(i);
    else i++;
    j++;
  }
  if (O.exists("verbose")) cerr << " -> " << line->size() << " error: " << err << endl;

}

void
skip(geo_data & world, const string & sk, const Options & O){
    if (sk == "") return;
    if (O.exists("verbose")) cerr << "skip data filter: " << sk << endl;
    bool m = (sk.find("m")!=string::npos) || (sk.find("M")!=string::npos);
    bool w = (sk.find("w")!=string::npos) || (sk.find("W")!=string::npos);
    bool t = (sk.find("t")!=string::npos) || (sk.find("T")!=string::npos);
    bool o = (sk.find("o")!=string::npos) || (sk.find("O")!=string::npos);
    bool a = (sk.find("a")!=string::npos) || (sk.find("A")!=string::npos);

    if (m) world.maps.clear();
    if (w) world.wpts.clear();

    if (t) world.trks.clear();
    else if (o || a){
      vector<g_track>::iterator t=world.trks.begin();
      while (t!=world.trks.end()){
        if      (o && (t->comm != "ACTIVE LOG")) t=world.trks.erase(t);
        else if (a && (t->comm == "ACTIVE LOG")) t=world.trks.erase(t);
        else t++;
      }
    }
  }


void
filter(geo_data & world, const Options & opt){

  if (opt.exists("shift_maps")){
    dPoint shift_maps = opt.get("shift_maps", dPoint(0,0));
    for (vector<g_map_list>::iterator ml=world.maps.begin(); ml!=world.maps.end(); ml++){
      for (vector<g_map>::iterator m=ml->begin(); m!=ml->end(); m++){
        *m+=shift_maps;
      }
    }
  }

  if (opt.exists("rescale_maps")){
    double rescale_maps = opt.get("rescale_maps", 1.0);
    for (vector<g_map_list>::iterator ml=world.maps.begin(); ml!=world.maps.end(); ml++){
      for (vector<g_map>::iterator m=ml->begin(); m!=ml->end(); m++){
        *m*=rescale_maps;
      }
    }
  }

  if (opt.exists("map_nom_brd")){
    io::map_nom_brd(world, opt);
  }

  if (opt.exists("map_ref_brd")){
    io::map_ref_brd(world, opt);
  }

  if (opt.exists("skip")){
    skip(world, opt.get<string>("skip"), opt);
  }

  if (opt.exists("gen_n") || opt.exists("gen_e")){
    int n =  opt.get<int>("gen_n", 0);
    int e =  opt.get<double>("gen_e", 0.0);
    for (vector<g_track>::iterator tr=world.trks.begin(); tr!=world.trks.end(); tr++){
      generalize(&(*tr), e, n, opt);
    }
  }

  //join maps:
  if (opt.exists("join_maps") && world.maps.size()>1){
    vector<g_map_list>::iterator l0 = world.maps.begin(), l=l0+1;
    while (l!=world.maps.end()){
      l0->insert(l0->begin(), l->begin(), l->end());
      l=world.maps.erase(l);
    }
  }

  //join wpts:
  if (opt.exists("join_wpts") && world.wpts.size()>1){
    vector<g_waypoint_list>::iterator l0 = world.wpts.begin(), l=l0+1;
    while (l!=world.wpts.end()){
      l0->insert(l0->begin(), l->begin(), l->end());
      l=world.wpts.erase(l);
    }
  }

  //join trks:
  if (opt.exists("join_trks") && world.trks.size()>1){
    vector<g_track>::iterator l0 = world.trks.begin(), l=l0+1;
    while (l!=world.trks.end()){
      l0->insert(l0->begin(), l->begin(), l->end());
      l=world.trks.erase(l);
    }
  }

}



}//namespace
