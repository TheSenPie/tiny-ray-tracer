#ifndef VEC4_h
#define VEC4_h

#include <cmath>
#include <iostream>

using std::sqrt;

class vec3f;

/* float version */
class vec4f {
public:
  float e[4];

  vec4f() : e{ 0.0f, 0.0f, 0.0f, 0.0f } {}
  vec4f(float e0, float e1, float e2, float e3) : e{e0, e1, e2, e3} {}
  vec4f(const vec3f& other, float a);

  float x() const { return e[0]; }
  float y() const { return e[1]; }
  float z() const { return e[2]; }
  float w() const { return e[3]; }

  vec4f operator-() const { return vec4f{ -e[0], -e[1], -e[2], -e[3] }; }
  float operator[](int i) const { return e[i]; }
  float& operator[](int i) { return e[i]; }

  vec4f& operator+=(const vec4f& v) {
    e[0] += v.e[0];
    e[1] += v.e[1];
    e[2] += v.e[2];
    e[3] += v.e[3];
    return* this;
  }

  vec4f& operator*=(float t) {
    e[0] *= t;
    e[1] *= t;
    e[2] *= t;
    e[3] *= t;
    return *this;
  }

  vec4f& operator/=(float t) {
    e[0] /= t;
    e[1] /= t;
    e[2] /= t;
    e[3] /= t;
    return* this;
  }

  float length() {
    return sqrt(length_squared());
  }

  float length_squared() {
    return e[0] * e[0] + e[1] * e[1] + e[2] * e[2] + e[3] * e[3];
  }
  
  bool near_zero() const {
    // Return true if the vector is close to zero in all dimensions.
    auto s = 1e-8;
    return (fabs(e[0]) < s) && (fabs(e[1]) < s) && (fabs(e[2]) < s) && (fabs(e[3]));
   }
};

// point4f is just an alias for vec4f, but useful for geometric clarity in the code.
using point4f = vec4f;

// Vector Utility Functions

inline std::ostream& operator<<(std::ostream & out, const vec4f & v) {
	return out << v.e[0] << ' ' << v.e[1] << ' ' << v.e[2] << ' ' << v.e[3];
}

inline vec4f operator+(const vec4f& u, const vec4f& v) {
	return vec4f{ u.e[0] + v.e[0], u.e[1] + v.e[1], u.e[2] + v.e[2], u.e[3] + v.e[3] };
}

inline vec4f operator-(const vec4f& u, const vec4f& v) {
	return vec4f{ u.e[0] - v.e[0], u.e[1] - v.e[1], u.e[2] - v.e[2], u.e[3] - v.e[3] };
}

inline vec4f operator*(const vec4f& u, const vec4f& v) {
	return vec4f{ u.e[0] * v.e[0], u.e[1] * v.e[1], u.e[2] * v.e[2], u.e[3] * v.e[3] };
}

inline vec4f operator*(float t, const vec4f &v) {
	return vec4f{t*v.e[0], t*v.e[1], t*v.e[2], t*v.e[3]};
}

inline vec4f operator/(const vec4f& v, float t) {
	return (1/t) * v;
}

inline double dot(const vec4f& u, const vec4f& v) {
	return u.e[0] * v.e[0] +
		u.e[1] * v.e[1] +
		u.e[2] * v.e[2] +
    u.e[3] * v.e[3];
}

inline vec4f unit_vector(vec4f v) {
	return v / v.length();
}

inline vec4f fminf( const vec4f& a, const vec4f& b ) { return vec4f{ fminf( a.x(), b.x() ), fminf( a.y(), b.y() ), fminf( a.z(), b.z() ), fminf( a.w(), b.w() ) }; }
inline vec4f fmaxf( const vec4f& a, const vec4f& b ) { return vec4f{ fmax( a.x(), b.x() ), fmax( a.y(), b.y() ), fmax( a.z(), b.z() ), fmax( a.w(), b.w() ) }; }

#endif
