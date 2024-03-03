#ifndef AABB
#define AABB

#include "rtweekend.h"

class aabb {
public:
  point3f bmin = point3f{infinity, infinity, infinity};
  point3f bmax = point3f{-infinity, -infinity, -infinity};

  aabb() {}   // The default AABB is empty, since intervals are empty by default.

  aabb(const point3f& a, const point3f& b) {
    // Treat the two points a and b as extrema for the bounding box, so we don't require a
    // particular minimum/maxium coordinate order.
    bmin = fminf(a, b);
    bmax = fmaxf(a, b);
  }
  
  aabb(const aabb& box0, const aabb& box1) {
    bmin = fminf(box0.bmin, box1.bmin);
    bmax = fmaxf(box0.bmax, box1.bmax);
  }

  double hit(const ray& r, interval ray_t) const {
    for (int a = 0; a < 3; a++) {
      auto invD = 1 / r.direction()[a];
      auto orig = r.origin()[a];

      auto t0 = (bmin[a] - orig) * invD;
      auto t1 = (bmax[a] - orig) * invD;

      if (invD < 0)
          std::swap(t0, t1);

      if (t0 > ray_t.min) ray_t.min = t0;
      if (t1 < ray_t.max) ray_t.max = t1;

      if (ray_t.max <= ray_t.min)
          return infinity;
    }
    return ray_t.min;
  }
  
  float area() {
    vec3f extent = bmax - bmin;
    return extent.x() * extent.y() + extent.y() * extent.z() + extent.z() * extent.x();
  }
  
  aabb pad() const {
    // Return an AABB that has no side narrower than some delta, padding if necessary.
    float delta = 0.0001f;
    float delta2 = delta / 2.0f;
    vec3f tmp_bmin{bmin}, tmp_bmax{bmax};
    
    if (fabs(bmax.x() - bmin.x()) < delta)
      tmp_bmin[0] -= delta2, tmp_bmax[0] += delta2;

    if (fabs(bmax.y() - bmin.y()) < delta)
      tmp_bmin[1] -= delta2, tmp_bmax[1] += delta2;

    if (fabs(bmax.z() - bmin.z()) < delta)
      tmp_bmin[2] -= delta2, tmp_bmax[2] += delta2;

    return aabb(tmp_bmin, tmp_bmax);
  }
};

aabb operator+(const aabb& bbox, const vec3f& offset) {
    return aabb{bbox.bmin + offset, bbox.bmax + offset};
}

aabb operator+(const vec3f& offset, const aabb& bbox) {
    return bbox + offset;
}

#endif
