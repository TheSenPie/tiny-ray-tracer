#ifndef VEC3_h
#define VEC3_h

#include "vec4.h"

#include <cmath>
#include <iostream>

using std::sqrt;

class vec3 {
public:
	double e[3];

	vec3() : e{ 0, 0, 0 } {}
	vec3(double e0, double e1, double e2) : e{e0, e1, e2} {}
  vec3(const vec3f& other);

	double x() const { return e[0]; }
	double y() const { return e[1]; }
	double z() const { return e[2]; }

	vec3 operator-() const { return vec3{ -e[0], -e[1], -e[2] }; }
	double operator[](int i) const { return e[i]; }
	double& operator[](int i) { return e[i]; }

	vec3& operator+=(const vec3& v) {
		e[0] += v.e[0];
		e[1] += v.e[1];
		e[2] += v.e[2];
		return* this;
	}

	vec3& operator*=(double t) {
		e[0] *= t;
		e[1] *= t;
		e[2] *= t;
		return *this;
	}

	vec3& operator/=(double t) {
		e[0] /= t;
		e[1] /= t;
		e[2] /= t;
		return* this;
	}

	double length() {
		return sqrt(length_squared());
	}

	double length_squared() {
		return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
	}
  
  bool near_zero() const {
    // Return true if the vector is close to zero in all dimensions.
    auto s = 1e-8;
    return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
   }
  
  static vec3 random() {
    return vec3{random_double(), random_double(), random_double()};
  }
  
  static vec3 random(double min, double max) {
    return vec3{random_double(min, max), random_double(min, max), random_double(min, max)};
  }
};

// point3 is just an alias for vec3, but useful for geometric clarity in the code.
using point3 = vec3;

// Vector Utility Functions

inline std::ostream& operator<<(std::ostream & out, const vec3 & v) {
	return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3 operator+(const vec3& u, const vec3& v) {
	return vec3{ u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2] };
}

inline vec3 operator-(const vec3& u, const vec3& v) {
	return vec3{ u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2] };
}

inline vec3 operator*(const vec3& u, const vec3& v) {
	return vec3{ u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2] };
}

inline vec3 operator*(double t, const vec3 &v) {
	return vec3{t*v.e[0], t*v.e[1], t*v.e[2]};
}

inline vec3 operator/(const vec3& v, double t) {
	return (1/t) * v;
}

inline double dot(const vec3& u, const vec3& v) {
	return u.e[0] * v.e[0] +
		u.e[1] * v.e[1] +
		u.e[2] * v.e[2];
}

inline vec3 cross(const vec3& u, const vec3& v) {
	return vec3{u.e[1] * v.e[2] - u.e[2] * v.e[1],
		u.e[2] * v.e[0] - u.e[0] * v.e[2],
		u.e[0] * v.e[1] - u.e[1] * v.e[0]};
}

inline vec3 unit_vector(vec3 v) {
	return v / v.length();
}

inline vec3 random_in_unit_disk() {
  while(true) {
    auto p = vec3(random_double(-1, 1), random_double(-1, 1), 0);
    if (p.length_squared() < 1)
      return p;
  }
}

inline vec3 random_in_unit_sphere() {
  while(true) {
    auto p = vec3::random(-1, 1);
    if (p.length_squared() < 1)
      return p;
  }
}

inline vec3 random_unit_vector() {
  return unit_vector(random_in_unit_sphere());
}

inline vec3 random_on_hemisphere(const vec3& normal) {
  vec3 on_unit_sphere = random_unit_vector();
  if (dot(on_unit_sphere, normal) > 0.0) // In the name hemisphere as the normal
    return on_unit_sphere;
  else
    return -on_unit_sphere;
}

inline vec3 reflect(const vec3& v, const vec3& n) {
  return v - 2*dot(v,n)*n;
}

