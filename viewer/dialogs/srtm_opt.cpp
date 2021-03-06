#include "srtm_opt.h"

using namespace std;

Options
DlgSrtmOpt::get_opt() const{
  Options o;

  if (m_normal->get_active()) o.put<string>("srtm_mode", "normal");
  if (m_slopes->get_active()) o.put<string>("srtm_mode", "slopes");

  if (cnt->get_active())
    o.put<double>("srtm_cnt_step", cnt_val->get_value());
  else
    o.put<double>("srtm_cnt_step", 0);

  o.put<int>("srtm_hmin", rh->get_v1());
  o.put<int>("srtm_hmax", rh->get_v2());
  o.put<int>("srtm_smin", rs->get_v1());
  o.put<int>("srtm_smax", rs->get_v2());
  o.put<string>("srtm_dir", dir->get_text());
  return o;
}

void
DlgSrtmOpt::set_opt(const Options & o){

  double step=o.get<double>("srtm_cnt_step", 0);
  if (step>0){
    cnt->set_active(true);
    cnt_val->set_value(step);
  }
  else{
    cnt->set_active(false);
  }

  string mode = o.get<string>("srtm_mode", "normal");
  if (mode == "slopes") m_slopes->set_active();
  else                  m_normal->set_active();

  rh->set(
    o.get<int>("srtm_hmin", 0),
    o.get<int>("srtm_hmax", 0)
  );
  rs->set(
    o.get<int>("srtm_smin", 0),
    o.get<int>("srtm_smax", 0)
  );
  string d=o.get<string>("srtm_dir", "");
  dir->set_text(d);
  fdlg.set_filename(d);
}

void
DlgSrtmOpt::on_ch(int mode){
  // No need to emit signal if changes does not
  // affect the current mode.
  // We get mode for which signal must be emitted:
  // 0 - all modes, 1 - normal, 2 - slopes
  if (!is_visible()) return;
  if ( ((mode == 1) && !cnt->get_active()) ||
       ((mode == 2) && !m_normal->get_active()) ||
       ((mode == 3) && !m_slopes->get_active()) ) return;
  signal_changed_.emit();
}

DlgSrtmOpt::DlgSrtmOpt(): cnt_val_adj(50,0,9999){
  add_button (Gtk::Stock::OK,     Gtk::RESPONSE_OK);
  add_button (Gtk::Stock::CANCEL, Gtk::RESPONSE_CANCEL);

  cnt     = manage(new Gtk::CheckButton("Draw contours"));
  cnt_val = manage(new Gtk::SpinButton(cnt_val_adj));

  m_normal =  manage(new Gtk::RadioButton("Height"));
  Gtk::RadioButtonGroup gr = m_normal->get_group();
  m_slopes =  manage(new Gtk::RadioButton(gr, "Slopes"));
  m_normal->set_active();

  simple_rainbow rb1(0,1,RAINBOW_NORMAL);
  simple_rainbow rb2(0,1,RAINBOW_BURNING);
  rh =  manage(
    new Rainbow(256, rb1.get_data(), rb1.get_size(), -999, 9999, 10, 0));
  rs =  manage(
    new Rainbow(256, rb2.get_data(), rb2.get_size(), 0, 90, 1, 1));

  dirbtn = manage(new Gtk::Button("Change data folder..."));
  dir  = manage(new Gtk::Label("", Gtk::ALIGN_LEFT));

  Gtk::Table *t = manage(new Gtk::Table(2,8));
  t->attach(*cnt,      0, 1, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*cnt_val,  1, 2, 0, 1, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*m_normal, 0, 2, 1, 2, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*rh,       0, 2, 2, 3, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*m_slopes, 0, 2, 3, 4, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*rs,       0, 2, 4, 5, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*dirbtn,   0, 1, 5, 6, Gtk::FILL, Gtk::SHRINK, 3, 3);
  t->attach(*dir,      0, 2, 6, 7, Gtk::FILL, Gtk::SHRINK, 3, 3);

  get_vbox()->add(*t);

  m_normal->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 0));
  m_slopes->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 0));

  cnt->signal_toggled().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 0));
  cnt_val->signal_value_changed().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 1));

  rh->signal_changed().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 2));
  rs->signal_changed().connect(
      sigc::bind(sigc::mem_fun(this, &DlgSrtmOpt::on_ch), 3));

  dirbtn->signal_clicked().connect(
      sigc::mem_fun(this, &DlgSrtmOpt::on_dirbtn));

  /* file selection dialog */
  fdlg.set_title("Choose SRTM directory");
  fdlg.signal_response().connect(
    sigc::mem_fun (this, &DlgSrtmOpt::on_fresult));
  fdlg.get_file_list()->set_sensitive(false);

}
void
DlgSrtmOpt::on_dirbtn(){
  fdlg.show_all();
  return;
}

void
DlgSrtmOpt::on_fresult(int r){
  fdlg.hide_all();
  if (r == Gtk::RESPONSE_CANCEL) return;
  std::string d = fdlg.get_filename();
  dir->set_text(d);
  signal_changed_.emit();
}
