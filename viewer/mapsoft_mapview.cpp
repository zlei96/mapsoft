#include <gtkmm.h>

//#include <boost/shared_ptr.hpp>

//#include <io_new/io.h>
//#define DEBUG_VIEWER
//#define DEBUG_GOOGLE
//#define DEBUG_LAYER_GEOMAP
//#define DEBUG_LAYER_GEODATA
//#define DEBUG_JPEG
//#define DEBUG_MAPVIEW

#include "viewer.h"
#include "mapview.h"

#include "../core/libgeo_io/io.h"

int
main(int argc, char **argv)
{
    Gtk::Main kit (argc, argv);

    Mapview mapview;
    
    for (int i = 1; i < argc; ++i) {
	mapview.load_file(argv[i]);
    }
    kit.run(mapview);
}
