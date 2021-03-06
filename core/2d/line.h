#ifndef LINE_H
#define LINE_H

#include <boost/operators.hpp>
#include <iostream>
#include <ios>
#include <cmath>
#include <list>
#include <vector>
#include "point.h"
#include "rect.h"

///\addtogroup lib2d
///@{
///\defgroup line
///@{


/// 2d line:  std::vector<Point<T> >.
template <typename T>
struct Line : std::vector<Point<T> >
#ifndef SWIG
    , public boost::additive<Line<T> >,
    public boost::additive<Line<T>, Point<T> >,
    public boost::multiplicative<Line<T>,T>,
    public boost::less_than_comparable<Line<T> >,
    public boost::equality_comparable<Line<T> >
#endif
{

  /// Divide coordinates by k.
  Line<T> & operator/= (T k) {
    for (typename Line<T>::iterator i=this->begin(); i!=this->end(); i++) (*i)/=k;
    return *this;
  }

  /// Multiply coordinates by k.
  Line<T> & operator*= (T k) {
    for (typename Line<T>::iterator i=this->begin(); i!=this->end(); i++) (*i)*=k;
    return *this;
  }

  /// Add p to every point of line.
  Line<T> & operator+= (Point<T> p) {
    for (typename Line<T>::iterator i=this->begin(); i!=this->end(); i++) (*i)+=p;
    return *this;
  }

  /// Subtract p from every point of line.
  Line<T> & operator-= (Point<T> p) {
    for (typename Line<T>::iterator i=this->begin(); i!=this->end(); i++) (*i)-=p;
    return *this;
  }

  /// Calculate line length.
  double length() const {
    double ret=0;
    for (int i=0; i<this->size()-1; i++)
      ret+=pdist((*this)[i], (*this)[i+1]);
    return ret;
  }

#ifndef SWIG
  /// Less then operator.
  /// линия меньше, если первая отличающаяся точка меньше,
  /// или не существует
  bool operator< (const Line<T> & p) const {
    typename Line<T>::const_iterator i1=this->begin(), i2=p.begin();
    do {
      if (i1==this->end()){
        if (i2==p.end()) return false;
        else return true;
      }
      else if (i2==p.end()) return false;

      if ((*i1)!=(*i2)) return (*i1) < (*i2);
      i1++; i2++;
    } while(1);
  }

  /// Equal opertator.
  bool operator== (const Line<T> & p) const {
    if (this->size()!=p.size()) return false;
    typename Line<T>::const_iterator i1=this->begin(), i2=p.begin();
    do {
      if (i1==this->end()) return true;
      if ((*i1)!=(*i2)) return false;
      i1++; i2++;
    } while(1);
  }
#endif

  /// такая же проверка, как ==, но для линий идущих навстречу...
  bool isinv(const Line<T> & p) const {
    if (this->size()!=p.size()) return false;
    typename Line<T>::const_iterator i1=this->begin();
    typename Line<T>::const_reverse_iterator  i2=p.rbegin();
    do {
      if (i1==this->end()) return true;
      if ((*i1)!=(*i2)) return false;
      i1++; i2++;
    } while(1);
  }

  /// Invert line.
  Line<T> inv(void) const{
    Line<T> ret;
    for (typename Line<T>::const_reverse_iterator i=this->rbegin();
                              i!=this->rend(); i++) ret.push_back(*i);
    return ret;
  }

  /// Is line l just shifted version of this.
  /// проверить, не переходит ли линия в линию l сдвигом на некоторый вектор
  /// (вектор записывается в shift)
  bool isshifted(const Line<T> & l, Point<T> & shift) const{
    shift = Point<T>(0,0);
    if (this->size()!=l.size()) return false;
    if (this->size()==0) return true;
    typename Line<T>::const_iterator i1=this->begin(), i2=l.begin();
    shift = (*i2) - (*i1);
    do {
      if (i1==this->end()) return true;
      if ((*i2)-(*i1) != shift) return false;
      i1++; i2++;
    } while(1);
  }

  /// Line range.
  Rect<T> range() const{
    if (this->size()<1) return Rect<T>(0,0,0,0);
    Point<T> min((*this)[0]), max((*this)[0]);

    for (typename Line<T>::const_iterator i = this->begin(); i!=this->end(); i++){
      if (i->x > max.x) max.x = i->x;
      if (i->y > max.y) max.y = i->y;
      if (i->x < min.x) min.x = i->x;
      if (i->y < min.y) min.y = i->y;
    }
    return Rect<T>(min,max);
  }

  /// Center of the range.
  /// \todo use the same name for Line and Rect?
  Point<T> center() const{
    return range().CNT();
  }

#ifndef SWIG
  /// Cast to Line<double>
  operator Line<double>() const{
    Line<double> ret;
    for (typename Line<T>::const_iterator i=this->begin(); i!=this->end(); i++)
      ret.push_back(dPoint(*i));
    return ret;
  }

  /// Cast to Line<int>
  operator Line<int>() const{
    Line<int> ret;
    for (typename Line<T>::const_iterator i=this->begin(); i!=this->end(); i++)
      ret.push_back(iPoint(*i));
    return ret;
  }
#endif  // SWIG
#ifdef SWIG
  %extend {
    Line<T> operator+ (Point<T> &p) { return *$self + p; }
    Line<T> operator- (Point<T> &p) { return *$self - p; }
    Line<T> operator* (T p) { return *$self * p; }
    Line<T> operator/ (T p) { return *$self / p; }
    swig_cmp(Line<T>);
    swig_str();
  }
#endif  // SWIG
};

