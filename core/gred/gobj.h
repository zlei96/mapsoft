#ifndef GOBJ_IFACE_H
#define GOBJ_IFACE_H

#include "2d/rect.h"
#include "2d/image.h"

// return codes for draw function
extern const int GOBJ_FILL_NONE; // object draws nothing
extern const int GOBJ_FILL_PART; // object draws some points
extern const int GOBJ_FILL_ALL; // object fills in the whole image with opaque colors

extern const iRect GOBJ_MAX_RANGE;

class GObj{
public:

  GObj();

  virtual int draw(iImage &img, const iPoint &origin) = 0;

  virtual iRect range(void) const;

  virtual void set_scale(const double k);

  virtual bool get_xloop() const;
  virtual bool get_yloop() const;
};

#endif