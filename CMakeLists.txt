cmake_minimum_required(VERSION 3.27.0)
project(tiny-ray-tracer)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 20)

add_executable(${PROJECT_NAME} main.cpp vec4.h vec3.h vec2.h mat4.h color.h texture.h ray.h material.h hittable.h sphere.h triangle.h model.h hittable_list.h model.h rtweekend.h interval.h aabb.h bvh.h tlas.h camera.h rtw_stb_image.h)

include_directories("include")

add_subdirectory(deps/assimp)

target_include_directories(${PROJECT_NAME} PUBLIC deps/assimp/include)

target_link_directories(${PROJECT_NAME} PRIVATE deps/assimp/code)

target_link_libraries(${PROJECT_NAME} assimp)
