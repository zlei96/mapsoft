#ifndef MAP_TRACER_H
#define MAP_TRACER_H

#include <vector>
#include <string>

#include "srtm3.h"
#include <2d/point.h>

// построение хребтовки-речевки

struct map_pt{ // точка на карте
  short  alt;        //altitude
  char   rdir, mdir; //directions 0-7, 
                     //-1 (неизвестное направление), 8 (бессточная точка)
  double rarea, marea;  //areas
                     // m- и r- -- для речной и хребтовой сети
  map_pt(){
    alt = srtm_undef;
    rdir=-1; mdir=-1;
    rarea=0; marea=0;
  }
};

struct map_tracer{
  std::vector<map_pt> data; // точки 
  int lat1,lat2,lon1,lon2;  // координаты углов в 3-х секундах
  int w;
  int h;
  map_pt p0; // точка по умолчанию.

  map_tracer(int Lat1,int Lon1, int Lat2, int Lon2,
             const std::string & dir=def_srtm_dir);

  map_pt* pt(int lat, int lon);
  map_pt* pt(iPoint p){ return pt(p.y, p.x);}

  short   geth(int lat, int lon);
  short   geth(iPoint p){ return geth(p.y, p.x);}


  void trace(iPoint p, bool up=false, int rmax=1000);
  void set_dirs(int rmax=1000, int mmax=1000);
  void set_areas(void);

};
#endif