#include "vmap.h"

main(int argc, char* argv[]){
  if (argc<2) {
    std::cerr << "Usage: test_dir2mp <in_dir1> ... <out_mp>\n";
    exit(1);
  }

  vmap::world w;

  for (int i=1; i<argc-1; i++){
    std::cerr << "Reading " << argv[i] << "\n";
    if (!vmap::read_dir(argv[i], w)){
      std::cerr << "Read error.\n";
      exit(1);
    }
  }

  vmap::write_mp(argv[argc-1], vmap::legend("./mmb.cnf"), w);
}