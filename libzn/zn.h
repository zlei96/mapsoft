#ifndef ZN_H
#define ZN_H

#include <string>
#include <list>
#include <map>
#include <set>
#include "../geo_io/geo_convs.h"
#include "../geo_io/geofig.h"
#include "../libmp/mp.h"
#include "../yaml/yaml.h"

#include "../utils/m_time.h"

// �������� � ����.������ � ������ ��������
// �������, ���� ���� ����� ������ �� ���� ��� �����, � ������� ������ � ������...

namespace zn{

const int line_mask = 0x100000; // ������� ��������� � ���� ��� �������� ��������
const int area_mask = 0x200000; // ������� ��������� � ���� ��� ��������� ��������

// ���������� �� �������� �����������, ����������� �� ����.�����
struct zn{
  std::string name, desc;  // ��������, ��������� ��������
  mp::mp_object mp;        // ��������� mp-�������
  fig::fig_object fig;     // ��������� fig-�������
  fig::fig_object txt;     // ��������� fig-������� �������
  std::string pic;         // �������� ��� ��������� ������� (�.�. ������)
  bool istxt;              // ������ �� �������?
};


// �����, ������� (�� ����.�����) ��� ���������� �������� ����������� 
// � ������� �� ���������������
class zn_conv{
  public:

  std::map<int, zn> znaki;      // ����������� �� ����
  fig::fig_object default_fig;  // ��� �������� ����������� �����
  mp::mp_object default_mp;     // ��� �������� ����������� �����

  std::set<int> unknown_types; // ���� ����������� �����, ���� �� ������ �������� ��������
  int min_depth, max_depth;

  // ����������� - ������ ����������������� �����
  zn_conv(const std::string & conf_file);

  // ��������������� ��������� ��� ������������
  bool load_znaki(YAML::Node &root);

    // �������� � mp- � fig-���������, ��� ������� 
    // ��������� ����.����

  // ���������� ��� mp-������� (����� ����������� ������� :))
  int get_type(const mp::mp_object & o) const;

  // ���������� ��� fig-������� �� �������� ����, � ������������ � ����.������
  int get_type(const fig::fig_object & o) const;

      // � ��������� ��������, ���� ������ ��� 0, �� �� ������������ �� �������

  // ������������� mp-������ � fig-������
  fig::fig_object mp2fig(const mp::mp_object & mp, convs::map2pt & cnv, int type=0);

  // ������������� fig-������ � mp-������
  mp::mp_object fig2mp(const fig::fig_object & fig, convs::map2pt & cnv, int type=0);

  // �������� ��������� � ������������ � �����
  void fig_update(fig::fig_object & fig, int type=0);

  // ������� �������� � �������
  std::list<fig::fig_object> make_pic(const fig::fig_object & fig, int type=0);

  // ������� ������� � �������
  std::list<fig::fig_object> make_labels(const fig::fig_object & fig, int type=0);

  // �������� ��������� ������� � ������������ � �����
  // (�����, ������, ����, �������)
  // ���� ��� 0 - ������ �� ������
  void label_update(fig::fig_object & fig, int type) const;

  //������ ��������, �������� �� ������ ���������������� ��������.
  //����� �� ��� ������� � ��������� ����� ������������ � ����������� ��������
  //�������� �� ����.�����.
  bool is_map_depth(const fig::fig_object & o);
};


} // namespace

#endif
