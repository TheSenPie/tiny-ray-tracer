#ifndef TRIANGLE_H
#define TRIANGLE_H

#define TEST_CULL 1

#include "hittable.h"
#include "vec3.h"


class triangle : public hittable {
public:
	triangle(point3 _v0, point3 _v1, point3 _v2, shared_ptr<material> _material)
		: v0{ _v0 }, v1{ _v1 }, v2{ _v2 }, mat {_material} {};

	bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
		double t, u, v;

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

        rec.t = t;
        rec.p = r.at(rec.t);
        vec3 outward_normal = cross(edge1, edge2);
        rec.set_face_normal(r, outward_normal / outward_normal.length());
        rec.mat = mat;

		return true;
	}
private:
	point3 v0;
	point3 v1;
	point3 v2;
	shared_ptr<material> mat;
};

#endif
