#include "layer_geomap.h"
#include <sstream>
#include <fstream>
#include <cmath>

#include "loaders/image_r.h"
#include "loaders/image_i.h"
#include "utils/cairo_wrapper.h"

using namespace std;

LayerGeoMap::LayerGeoMap(const geo_data *_world, bool _drawborder) :
      world(_world),
      image_cache(4),
      mymap(convs::mymap(*_world)),
      drawborder(_drawborder){
  make_m2ms();
}

g_map
LayerGeoMap::get_ref() const {
  return mymap;
}

void
LayerGeoMap::set_ref(const g_map & map){
  mymap=map;
  make_m2ms();
}

void
LayerGeoMap::set_ref(){
  set_ref(convs::mymap(*world));
}

const geo_data *
LayerGeoMap::get_world() const{
  return world;
}

void
LayerGeoMap::refresh(){
  image_cache.clear();
}

iImage
LayerGeoMap::get_image (iRect src){
  if (rect_intersect(myrange, src).empty()) return iImage(0,0);
  iImage ret(src.w, src.h, 0);
  draw(src.TLC(), ret);
  return ret;
}

void
LayerGeoMap::draw(const iPoint origin, iImage & image){
  iRect src_rect = image.range() + origin;

#ifdef DEBUG_LAYER_GEOMAP
  cerr  << "LayerMap: draw " << src_rect << " my: " << myrange << endl;
#endif
  if (rect_intersect(myrange, src_rect).empty()) return;
  if ((world == NULL)||(world->maps.size()==0)) return;
  CairoWrapper cr(image);
  cr->set_line_width(1);
  cr->set_color(0x0000FF);

  for (int i=0; i<world->maps.size(); i++){
    string file = world->maps[i].file;
    if (!m2ms[i].tst_frw.test_range(src_rect)) continue;

    int scale = int(scales[i]+0.05);
    if (scale <=0) scale = 1;

    if (scale<=32){
      if (!image_cache.contains(i) || (iscales[i] > scale)) {
#ifdef DEBUG_LAYER_GEOMAP
        cerr  << "LayerMap: Loading Image " << file
	           << " at scale " << scale << "\n";
#endif
        image_cache.add(i, image_r::load(file.c_str(), scale));
        iscales[i] = scale;
      }
#ifdef DEBUG_LAYER_GEOMAP
      cerr  << "LayerMap: Using Image " << file
		 << " at scale " << scale << " (loaded at scale " << iscales[i] <<", scales[i]: " << scales[i] << ")\n";
#endif
      iImage im = image_cache.get(i);
      m2ms[i].image_frw(im, iscales[i], src_rect, image, image.range());
    }

    if (drawborder){
      for (dLine::iterator p=m2ms[i].border_dst.begin();
                           p!=m2ms[i].border_dst.end(); p++){
        if (p==m2ms[i].border_dst.begin())
          cr->move_to((*p)-dPoint(src_rect.TLC()));
        else
          cr->line_to((*p)-dPoint(src_rect.TLC()));
      }
      cr->close_path();
    }
  }
  cr->stroke();
}

iRect
LayerGeoMap::range() const{
  return myrange;
}

void
LayerGeoMap::dump_maps(const char *file){
  ofstream f(file);
  f<< "#FIG 3.2\n"
   << "Portrait\n"
   << "Center\n"
   << "Metric\n"
   << "A4\n"
   << "100.0\n"
   << "Single\n"
   << "-2\n"
   << "1200 2\n";
  for (int i=0;i<m2ms.size();i++){
    int bs = m2ms[i].border_dst.size();
    f << "2 3 0 1 4 29 8 -1 20 0.000 0 0 -1 0 0 "
      << bs << "\n\t";
    double minx=1e99, maxx=-1e99;
	for (int j=0; j<bs; j++){
      double x=m2ms[i].border_dst[j].x;
      double y=m2ms[i].border_dst[j].y;
      if (x<minx) minx=x;
      if (x>maxx) maxx=x;
      f << " " << int(x) << " " << int(y);
    }
    f << "\n";
    if (bs==0) continue;
    double lettw=190;
    double letth=400;
    string s1;
    int n=0, l=0;
    while (n<world->maps[i].comm.size()>0){
      s1+=world->maps[i].comm[n];
      n++;
      if ((n==world->maps[i].comm.size()) || (s1.size()*lettw > maxx-minx)){
      f << "4 0 4 6 -1 18 20 0.0000 4 225 630 " 
        << int(m2ms[i].border_dst[0].x+100) << " " 
        << int(m2ms[i].border_dst[0].y+500 + l*letth) << " " 
        << s1 << "\\001\n";
        s1=""; l++;
      }
    }
  }
  f.close();
}

void
LayerGeoMap::make_m2ms(){
  if ((world == NULL)||(world->maps.size()==0)) return;

  m2ms.clear();
  scales.clear();

  for (int i=0; i< world->maps.size(); i++){
    convs::map2map c(world->maps[i], mymap);
    m2ms.push_back(c);

    dPoint p1(0,0), p2(1000,0), p3(0,1000);
    c.frw(p1); c.frw(p2); c.frw(p3);
    double sc_x = 1000/pdist(p1,p2);
    double sc_y = 1000/pdist(p1,p3);

    scales.push_back(sc_x<sc_y ? sc_x:sc_y); // каков масштаб карты в соотв.с проекцией

    if ((i==0) && (c.border_dst.size()!=0)){
      myrange=iRect(c.border_dst[0], c.border_dst[0]);
    }
    for (int j=0; j<c.border_dst.size(); j++){
      myrange = rect_pump(myrange, iPoint(c.border_dst[j]));
    }
  }
  // старые данные нам тоже интересны (мы можем использовать уже загруженные картинки)
  if (iscales.size() != world->maps.size())
    iscales.resize(world->maps.size(),1);
#ifdef DEBUG_LAYER_GEOMAP
  cerr << "LayerMap: Setting map conversions. Range: " << myrange << "\n";
#endif
}