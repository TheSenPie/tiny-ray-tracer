#ifndef VEC2_h
#define VEC2_h

#include <cmath>
#include <iostream>

using std::sqrt;

class vec2 {
public:
    double e[2];

    vec2() : e{0, 0} {}
    vec2(double e0, double e1) : e{e0, e1} {}

    double x() const { return e[0]; }
    double y() const { return e[1]; }

    vec2 operator-() const { return vec2{-e[0], -e[1]}; }
    double operator[](int i) const { return e[i]; }
    double& operator[](int i) { return e[i]; }

    vec2& operator+=(const vec2& v) {
        e[0] += v.e[0];
        e[1] += v.e[1];
        return *this;
    }

    vec2& operator*=(double t) {
        e[0] *= t;
        e[1] *= t;
        return *this;
    }

    vec2& operator/=(double t) {
        e[0] /= t;
        e[1] /= t;
        return *this;
    }

    double length() {
        return sqrt(length_squared());
    }

    double length_squared() {
        return e[0] * e[0] + e[1] * e[1];
    }

    bool near_zero() const {
        // Return true if the vector is close to zero in all dimensions.
        auto s = 1e-8;
        return (fabs(e[0]) < s) && (fabs(e[1]) < s);
    }
};

// point3 is just an alias for vec3, but useful for geometric clarity in the code.
using point2 = vec2;

// Vector Utility Functions (adapted for vec2)

inline std::ostream& operator<<(std::ostream & out, const vec2 & v) {
    return out << v.e[0] << ' ' << v.e[1];
}

inline vec2 operator+(const vec2& u, const vec2& v) {
    return vec2{u.e[0] + v.e[0], u.e[1] + v.e[1]};
}

inline vec2 operator-(const vec2& u, const vec2& v) {
    return vec2{u.e[0] - v.e[0], u.e[1] - v.e[1]};
}

inline vec2 operator*(const vec2& u, const vec2& v) {
    return vec2{u.e[0] * v.e[0], u.e[1] * v.e[1]};
}

inline vec2 operator*(double t, const vec2 &v) {
    return vec2{t * v.e[0], t * v.e[1]};
}

inline vec2 operator/(const vec2& v, double t) {
    return (1 / t) * v;
}

inline double dot(const vec2& u, const vec2& v) {
    return u.e[0] * v.e[0] + u.e[1] * v.e[1];
}

#endif 
