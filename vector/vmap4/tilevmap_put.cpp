#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "options/m_getopt.h"
#include "2d/line_utils.h"
#include "2d/line_rectcrop.h"
#include "tmap.h"


using namespace std;
void usage(){
  const char * prog = "tilevmap_put";
  cerr
     << prog << " -- put tiles to the tiled vmap.\n"
     << "  usage: " << prog << " [<input_options>] <in file> [<input_options>]\\\\n"
     << "         (--out|-o) <map dir> [<output_options>]\n"

  ;
  exit(1);
}

static struct option in_options[] = {
  {"autorange",      0, 0, 'a'},
  {"out",            0, 0, 'o'},
  {"verbose",        0, 0, 'v'},
  {0,0,0,0}
};



main(int argc, char **argv){

/// PARSE CMDLINE

  if (argc==1) usage();

  // options before filename
  Options OI = parse_options(&argc, &argv, in_options, "out");

  if (OI.exists("out") || (argc<1)){
    cerr << "no input file\n";
    exit(1);
  }

  const char * in_file = argv[0];

  // options after filename
  Options OI1 = parse_options(&argc, &argv, in_options, "out");

  if (!OI1.exists("out") || (argc<1)){
    cerr << "no output filename\n";
    exit(1);
  }

  const char * map_dir = argv[0];

  OI.insert(OI1.begin(), OI1.end());

  // options after map_dir

  Options OO = parse_options(&argc, &argv, in_options);


/// READ MAP INFO

  vmap::world V0;
  double tsize = read_tmap_data(V0, map_dir);

/// GET TILE RANGE

  iRect trange;

  int verbose = OI.get<int>("verbose",0);
  int autorange = OI.get<int>("autorange",0);

  vmap::world V = vmap::read(in_file);

  if (!autorange){
    double ntsize=0;
    istringstream sn(V.name);
    sn >> ntsize >> trange;

    if (trange.empty() || (ntsize==0)){
      cerr << "bad input file - can't get tsize or trange values\n";
      exit(1);
    }

    if (tsize!=ntsize){
      cerr << "wrong tsize: " << tsize << " != " <<  ntsize << "\n";
      exit(1);
    }
  }
  else { // add mode
    trange = rect_pump_to_int(V.range()/tsize);
  }
  if (verbose){
    cout << "reading from:      " << in_file << "\n"
         << "map tile size:     " << tsize << "\n"
         << "lonlat range:      " << trange*tsize << "\n"
         << "tile range:        " << trange << "\n"
         << "writing to:        " << map_dir << "\n"
    ;
  }

  split_labels(V);

  for (int j=0; j<trange.h; j++){
    for (int i=0; i<trange.w; i++){

      dRect crop_range = (trange.TLC() + dRect(i,j,1,1))*tsize;
      vmap::world V1(V0);
      V1.brd=rect2line(crop_range);

      /// crop objects
      for (vmap::world::const_iterator o=V.begin(); o!=V.end(); o++){
        bool closed = (o->get_class() == vmap::POLYGON);

        vmap::object o1 = *o;
        dMultiLine::iterator l;
        for (l = o1.begin(); l != o1.end(); l++){
          rect_crop(crop_range, *l, closed);
/*
          if (!closed){
            dMultiLine ml = rect_split_cropped(crop_range, *l);
            for (dMultiLine::const_iterator m=ml.begin(); m!=ml.end(); m++){
              l = o1.insert(l, *m) + 1;
            }
            l->clear();
          }
*/
        }
        // remove empty lines
        l = o1.begin();
        while (l != o1.end()){
          if (l->size()==0) l=o1.erase(l);
          else l++;
        }
        if (o1.size()) V1.push_back(o1);
      }

      /// crop lbuf
      std::list<vmap::lpos_full>::iterator lb;
      for (lb=V.lbuf.begin(); lb!=V.lbuf.end(); lb++){
        if (point_in_rect(lb->ref, crop_range)) V1.lbuf.push_back(*lb);
      }

      // write tile
      if (V1.size()){
        ostringstream ss;
        ss << map_dir << "/" << "t:" << i+trange.x
           << ":" << j+trange.y << ".vmap";
        string fname = ss.str();
       vmap::write(fname.c_str(), V1, OO);
      }
    }
  }
  exit(0);
}