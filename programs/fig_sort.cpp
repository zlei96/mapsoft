#include <fstream>
#include "../libfig/fig_utils.h"
#include "../libfig/fig_io.h"

// ���������� fig-����� (��� �������� �������� � git, ��������� diff'a � �.�.)

main(int argc, char **argv){
  if (argc<2) {
      std::cerr << "usage: " << argv[0] << " <fig>\n";
      exit(0);
  }
  fig::fig_world W;

  if (! fig::read(argv[1], W)){
    std::cerr << "Read error. File was not modified.\n";
    exit(1);
  }

  fig::fig_world::iterator i=W.begin();
  while (i!=W.end()){
    if ((i->type==6) || (i->type==-6)) i=W.erase(i);
    else i++;
  }

  W.sort();

  std::ofstream f(argv[1]);
  fig::write(f, W);
}