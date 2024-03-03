#ifndef TRIANGLE_H
#define TRIANGLE_H

#define TEST_CULL

#include "rtweekend.h"
#include "hittable.h"

class triangle : public hittable {
public:
  point3 v1, v2, v3;
  point3 n1, n2, n3;
  vec2 uv1, uv2, uv3;
 
  shared_ptr<material> mat = nullptr;
  
  triangle() = default;
  
  triangle(
    point3 _v1, point3 _v2, point3 _v3,
    point3 _n1, point3 _n2, point3 _n3,
    point2 _uv1, point2 _uv2, point2 _uv3)
    :
    v1{_v1}, v2{_v2}, v3{_v3}, n1{_n1}, n2{_n2}, n3{_n3}, uv1{_uv1}, uv2{_uv2}, uv3{_uv3}
  {
    update_bounds();
  }
  
  void update_bounds () {
     // calculate aabb
    bbox.bmin = fminf(bbox.bmin, vec3f{v1});
    bbox.bmin = fminf(bbox.bmin, vec3f{v2});
    bbox.bmin = fminf(bbox.bmin, vec3f{v3});
    bbox.bmax = fmaxf(bbox.bmax, vec3f{v1});
    bbox.bmax = fmaxf(bbox.bmax, vec3f{v2});
    bbox.bmax = fmaxf(bbox.bmax, vec3f{v3});
 
    bbox = bbox.pad();
 
    // callculate centroid
//    center = point3{
//      (bbox.x.min + bbox.x.max) / 2.0,
//      (bbox.y.min + bbox.y.max) / 2.0,
//      (bbox.z.min + bbox.z.max) / 2.0
//    };
    center = 0.3333333333333 * (v1 + v2 + v3); 
  }
  
      
  aabb bounding_box() const override { return bbox; }
  
  point3f centroid() const override { return center; }

  bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
    double t, u, v; // t - represents the distance from ray origin to hit point
    bool hit = intersect_triangle(r, v1, v2, v3, t, u, v);
    // Find if the nearest root lies in the acceptable range
    if (!hit || !ray_t.surrounds(t)) {
       return false;
    }

    auto normal = (1 - u - v) * n1 + u * n2 + v * n3;
    auto tex_u = uv1.x() * (1 - u - v)
      + uv2.x() * u
      + uv3.x() * v;
    auto tex_v = uv1.y() * (1 - u - v)
      + uv2.y() * u
      + uv3.y() * v;
    
    rec.t = t;
    rec.p = r.at(rec.t);
    rec.set_face_normal(r, normal);
    rec.u = tex_u;
    rec.v = tex_v;
    rec.mat = mat;
    return true;
    return false;
  }
 
    friend std::ostream& operator<<(std::ostream & out, const triangle & t);
private:
  point3 center;
  aabb bbox;
  
  static bool intersect_triangle(
    const ray& r,
    const point3 v0, const point3 v1, const point3 v2,
    double& t, double& u, double& v) {
      /* find vectors for two edges sharing v0 */
      point3 edge1 = v1 - v0;
      point3 edge2 = v2 - v0;
    
      /* begin calculating determinant - also used to calculate U parameter */
      point3 pvec = cross(r.direction(), edge2);

      /* if determinant is near zero, ray lies in plane of triangle */
      double det = dot(edge1, pvec);

#ifdef TEST_CULL           /* define TEST_CULL if culling is desired */
      if (det < epsilon) {
        return false;
      }

      /* calculate distance from v0 to ray origin */
      point3 tvec = r.origin() - v0;

      /* calculate U parameter and test bounds */
      u = dot(tvec, pvec);
      if (u < 0.0 || u > det)
          return false;

      /* prepare to test V parameter */
      point3 qvec = cross(tvec, edge1);

      /* calculate V parameter and test bounds */
      v = dot(r.direction(), qvec);
      if (v < 0.0 || u + v > det)
          return false;

      /* calculate t, scale parameters, ray intersects triangle */
      t = dot(edge2, qvec);
      double inv_det = 1.0 / det;
      t *= inv_det;
      u *= inv_det;
      v *= inv_det;
#else                    /* the non-culling branch */
      if (det > -epsilon && det < epsilon)
          return false;
      double inv_det = 1.0 / det;

      /* calculate distance from v0 to ray origin */
      point3 tvec = r.origin() - v0;

      /* calculate U parameter and test bounds */
      u = dot(tvec, pvec) * inv_det;
      if (u < 0.0 || u > 1.0)
          return false;

      /* prepare to test V parameter */
      point3 qvec = cross(tvec, edge1);

      /* calculate V parameter and test bounds */
      v = dot(r.direction(), qvec) * inv_det;
      if (v < 0.0 || u + v > 1.0)
          return false;

      /* calculate t, ray intersects triangle */
      t = dot(edge2, qvec) * inv_det;

#endif
    return true;
  }
};

inline std::ostream& operator<<(std::ostream & out, const triangle & t) {
	return out << "vertices: \n" << t.v1 << '\n' << t.v2 << '\n' << t.v3 << "\n uvs: \n"
  << t.uv1 << '\n' << t.uv2 << '\n' << t.uv3 << '\n' << std::endl;
}

#endif
