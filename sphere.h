#ifndef SPHERE_H
#define SPHERE_H

#include "hittable.h"
#include "vec3.h"

class sphere : public hittable {
public:
	sphere(point3 _center, double _radius, shared_ptr<material> _material)
    : center{ _center }, radius{ _radius }, mat{_material} {
      auto rvec = vec3f(radius, radius, radius);
      bbox = aabb(vec3f{center} - rvec, vec3f{center} + rvec);
    };

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
		vec3 oc = r.origin() - center;
		auto a = r.direction().length_squared();
		auto half_b = dot(oc, r.direction());
		auto c = oc.length_squared() - radius * radius;

		auto discriminant = half_b * half_b - a * c;
		if (discriminant < 0) return false;
		auto sqrtd = sqrt(discriminant);
		
		// Find the nearest root that lies in the acceptable range
		auto root =	( - half_b - sqrt(discriminant)) / a;
		if (!ray_t.surrounds(root)) {
			root = (-half_b + sqrt(discriminant)) / a;
			if (!ray_t.surrounds(root)) {
				return false;
			}
		}

		rec.t = root;
		rec.p = r.at(rec.t);
		vec3 outward_normal = (rec.p - center) / radius;
		rec.set_face_normal(r, outward_normal);
    get_sphere_uv(outward_normal, rec.u, rec.v);
		rec.mat = mat;

		return true;
	}
 
  aabb bounding_box() const override { return bbox; }
  point3f centroid() const override { return vec3f{center}; }
 
private:
	point3 center;
	double radius;
  shared_ptr<material> mat;
  aabb bbox;
  
  static void get_sphere_uv(const point3& p, double& u, double& v) {
    // p: a given point on the sphere of radius one, centered at the origin.
    // u: returned value [0,1] of angle around the Y axis from X=-1.
    // v: returned value [0,1] of angle from Y=-1 to Y=+1.
    //     <1 0 0> yields <0.50 0.50>       <-1  0  0> yields <0.00 0.50>
    //     <0 1 0> yields <0.50 1.00>       < 0 -1  0> yields <0.50 0.00>
    //     <0 0 1> yields <0.25 0.50>       < 0  0 -1> yields <0.75 0.50>
    
    auto theta = acos(-p.y());
    auto phi = atan2(-p.z(), p.x()) + pi;
    
    u = phi / (2*pi);
    v = theta / pi;
  }
};

#endif
