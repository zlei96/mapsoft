#include <iostream>
#include "dthread_viewer.h"


int main(int argc, char **argv){

    Gtk::Main     kit (argc, argv);
    Gtk::Window   win;
    GPlaneTestGridSlow  pl;

    DThreadViewer viewer(&pl);

    win.add(viewer);
    win.set_default_size(640,480);
    win.show_all();

    kit.run(win);
}