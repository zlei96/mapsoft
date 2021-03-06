#ifndef IO_XML_H
#define IO_XML_H

#include <vector>
#include <fstream>

#include "geo/geo_data.h"
#include "options/options.h"

namespace xml {

    void read_file (const char* filename, geo_data & world, const Options & opt = Options());

    bool write_map(std::ofstream & f, const g_map & m, const Options & opt);
    bool write_track(std::ofstream & f, const g_track & m, const Options & opt);
    bool write_waypoint_list(std::ofstream & f, const g_waypoint_list & m, const Options & opt);

    void write_file (const char* filename, const geo_data & world, const Options & opt = Options());

// Для всех типов точек - один map<string,string>, но разные преобразования
// Это все вынесено в отдельный h-файл, поскольку используется и при чтении точек из fig

// Переделать на встроенные преобразования в Options?

    struct xml_point : Options{
	operator g_waypoint () const;
	operator g_trackpoint () const;
	operator g_refpoint () const;
    };
    struct xml_point_list : Options {
	std::vector<xml_point> points;
	operator g_waypoint_list () const;
	operator g_track () const;
	operator g_map () const;
    };
    struct xml_map_list : Options {
	std::vector<g_map> maps;
	operator g_map_list () const;
    };
}
#endif
