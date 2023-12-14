cmake_minimum_required(VERSION 3.27.0)
project(tiny-ray-tracer)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} main.cpp vec3.h color.h ray.h hittable.h sphere.h hittable_list.h rtweekend.h interval.h camera.h)
