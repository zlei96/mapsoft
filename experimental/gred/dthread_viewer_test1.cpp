#include <iostream>
#include "dthread_viewer.h"


int main(int argc, char **argv){

    Gtk::Main     kit (argc, argv);
    Gtk::Window   win;
    GPlaneTestTile      pl1;
    GPlaneTestTileSlow  pl2;

    DThreadViewer viewer(&pl2);
    viewer.set_fast_plane(&pl1);

    win.add(viewer);
    win.set_default_size(640,480);
    win.show_all();

    kit.run(win);
}