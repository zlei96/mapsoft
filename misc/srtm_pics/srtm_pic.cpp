#include <stdexcept>
#include <iostream>
#include <fstream>
#include <cmath>
#include <cstdlib>

#include "srtm/srtm3.h"
#include "geo/geo_convs.h"
#include "geo_io/io_oe.h"
#include "2d/line_utils.h"
#include "2d/rainbow.h"
#include "err/err.h"

#define GETH geth4

// построение растровых горизонталей и выделение крутых уклонов цветами
// (СК Пулково, координаты Г-К)

// todo -- стандартные параметры --geom --lon0 ...

bool interp=false;

struct rainbow_data RD_podm[]={
  {5.0,  0xFFFFFF},
  {10.0, 0xFFFF00},
  {15.0, 0xFF0000},
  {20.0, 0xFF00FF},
  {25.0, 0x0000FF},
  {30.0, 0x404040}
};
int RDS_podm=sizeof(RD_podm)/sizeof(rainbow_data);

struct rainbow_data RD_hr[]={
  {30.0, 0xFFFFFF},
  {35.0, 0xFFFF00},
  {40.0, 0xFF0000},
  {45.0, 0xFF00FF},
  {50.0, 0x0000FF},
  {55.0, 0x404040}
};
int RDS_hr=sizeof(RD_hr)/sizeof(rainbow_data);


using namespace std;
int
main(int argc, char *argv[]) {
try{
    if (argc!=11) {
	std::cerr << "usage: " << argv[0] << " scale dpi lon0 X1 X2 Y1 Y2 step sstep style\n";
	exit(0);
    }
    // Зададим масштаб нужной нам карты, ее границы (в км Г-К), разрешение, осевой меридиан:
    double scale = atof(argv[1]);
    double dpi   = atof(argv[2]);
    double lon0  = atof(argv[3]);
    double X1    = atof(argv[4]);
    double X2    = atof(argv[5]);
    double Y1    = atof(argv[6]);
    double Y2    = atof(argv[7]);
    int step   = atoi(argv[8]);
    int sstep  = atoi(argv[9]);
    std::string style  = argv[10];

    const int grid_step = 1000;

// определим размер картинки
    if ((X1>X2)||(Y1>Y2)) throw Err() << "Range error!\n";

    double k = scale/2.54e-2*dpi;

    int W = (int)((X2-X1)*k);
    int H = (int)((Y2-Y1)*k);
    std::cerr << W << "x" << H << "\n";

    SRTM3 s("", 10);

    std::cout << "P6\n" << W << " " << H << "\n255\n";

    Options O;
    O.put<double>("lon0", lon0);
    convs::pt2wgs cnv(Datum("pulkovo"), Proj("tmerc"), O);

    g_map m;
    m.comm = "made by mapsoft_srtm_hor";
    m.file = "srtm_pic_out.pnm";
    m.map_proj = Proj("tmerc");

    dRect brd(0,0,W,H);
    m.border=rect2line(brd);

    brd = brd/k + dPoint(X1,Y1);

    for (int j = H; j > 0; j--){
      std::cerr << j << " ";
      for (int i = 0; i < W; i++){

        dPoint p_tmerc((double)i/k + X1, (double)j/k + Y1);
	dPoint p  = p_tmerc;
        cnv.frw(p);

        int h = s.GETH(p, interp);
        dPoint px = p_tmerc - dPoint(1/k, 0);
        cnv.frw(px);
        int hx=s.GETH(px, interp);

        dPoint py = p_tmerc - dPoint(0, 1/k);
        cnv.frw(py);
        int hy=s.GETH(py, interp);

        int c=0xFFFFFF;

        // g_map
        if (((i==0)||(i==W-1))&&((j==H)||(j==1))){
          m.push_back(g_refpoint(p.x, p.y, i, H-j));
        }

        // holes
        if ((h < srtm_min) || (hx < srtm_min) || (hy < srtm_min)){
          c=0xC8C8C8; goto print_colors;
	}

/*        // step contours
          if ((hx/step - h/step)||(hy/step - h/step)){
            c=0x808080; goto print_colors;
          }
*/
        // sstep contours
        if ((hx/sstep - h/sstep)||(hy/sstep - h/sstep)){
          c=0; goto print_colors;
        }

        { // grid
	  double d1 = ((int)(p_tmerc.x+1/k)/grid_step - (int)(p_tmerc.x)/grid_step);
	  double d2 = ((int)(p_tmerc.y+1/k)/grid_step - (int)(p_tmerc.y)/grid_step);
          if ((d1>0)||(d2>0)) {c=0x808080; goto print_colors; }
        }

        { // slopes
          if (style == "podm")
            c=get_rainbow(s.slope4(p), RD_podm, RDS_podm);
          else
            c=get_rainbow(s.slope4(p), RD_hr, RDS_hr);
        }

        print_colors:
	unsigned char c1,c2,c3;
        c1  = (c >> 16) & 0xFF;
        c2  = (c >> 8) & 0xFF;
        c3  = c & 0xFF;
	std::cout << c1 << c2 << c3;

      }
    }
    try {oe::write_map_file("srtm_pic_out.map", m);}
    catch (Err e) {cerr << e.get_error() << endl;}
}
catch (Err e) {
  cerr << e.get_error() << endl;
  return 1;
}
}

