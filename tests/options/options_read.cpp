#include <iostream>
#include <fstream>
#include <options/options.h>

using namespace std;



main(int argc, char **argv){
  if (argc==1) cerr << "usage: " << argv[0] << " <option files>\n";

  Options O;

  for (int i=1; i<argc; i++){
    ifstream s(argv[i]);
    s >> O;
  }
  cout << O;
}