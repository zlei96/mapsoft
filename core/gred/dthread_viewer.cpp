#include "dthread_viewer.h"

DThreadViewer::DThreadViewer(GObj * pl) :
    SimpleViewer(pl),
    fast_obj(&default_fast_obj),
    updater_needed(true) {

  if (!Glib::thread_supported()) Glib::thread_init();
  done_signal.connect(sigc::mem_fun(*this, &DThreadViewer::on_done_signal));

  updater_mutex = new(Glib::Mutex);
  updater_cond = new(Glib::Cond);
  updater_thread = Glib::Thread::create(sigc::mem_fun(*this, &DThreadViewer::updater), true);
}

DThreadViewer::~DThreadViewer(){

    updater_mutex->lock();
    updater_needed = false;
    updater_cond->signal();
    updater_mutex->unlock();

    updater_thread->join(); // waiting for our thread to exit
    delete(updater_mutex);
    delete(updater_cond);
}

void DThreadViewer::set_fast_obj(){
  fast_obj = &default_fast_obj;
}

void DThreadViewer::set_fast_obj(GObj * o){
  fast_obj=o;
}

GObj * DThreadViewer::get_fast_obj() const {
  return fast_obj;
}

void DThreadViewer::redraw (void){
  updater_mutex->lock();
  tiles_cache.clear();
  tiles_todo.clear();
  updater_mutex->unlock();
  inc_epoch();
  draw(iRect(0, 0, get_width(), get_height()));
}

iRect DThreadViewer::tile_to_rect(const iPoint & key) const{
  return iRect(key, key + iPoint(1,1))*TILE_SIZE;
}

void DThreadViewer::updater(){
  do {

    // generate tiles
    updater_mutex->lock();
    if (!tiles_todo.empty()){

      iPoint key = *tiles_todo.begin();

      int e=get_epoch();
      updater_mutex->unlock();
      iImage tile = get_obj()->draw(tile_to_rect(key));
      updater_mutex->lock();
      if (e==get_epoch()){
        if (tiles_cache.count(key)>0) tiles_cache.erase(key);
        tiles_cache.insert(std::pair<iPoint,iImage>(key, tile));
        tiles_done.push(key);
        tiles_todo.erase(key);
        done_signal.emit();
      }
    }
    updater_mutex->unlock();

    // cleanup queue
    iRect tiles_to_keep = tiles_on_rect(
        iRect(get_origin().x, get_origin().y,  get_width(), get_height()), TILE_SIZE);

    std::set<iPoint>::iterator qit=tiles_todo.begin(), qit1;
    while (qit!=tiles_todo.end()) {
      if (point_in_rect(*qit, tiles_to_keep)) qit++;
      else {
        qit1=qit; qit1++;
        updater_mutex->lock();
        tiles_todo.erase(qit);
        updater_mutex->unlock();
        qit=qit1;
      }
    }

    // cleanup cache
    tiles_to_keep = rect_pump(tiles_to_keep, TILE_MARG);

    std::map<iPoint,iImage>::iterator it=tiles_cache.begin(), it1;
    while (it!=tiles_cache.end()) {
      if (point_in_rect(it->first, tiles_to_keep)) it++;
      else {
        it1=it; it1++;
        updater_mutex->lock();
        tiles_cache.erase(it);
        updater_mutex->unlock();
        it=it1;
      }
    }

    if (tiles_todo.empty()) updater_cond->wait(*updater_mutex);
    updater_mutex->unlock();
  }
  while (updater_needed);
}


void DThreadViewer::on_done_signal(){
  while (!tiles_done.empty()){
    iPoint key=tiles_done.front();
    draw(tile_to_rect(key)-get_origin());

    updater_mutex->lock();
    tiles_done.pop();
    updater_mutex->unlock();
  }
}


void DThreadViewer::draw(const iRect & r){
  if (r.empty()) return;
  iRect tiles = tiles_on_rect(r + get_origin(), TILE_SIZE);
  iPoint key;

  for (key.y = tiles.y; key.y<tiles.y+tiles.h; key.y++){
    for (key.x = tiles.x; key.x<tiles.x+tiles.w; key.x++){

      iRect rect=tile_to_rect(key);
      clip_rect_to_rect(rect,r + get_origin());
      if (rect.empty()) continue;

      if (tiles_cache.count(key)==0){ // if there is no tile in cache
        draw_image(fast_obj->draw(rect), rect.TLC()-get_origin());
        if (tiles_todo.count(key)==0){
          updater_mutex->lock();
          tiles_todo.insert(key);
          updater_cond->signal();
          updater_mutex->unlock();
        }
      }
      else {
        draw_image(tiles_cache.find(key)->second,
          rect - key*TILE_SIZE, rect.TLC()-get_origin());
      }
    }
  }
}
