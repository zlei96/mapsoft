#include <string>
#include <cstring>
#include "options/m_getopt.h"
#include "vmap.h"
#include "libfig/fig.h"

using namespace std;

bool testext(const string & nstr, const char *ext){
    int pos = nstr.rfind(ext);
    return ((pos>0)&&(pos == nstr.length()-strlen(ext)));
}

void usage(){
  const char * prog = "vmap_copy";

  cerr
     << prog << " -- convert vector maps in mp or fig formats.\n"
     << "  usage: " << prog << " [<global_input_options>]\\\n"
     << "         <input_file1> [<input_options1>] ... \\\n"
     << "         (--out|-o) <output_file> [<output_options>]\n"
     << "\n"
     << "  input options:\n"
     << "\n"
     << "    --skip_labels               -- don't read labels\n"
     << "    --read_labels               -- do read labels\n"
     << "\n"
     << "    --set_source <string>       -- set source parameter\n"
     << "    --set_source_from_name      -- set source parameter from map name\n"
     << "    --set_source_from_fname     -- set source parameter from file name\n"
     << "    --select_source <string>    -- copy only objects with given source\n"
     << "    --skip_source <string>      -- copy all objects but ones with given source\n"
     << "    (select_source and skip_source options are processed before set_source_*)\n"
     << "\n"
     << "    --select_type <int value>   -- copy only objects with given type\n"
     << "    --skip_typee <int value>    -- copy all objects but ones with given type\n"
     << "    --skip_all                  -- don't read any objects, only labels\n"
     << "\n"
     << "    --skip_range <geometry>     -- skip objects touching range\n"
     << "    --select_range <geometry>   -- select objects touching range\n"
     << "    --crop_range <geometry>     -- crop objects to the range\n"
     << "    --skip_nom <string>         -- \n"
     << "    --select_nom <string>       -- \n"
     << "    --crop_nom <string>         -- \n"
     << "    --range_datum <string>      -- set datum for range settings (wgs84, pulkovo)\n"
     << "    --range_proj <string>       -- set proj for range settings (lonlat, tmerc)\n"
     << "    --range_lon0 <double>       -- set lon0tmerc proj in range settings\n"
     << "    (lon0 can be overriden by prefix in range x coord)\n"
     << "\n"
     << "    -v, --verbose               -- be verbose\n"
     << "\n"
     << "  output options:\n"
     << "\n"
     << "    -a, --append                -- don't remove map from output file\n"
     << "    -n, --name <string>         -- set map name\n"
     << "    -i, --mp_id <int>           -- set mp id\n"
     << "    -m, --rscale <double>       -- set reversed scale (50000 for 1:50000 map)\n"
     << "    -s, --style <string>        -- set map style\n"
     << "\n"
     << "    --skip_labels               -- don't write labels\n"
     << "\n"
     << "    --set_source <string>       -- set source parameter\n"
     << "    --set_source_from_name      -- set source parameter from map name\n"
     << "    --set_source_from_fname     -- set source parameter from file name\n"
     << "    --select_source <string>    -- copy only objects with given source\n"
     << "    --skip_source <string>      -- copy all objects but ones with given source\n"
     << "    (select_source and skip_source options are processed before set_source_*)\n"
     << "\n"
     << "    --select_type <int>         -- copy only objects with given type\n"
     << "    --skip_typee <int>          -- copy all objects but ones with given type\n"
     << "    --skip_all                  -- don't read any objects, only labels\n"
  ;
  exit(1);
}

static struct option in_options[] = {
  {"skip_labels",           0, 0, 0},
  {"read_labels",           0, 0, 0},

  {"set_source",            1, 0, 0},
  {"set_source_from_name",  0, 0, 0},
  {"set_source_from_fname", 0, 0, 0},
  {"select_source",         1, 0, 0},
  {"skip_source",           1, 0, 0},
  {"select_type",           1, 0, 0},
  {"skip_type",             1, 0, 0},
  {"skip_all",              0, 0, 0},

  {"skip_range",            1, 0, 0},
  {"select_range",          1, 0, 0},
  {"crop_range",            1, 0, 0},
  {"skip_nom",              1, 0, 0},
  {"select_nom",            1, 0, 0},
  {"crop_nom",              1, 0, 0},
  {"range_datum",           1, 0, 0},
  {"range_proj",            1, 0, 0},
  {"range_lon0",            1, 0, 0},

  {"out",         0, 0 , 'o'},
  {"verbose",     0, 0 , 'v'},
  {0,0,0,0}
};

static struct option out_options[] = {
  {"skip_labels",           0, 0, 0},

  {"set_source",            1, 0 , 0},
  {"set_source_from_name",  0, 0 , 0},
  {"set_source_from_fname", 0, 0 , 0},
  {"select_source",         1, 0 , 0},
  {"skip_source",           1, 0 , 0},
  {"select_type",           1, 0 , 0},
  {"skip_type",             1, 0 , 0},
  {"skip_all",              0, 0 , 0},

  {"append",      0, 0 , 'a'},
  {"name",        1, 0 , 'n'},
  {"mp_id",       1, 0 , 'i'},
  {"rscale",      1, 0 , 'm'},
  {"style",       1, 0 , 's'},
  {0,0,0,0}
};


main(int argc, char **argv){

  if (argc==1) usage();

  Options O;
  Options GO = parse_options(argc, argv, in_options, "out"); // read global options
  argc-=optind;
  argv+=optind;
  optind=0;

  if (GO.exists("out")){
      std::cerr << "error: no input files\n";
      exit(1);
  }
  vmap::world V;

  do {
    if (argc<1){
      std::cerr << "no output files\n";
      exit(1);
    }
    const char * ifile = argv[0];

    // parse options for this file and append global options
    O = parse_options(argc, argv, in_options, "out");
    argc-=optind;
    argv+=optind;
    optind=0;
    O.insert(GO.begin(), GO.end());

    if (O.exists("set_source_from_fname"))
      O.put<string>("set_source", ifile);

    if (O.get<int>("verbose",0))
      cout << "reading: " << ifile  << "\n";

    vmap::world V1 = vmap::read(ifile);
    filter(V1, O);
    V.add(V1);

  }
  while (!O.exists("out"));

  /***************** output ****************/

  if (argc<1){
    std::cerr << "no output files\n";
    exit(1);
  }
  const char * ofile = argv[0];

  // parse output options
  O = parse_options(argc, argv, out_options);
  argc-=optind;
  argv+=optind;
  optind=0;

  /***************** write file ****************/

  if (O.exists("set_source_from_fname"))
    O.put<string>("set_source", ofile);

  if (GO.get<int>("verbose",0))
    cout << "writing to: " << ofile << "\n";

  // find labels for each object
  add_labels(V);
  // create new labels
  new_labels(V);

  filter(V, O);

  if (!vmap::write(ofile, V, O)) exit(1);

  exit(0);
}