inline vec3 refract(const vec3& uv, const vec3&n, double etai_over_etat) {
  auto cos_theta = fmin(dot(-uv, n), 1.0);
  vec3 r_out_perp = etai_over_etat * (uv + cos_theta*n);
  vec3 r_out_parallel = -sqrt(fabs(1.0 - r_out_perp.length_squared())) * n;
  return r_out_perp + r_out_parallel;
}

inline vec3 fminf( const vec3& a, const vec3& b ) { return vec3{ fminf( a.x(), b.x() ), fminf( a.y(), b.y() ), fminf( a.z(), b.z() ) }; }
inline vec3 fmaxf( const vec3& a, const vec3& b ) { return vec3{ fmax( a.x(), b.x() ), fmax( a.y(), b.y() ), fmax( a.z(), b.z() ) }; }

/* float version */
class vec3f {
public:
  float e[3];

  vec3f() : e{ 0.0f, 0.0f, 0.0f } {}
  vec3f(float e0, float e1, float e2) : e{e0, e1, e2} {}
  vec3f(const vec3& other) : e{(float) other.x(), (float) other.y(), (float) other.z()} {}
  
  vec3f(const vec4f& other);

  float x() const { return e[0]; }
  float y() const { return e[1]; }
  float z() const { return e[2]; }

  vec3f operator-() const { return vec3f{ -e[0], -e[1], -e[2] }; }
  float operator[](int i) const { return e[i]; }
  float& operator[](int i) { return e[i]; }

  vec3f& operator+=(const vec3f& v) {
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    return* this;
  }

  vec3f& operator*=(float t) {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    return *this;
  }

  vec3f& operator/=(float t) {
    e[0] /= t;
    e[1] /= t;
    e[2] /= t;
    return* this;
  }

  float length() {
    return sqrt(length_squared());
  }

  float length_squared() {
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2];
  }
  
  bool near_zero() const {
    // Return true if the vector is close to zero in all dimensions.
    auto s = 1e-8;
    return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s);
   }
};

// point3f is just an alias for vec3f, but useful for geometric clarity in the code.
using point3f = vec3f;

// Vector Utility Functions

inline std::ostream& operator<<(std::ostream & out, const vec3f & v) {
	return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2];
}

inline vec3f operator+(const vec3f& u, const vec3f& v) {
	return vec3f{ u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2] };
}

inline vec3f operator-(const vec3f& u, const vec3f& v) {
	return vec3f{ u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2] };
}

inline vec3f operator*(const vec3f& u, const vec3f& v) {
	return vec3f{ u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2] };
}

inline vec3f operator*(float t, const vec3f &v) {
	return vec3f{t*v.e[0], t*v.e[1], t*v.e[2]};
}

inline vec3f operator/(const vec3f& v, float t) {
	return (1/t) * v;
}

inline double dot(const vec3f& u, const vec3f& v) {
	return u.e[0] * v.e[0] +
		u.e[1] * v.e[1] +
		u.e[2] * v.e[2];
}

inline vec3f cross(const vec3f& u, const vec3f& v) {
	return vec3f{u.e[1] * v.e[2] - u.e[2] * v.e[1],
		u.e[2] * v.e[0] - u.e[0] * v.e[2],
		u.e[0] * v.e[1] - u.e[1] * v.e[0]};
}

inline vec3f unit_vector(vec3f v) {
	return v / v.length();
}

inline vec3f fminf( const vec3f& a, const vec3f& b ) { return vec3f{ fminf( a.x(), b.x() ), fminf( a.y(), b.y() ), fminf( a.z(), b.z() ) }; }
inline vec3f fmaxf( const vec3f& a, const vec3f& b ) { return vec3f{ fmax( a.x(), b.x() ), fmax( a.y(), b.y() ), fmax( a.z(), b.z() ) }; }

inline vec3f random_in_unit_disk_flat(double max) {
  while(true) {
    auto p = vec3f((random_double_exp() - 0.5) * 2.0 * max, 0, (random_double_exp() - 0.5) * 2.0 * 800);
    return p;
  }
}

vec3::vec3(const vec3f& other) : e{ other.x(), other.y(), other.z() } {}


#endif
