#ifndef TRIANGLE_H
#define TRIANGLE_H

#define TEST_CULL 1

#include "hittable.h"
#include "vec3.h"

class mesh : public hittable {
public:
	mesh(std::vector<double> _verticies, std::vector<int> _indices, shared_ptr<material> _material)
		: verticies {_verticies}, indices {_indices}, mat {_material} {};
 
  bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
    for (int idx = 0; idx < indices.size(); idx += 3) {
      point3 v0{
        verticies[idx * stride], verticies[idx * stride + 1], verticies[idx * stride + 2],
      };
      point3 v1{
        verticies[(idx + 1) * stride], verticies[(idx + 1) * stride + 1], verticies[(idx + 1) * stride + 2]
      };
      point3 v2{
        verticies[(idx + 2) * stride], verticies[(idx + 2) * stride + 1], verticies[(idx + 2) * stride + 2]
      };
      
      double t, u, v;
      bool hit = intersect_triangle(r, v0, v1, v2, t, u, v);
      // Find the nearest root that lies in the acceptable range
      if (!hit || !ray_t.surrounds(t)) {
        continue; // test next triangle
      }
      
      
      point3 n0{
        verticies[idx * stride + 3], verticies[idx * stride + 4], verticies[idx * stride + 5],
      };
      point3 n1{
        verticies[(idx + 1) * stride + 3], verticies[(idx + 1) * stride + 4], verticies[(idx + 1) * stride + 5]
      };
      point3 n2{
        verticies[(idx + 2) * stride + 3], verticies[(idx + 2) * stride + 4], verticies[(idx + 2) * stride + 5]
      };
      
      auto normal = (1 - u - v) * n0 + u * n1 + v * n2;
      auto tex_u = verticies[idx * stride + 6] * (1 - u - v)
        + verticies[(idx + 1) * stride + 6] * u
        + verticies[(idx + 2) * stride + 6] * v;
      auto tex_v = verticies[idx * stride + 7] * (1 - u - v)
        + verticies[(idx + 1) * stride + 7] * u
        + verticies[(idx + 2) * stride + 7] * v;
      
      rec.t = t;
      rec.p = r.at(rec.t);
      rec.set_face_normal(r, normal);
      rec.u = tex_u;
      rec.v = tex_v;
      rec.mat = mat;
      return true;
    }
    return false;
  };

private:
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
        if (det < epsilon)
            return false;

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
 
  std::vector<double> verticies;
  std::vector<int> indices;
	shared_ptr<material> mat;
  
  static constexpr int stride = 3 + 3 + 2; // xyz n1n2n3 uv
};

#endif
