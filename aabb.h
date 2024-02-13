#ifndef AABB
#define AABB

#include "rtweekend.h"

class aabb {
public:
  interval x, y, z;
  
  aabb() {}   // The default AABB is epty, since intervals are empty by default.
  
  aabb(const interval& ix, const interval& iy, const interval& iz)
    : x{ix}, y{iy}, z{iz} { }
      
  aabb(const point3& a, const point3& b) {
    // Treat the two points a and b as extrema for the bounding box, so we don't require a
    // particular minimum/maxium coordinate order.
    x = interval(fmin(a[0], b[0]), fmax(a[0], b[0]));
    y = interval(fmin(a[1], b[1]), fmax(a[1], b[1]));
    z = interval(fmin(a[2], b[2]), fmax(a[2], b[2]));
  }
  
  aabb(const aabb& box0, const aabb& box1) {
    x = interval(box0.x, box1.x);
    y = interval(box0.y, box1.y);
    z = interval(box0.z, box1.z);
  }
  
  const interval& axis(int n) const {
    if (n == 1) return y;
    if (n == 2) return z;
    return x;
  }

//  bool hit(const ray& r, interval ray_t) const {
//    for (int a = 0; a < 3; a++) {
//      auto invD = 1 / r.direction()[a];
//      auto orig = r.origin()[a];
//
//      auto t0 = (axis(a).min - orig) * invD;
//      auto t1 = (axis(a).max - orig) * invD;
//
//      if (invD < 0)
//          std::swap(t0, t1);
//
//      if (t0 > ray_t.min) ray_t.min = t0;
//      if (t1 < ray_t.max) ray_t.max = t1;
//
//      if (ray_t.max <= ray_t.min)
//          return false;
//    }
//    return true;
//  }
  
  double hit(const ray& r, interval ray_t) const {
    double tx1{(x.min - r.origin().x()) / r.direction().x()}, tx2{(x.max - r.origin().x()) / r.direction().x()};
    double tmin{fmin( tx1, tx2 )}, tmax{fmax( tx1, tx2 )};
    double ty1{(y.min - r.origin().y()) / r.direction().y()}, ty2{(y.max - r.origin().y()) / r.direction().y()};
    tmin = fmax( tmin, fmin( ty1, ty2 ) ), tmax = fmin( tmax, fmax( ty1, ty2 ) );
    double tz1{(z.min - r.origin().z()) / r.direction().z()}, tz2{(z.max - r.origin().z()) / r.direction().z()};
    tmin = fmax( tmin, fmin( tz1, tz2 ) ), tmax = fmin( tmax, fmax( tz1, tz2 ) );
    if (tmax >= tmin && tmin < ray_t.max && tmax > 0) return tmin; else return infinity;
  }
  
  inline float area() {
    vec3 e{
      x.size(),
      y.size(),
      z.size(),
    };
    return e.x() * e.y() + e.y() * e.z() + e.z() * e.x();
  }
};
#endif
