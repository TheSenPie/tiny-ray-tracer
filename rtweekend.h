#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include <thread>
#include <vector>
#include <future>
#include <cstring>

// Usings

using std::shared_ptr;
using std::make_shared;
using std::sqrt;
using std::vector;
using std::future;

// Constants

constexpr double infinity = std::numeric_limits<double>::infinity();
constexpr double pi = 3.1415926535897932385;
constexpr double epsilon = 1e-6;


// Utility functions

inline double degrees_to_radians(double degrees) {
	return degrees * pi / 180.0;
}

static std::hash<std::thread::id> hasher;
static std::uniform_real_distribution<double> distribution(0.0, 1.0);

inline double random_double() {
  static thread_local std::mt19937 generator(
    static_cast<unsigned>(hasher(std::this_thread::get_id()))
  );
  return distribution(generator);
}

inline double random_double(double min, double max) {
  // Returns a random real in [min,max).
  return min + (max-min)*random_double();
}

inline int random_int(int min, int max) {
  // Returns a random integer in [min, max].
  return static_cast<int>(random_double(min, max+1));
}

template<typename T>
vector<T> wait_for_all(vector<future<T>>& vf) {
  vector<T> res;
  for(auto& fu : vf)
    res.push_back(fu.get());
  return res;
}

inline int ends_with(const char *str, const char *suffix) {
  size_t str_len = strlen(str);
  size_t suffix_len = strlen(suffix);

  return (str_len >= suffix_len) &&
         (!memcmp(str + str_len - suffix_len, suffix, suffix_len));
}

// Common Headeres

#include "interval.h"
#include "ray.h"

#include "vec4.h"
#include "vec3.h"
vec4f::vec4f(const vec3f& other, float a) : e{other.x(), other.y(), other.z(), a} {}
vec3f::vec3f(const vec4f& other) : e{other.x(), other.y(), other.z()} {}

#include "vec2.h"
#include "mat4.h"

#endif
