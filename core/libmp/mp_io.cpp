#include <boost/spirit/actor/assign_actor.hpp>
#include <boost/spirit/actor/push_back_actor.hpp>
#include <boost/spirit/actor/insert_at_actor.hpp>
#include <boost/spirit/actor/clear_actor.hpp>
#include <boost/spirit/actor/erase_actor.hpp>

#include <iomanip>

#include "mp_io.h"
#include "../utils/spirit_utils.h"
#include "../utils/iconv_utils.h"
#include "../lib2d/point_utils.h"

namespace mp {

using namespace std;
using namespace boost::spirit;

bool read(const char* filename, mp_world & world, const Options & opts){

  mp_world ret;
  mp_object o, o0;
  int l=0;
  Point<double> pt;

  Line<double>   line;
  string         comm, key, val;

  rule_t key_ch = anychar_p - eol_p - '=';
  rule_t ch = anychar_p - eol_p;

  rule_t comment = ch_p(';') >> (*ch)[assign_a(comm)] >> eol_p;
  rule_t option  = (*key_ch)[assign_a(key)] >> '=' >> (*ch)[assign_a(val)] >> eol_p;

  rule_t header =
      *(comment[push_back_a(ret.Comment, comm)] | space_p)  >>
      ("[IMG ID]") >> eol_p >> *(
      ( "ID="              >> !uint_p[assign_a(ret.ID)]            >> eol_p) |
      ( "Name="            >> (*ch)[assign_a(ret.Name)]            >> eol_p) |
      ( option[erase_a(ret.Opts, key)][insert_at_a(ret.Opts, key, val)] )
    ) >> "[END-IMG ID]" >> eol_p;

    rule_t pt_r = ch_p('(') 
		  >> real_p[assign_a(pt.y)] >> ',' 
		  >> real_p[assign_a(pt.x)]
                  >> ch_p(')');

    rule_t object =
      eps_p[assign_a(o,o0)] >> *(comment[push_back_a(o.Comment, comm)] | space_p) >>
      ch_p('[') >>
       ((str_p("POI") | "RGN10" | "RGN20")[assign_a(o.Class, "POI")] |
        (str_p("POLYLINE") | "RGN40")[assign_a(o.Class, "POLYLINE")] |
        (str_p("POLYGON")  | "RGN80")[assign_a(o.Class, "POLYGON")]
       ) >>
      ch_p(']') >> eol_p >>

      *(( "Type=0x"   >> hex_p[assign_a(o.Type)]  >> eol_p) |
        ( "Label="    >> (*ch)[assign_a(o.Label)] >> eol_p) |
        ( "EndLevel=" >> uint_p[assign_a(o.EL)]   >> eol_p) |
        ( "Endlevel=" >> uint_p[assign_a(o.EL)]   >> eol_p) |
        ( "Levels="   >> uint_p[assign_a(o.EL)]   >> eol_p) |

        ((str_p("Data") | "Origin") >> uint_p[assign_a(o.BL)][clear_a(line)] >> "=" >>
           (eol_p |
           (pt_r[push_back_a(line, pt)] >>
             *(',' >> pt_r[push_back_a(line, pt)]) >> eol_p) [push_back_a(o,line)]
           )
        ) |

        ( option[erase_a(ret.Opts, key)][insert_at_a(o.Opts, key, val)] )

        ) >>
      "[END" >> *(ch-ch_p(']')) >> ch_p(']') >> eol_p[push_back_a(ret,o)];

    rule_t main_rule = header >>
        *object >> *space_p >> *(+comment >> *space_p);
    // комментарии после объектов - теряются!

    if (!parse_file("mp::read", filename, main_rule)) return false;

    // converting some fields to UTF8
    string codepage="1251";
    codepage=ret.Opts.get("CodePage", codepage);   // override by file setting
    codepage=opts.get("mp_in_codepage", codepage); // override by user setting
    IConv cnv("CP" + codepage);
    cerr << "mp::read: Using codepage: " << codepage << "\n";

    for (mp_world::iterator i = ret.begin(); i != ret.end(); i++){
      i->Label = cnv.to_utf(i->Label);
      for (Options::iterator o=i->Opts.begin(); o!=i->Opts.end(); o++){
        o->second=cnv.to_utf(o->second);
      }
    }

    ret.Name = cnv.to_utf(ret.Name);
    for (Options::iterator o=ret.Opts.begin(); o!=ret.Opts.end(); o++){
      o->second=cnv.to_utf(o->second);
    }

    for (vector<string>::iterator
                  c = ret.Comment.begin(); c != ret.Comment.end(); c++){
      *c = cnv.to_utf(*c);
    }

    // removing bad objects
    mp_world::iterator i = ret.begin();
    while (i!= ret.end()){

      mp_object::iterator l = i->begin();
      while (l!= i->end()){

        if (l->size()==0){
          std::cerr << "MP:read warning: deleting empty object segment\n";
          l = i->erase(l);
          continue;
        }
        if ((i->Class!="POI") && (l->size()==1)){
          std::cerr << "MP:read warning: deleting line segment with 1 point\n";
          l = i->erase(l);
          continue;
        }
        if ((i->Class=="POI") && (l->size()>1)){
          std::cerr << "MP:read warning: cropping POI segment with > 1 points\n";
          l->resize(1);
        }
        l++;
      }
      if (i->size()==0){
        std::cerr << "MP:read warning: deleting empty object\n";
        i = ret.erase(i);
      }
      else i++;
    }

    // merging world with ret
    mp::mp_world tmp = world;
    world=ret;
    world.insert(world.begin(), tmp.begin(), tmp.end());

    return true;
}



bool write(std::ostream & out, const mp_world & world, const Options & opts){

  // converting some fields from UTF8 to default codepage
  // setting CodePage to default_codepage;

  string codepage="1251";
  codepage=world.Opts.get("CodePage", codepage);     // override by file setting
  codepage=opts.get("mp_out_codepage", codepage); // override by user setting
  IConv cnv("CP" + codepage);
  cerr << "mp::write: Using codepage: " << codepage << "\n";

  for (vector<string>::const_iterator c = world.Comment.begin();
       c!=world.Comment.end(); c++) out << ";" << cnv.from_utf(*c) << "\n";
//cerr <<"Name" << world.Name << "->" << cnv.from_utf(world.Name) << "\n";
  out << setprecision(6) << fixed
      << "\r\n[IMG ID]"
      << "\r\nID="              << world.ID
      << "\r\nName="            << cnv.from_utf(world.Name);

  for (Options::const_iterator o=world.Opts.begin(); o!=world.Opts.end(); o++){
    if (o->first == "CodePage") out << "\r\nCodePage=" << codepage; // use our new codepage
    else out << "\r\n" << o->first << "=" << cnv.from_utf(o->second);
  }

  out << "\r\n[END-IMG ID]\r\n";

  for (mp_world::const_iterator i=world.begin(); i!=world.end(); i++){
    for (vector<string>::const_iterator c = i->Comment.begin();
         c!=i->Comment.end(); c++) out << ";" << cnv.from_utf(*c) << "\n";
    out << "\r\n[" << i->Class << "]"
        << "\r\nType=0x"     << setbase(16) << i->Type << setbase(10);
    if (i->Label != "") out << "\r\nLabel=" << cnv.from_utf(i->Label);
    if (i->EL != 0)     out << "\r\nLevels=" << i->EL;

    for (Options::const_iterator o=i->Opts.begin(); o!=i->Opts.end(); o++){
      out << "\r\n" << o->first << "=" << cnv.from_utf(o->second);
    }

    for (MultiLine<double>::const_iterator l=i->begin(); l!=i->end(); l++){
      out << "\r\nData" << i->BL << "="; 
      for (int j=0; j<l->size(); j++){
        out << ((j!=0)?",":"") << "(" 
            << (*l)[j].y << "," << (*l)[j].x << ")";
      }
    }

    out << "\r\n[END]\r\n";
  }
  return true;
}

} //namespace