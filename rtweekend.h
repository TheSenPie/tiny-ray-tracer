#ifndef RTWEEKEND_H
#define RTWEEKEND_H

#include <cmath>
#include <limits>
#include <memory>
#include <random>
#include <thread>

// Usings

using std::shared_ptr;
using std::make_shared;
using std::sqrt;


// Constants

const double infinity = std::numeric_limits<double>::infinity();
const double pi = 3.1415926535897932385;


// Utility functions

inline double degrees_to_radians(double degrees) {
	return degrees * pi / 180.0;
}

inline double random_double() {
  static std::thread::id this_id = std::this_thread::get_id();
  static thread_local std::mt19937 generator;
  static std::uniform_real_distribution<double> distribution(0.0, 1.0);
  return distribution(generator);
}

inline double random_double(double min, double max) {
  // Returns a random real in [min,max).
  return min + (max-min)*random_double();
}

// Common Headeres

#include "interval.h"
#include "ray.h"
#include "vec3.h"

#endif