/// \relates Line
/// \brief Line with double coordinates
typedef Line<double> dLine;
/// \relates Line
/// \brief Line with int coordinates
typedef Line<int>    iLine;

/// \relates Line
/// \brief Output operator: print Line as comma-separated points
template <typename T>
std::ostream & operator<< (std::ostream & s, const Line<T> & l){
  for (typename Line<T>::const_iterator i=l.begin(); i!=l.end(); i++)
    s << ((i==l.begin())? "":",") << *i;
  return s;
}

/// \relates Rect
/// \brief Input operator: read Line from a comma-separated points string
template <typename T>
std::istream & operator>> (std::istream & s, Line<T> & l){
  Point<T> p;
  char sep;
  s >> std::ws;
  if (s.good()){
    s >> p;
    l.push_back(p);
  }
  while (s.good()){
    s >> std::ws >> sep >> std::ws >> p;
    if (sep!=','){
      s.setstate(std::ios::failbit);
      return s;
    }
    l.push_back(p);
  }
  s.setstate(std::ios::goodbit);
  return s;
}

/// Line with multiple segments (std::vector<Line<T> >)
template <typename T>
struct MultiLine : std::vector<Line<T> >
#ifndef SWIG
    , public boost::additive<MultiLine<T> >,
    public boost::additive<MultiLine<T>, Point<T> >,
    public boost::multiplicative<MultiLine<T>,T>,
    public boost::less_than_comparable<MultiLine<T> >,
    public boost::equality_comparable<MultiLine<T> >
#endif  //SWIG
{

  /// Divide coordinates by k.
  MultiLine<T> & operator/= (T k) {
    for (typename MultiLine<T>::iterator i=this->begin(); i!=this->end(); i++) (*i)/=k;
    return *this;
  }

  /// Multiply coordinates by k.
  MultiLine<T> & operator*= (T k) {
    for (typename MultiLine<T>::iterator i=this->begin(); i!=this->end(); i++) (*i)*=k;
    return *this;
  }

  /// Add p to every point of line.
  MultiLine<T> & operator+= (Point<T> p) {
    for (typename MultiLine<T>::iterator i=this->begin(); i!=this->end(); i++) (*i)+=p;
    return *this;
  }

  /// Subtract p from every point of line.
  MultiLine<T> & operator-= (Point<T> p) {
    for (typename MultiLine<T>::iterator i=this->begin(); i!=this->end(); i++) (*i)-=p;
    return *this;
  }

  /// Calculate sum of line lengths.
  double length () const {
    double ret=0;
    for(typename MultiLine<T>::const_iterator i=this->begin(); i!=this->end(); i++)
      ret+=i->length();
    return ret;
  }

#ifndef SWIG
  /// Less then operator.
  /// меньше, если первая отличающаяся линия меньше,
  /// или не существует
  bool operator< (const MultiLine<T> & p) const {
    typename MultiLine<T>::const_iterator i1=this->begin(), i2=p.begin();
    do {
      if (i1==this->end()){
        if (i2==p.end()) return false;
        else return true;
      }
      else if (i2==p.end()) return false;

      if ((*i1)!=(*i2)) return (*i1) < (*i2);
      i1++; i2++;
    } while(1);
  }

  /// Equal opertator.
  bool operator== (const MultiLine<T> & p) const {
    if (this->size()!=p.size()) return false;
    typename MultiLine<T>::const_iterator i1=this->begin(), i2=p.begin();
    do {
      if (i1==this->end()) return true;
      if ((*i1)!=(*i2)) return false;
      i1++; i2++;
    } while(1);
  }
#endif

  /// MultiLine range.
  Rect<T> range() const{
    if (this->size()<1) return Rect<T>(0,0,0,0);
    typename MultiLine<T>::const_iterator i=this->begin();
    Rect<T> ret=i->range();
    while ((++i) != this->end())  ret = rect_bounding_box(ret, i->range());
    return ret;
  }

  /// Center of the range.
  /// \todo use the same name for Line, MultiLine and Rect?
  Point<T> center() const{
    return range().CNT();
  }

#ifndef SWIG
  /// Cast to MultiLine<double>
  operator MultiLine<double>() const{
    MultiLine<double> ret;
    for (typename MultiLine<T>::const_iterator i=this->begin(); i!=this->end(); i++)
      ret.push_back(dLine(*i));
    return ret;
  }

  /// Cast to MultiLine<int>
  operator MultiLine<int>() const{
    MultiLine<int> ret;
    for (typename MultiLine<T>::const_iterator i=this->begin(); i!=this->end(); i++)
      ret.push_back(iLine(*i));
    return ret;
  }
#else  //SWIG
  %extend {
    MultiLine<T> operator+ (Point<T> &p) { return *$self + p; }
    MultiLine<T> operator- (Point<T> &p) { return *$self - p; }
    MultiLine<T> operator* (T p) { return *$self * p; }
    MultiLine<T> operator/ (T p) { return *$self / p; }
    swig_cmp(MultiLine<T>);
  }
#endif  //SWIG  
};

/// \relates MultiLine
/// \brief MultiLine with double coordinates
typedef MultiLine<double> dMultiLine;
/// \relates MultiLine
/// \brief MultiLine with int coordinates
typedef MultiLine<int>    iMultiLine;

#endif /* LINE_H */
