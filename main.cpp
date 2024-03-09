#include "rtweekend.h"

#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "model.h"
#include "bvh.h"
#include "tlas.h"

#include <array>

void final_scene(const char* out_path, int image_width, int samples_per_pixel, int max_depth) {
  hittable_list world;

  sphere* spheres = new sphere[1 + 24*24 + 3];
  int sphere_count = 0;
  
  auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
  spheres[sphere_count++] = sphere{point3(0,-1000,0), 1000, ground_material};
  
  for (int a = -11; a < 11; a++) {
    for (int b = -11; b < 11; b++) {
      auto choose_mat = random_double();
      point3 center(a + 0.9*random_double(), 0.2, b + 0.9*random_double());

      if ((center - point3(4, 0.2, 0)).length() > 0.9) {
        shared_ptr<material> sphere_material;
 
        if (choose_mat < 0.8) {
            // diffuse
            auto albedo = color::random() * color::random();
            sphere_material = make_shared<lambertian>(albedo);
            spheres[sphere_count++] = sphere{center, 0.2, sphere_material};
        } else if (choose_mat < 0.95) {
            // metal
            auto albedo = color::random(0.5, 1);
            auto fuzz = random_double(0, 0.5);
            sphere_material = make_shared<metal>(albedo, fuzz);
            spheres[sphere_count++] = sphere{center, 0.2, sphere_material};
        } else {
          // glass
          sphere_material = make_shared<dielectric>(1.5);
          spheres[sphere_count++] = sphere{center, 0.2, sphere_material};
        }
      }
    }
  }

  auto material1 = make_shared<dielectric>(1.5);
  spheres[sphere_count++] = sphere{point3(0, 1, 0), 1.0, material1};

  auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
  spheres[sphere_count++] = sphere{point3(-4, 1, 0), 1.0, material2};

  auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
  spheres[sphere_count++] = sphere{point3(4, 1, 0), 1.0, material3};
  
  world.add(make_shared<bvh<sphere>>(spheres, sphere_count));
  std::cout << "Rendering " << sphere_count << " spheres" << std::endl;

  camera cam;

  cam.aspect_ratio      = 16.0 / 9.0;
  cam.image_width       = 1920;
  cam.samples_per_pixel = 200;
  cam.max_depth         = 50;
  cam.background        = color(0.5, 0.7, 1.0);

  cam.vfov     = 20;
  cam.lookfrom = point3(13,2,3);
  cam.lookat   = point3(0,0,0);
  cam.vup      = vec3(0,1,0);

  cam.defocus_angle = 0.6;
  cam.focus_dist    = 10.0;
  
  if (out_path) {
    cam.out_path = out_path;
  }

  cam.render(world);
  
  delete[] spheres;
}

void simple_light(const char* out_path) {
  hittable_list world;

  auto checker = make_shared<checker_texture>(0.32, color(.2,  .3, .1), color(.9, .9, .9));
  world.add(make_shared<sphere>(point3(0,-1000,0), 1000, make_shared<lambertian>(checker)));
  world.add(make_shared<sphere>(point3(0,2,0), 2, make_shared<lambertian>(checker)));

  auto difflight = make_shared<diffuse_light>(color(4,4,4));
  world.add(make_shared<sphere>(point3(0,7,0), 2, difflight));

  camera cam;

  cam.aspect_ratio      = 16.0 / 9.0;
  cam.image_width       = 640;
  cam.samples_per_pixel = 100;
  cam.max_depth         = 50;
  cam.background        = color(0,0,0);

  cam.vfov     = 20;
  cam.lookfrom = point3(26,3,6);
  cam.lookat   = point3(0,2,0);
  cam.vup      = vec3(0,1,0);

  cam.defocus_angle = 0;
  
  if (out_path) {
    cam.out_path = out_path;
  }

  cam.render(world);
}

void dragon(const char* out_path, bool high_res) {
  hittable_list world;
  
  string model_path;
  if (high_res)
    model_path = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/dragon-high-res.obj";
  else
    model_path  = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/dragon.obj";
    
  auto model_material = make_shared<lambertian>(color{0.882, 0.678, 0.003});
  model m{model_path.c_str(), model_material};
  bvh<triangle> mb{m.primitives, m.primitives_count};
  auto instance = make_shared<bvh_instance<triangle>>(&mb);
  instance->set_transform(mat4::RotateY(degrees_to_radians(-25)));
  
  world.add(instance);
 
  camera cam;

  cam.aspect_ratio      = 16.0 / 9.0;
  cam.image_width       = 1920;
  cam.samples_per_pixel = 100;
  cam.max_depth         = 50;
  cam.background        = color(0.5, 0.7, 1.0);

  cam.vfov     = 20;
  cam.lookfrom = point3(5,0,15);
  cam.lookat   = point3(0.5,0.5,0);
  cam.vup      = vec3(0,1,0);

  cam.defocus_angle = 0;
  
  if (out_path) {
    cam.out_path = out_path;
  }

  cam.render(world);
}

void cow(const char* out_path) {
  hittable_list world;
  
  string model_path = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/cow/cow.obj";
  model m{model_path.c_str()};
  bvh<triangle> mb{m.primitives, m.primitives_count};
  auto instance = make_shared<bvh_instance<triangle>>(&mb);
  instance->set_transform(mat4::RotateY(degrees_to_radians(-90)));
  
  world.add(instance);

  camera cam;

  cam.aspect_ratio      = 1;
  cam.image_width       = 1280;
  cam.samples_per_pixel = 100;
  cam.max_depth         = 50;
  cam.background        = color(0.5, 0.7, 1.0);

  cam.vfov     = 20;
  cam.lookfrom = point3(3,3,6);
  cam.lookat   = point3(0,0,0);
  cam.vup      = vec3(0,1,0);

  cam.defocus_angle = 0;
  
  if (out_path) {
    cam.out_path = out_path;
  }

  cam.render(world);
}

void robot(const char* out_path, bool render_many) {
  hittable_list world;
  
  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/robo/robo.obj";
 
  model m{modelPath.c_str()};
  bvh<triangle> mb{m.primitives, m.primitives_count};
  bvh_instance<triangle>* nodes = nullptr;

  if (render_many) {
    nodes = new bvh_instance<triangle>[256];
    
    vec3f origin{-39.0f, 42.4f, 0};
    for (int i  = 0; i < 256; i++) {
      nodes[i] = bvh_instance<triangle>(&mb);
      nodes[i].set_transform(
        mat4::Translate(origin + vec3f{ (i%16) * 17.f * 0.3f, (i/16) * -17.f * 0.3f, 0  })
        * mat4::RotateY(i * (pi/256))
        * mat4::Scale(0.3f)
      );
    }
    
    shared_ptr<tlas<triangle>> models{make_shared<tlas<triangle>>(nodes, 256)};
    models->build();

    world.add(models);
  } else {
     world.add(make_shared<bvh_instance<triangle>>(&mb));
  }
  
  camera cam;

  cam.aspect_ratio      = 1.0;
  if (render_many) {
    cam.image_width = 1920;
  } else {
    cam.image_width       = 640;
  }
  cam.samples_per_pixel = 50;
  cam.max_depth         = 50;
  cam.background        = color(0.5, 0.7, 1.0);

  cam.vfov     = 20;
  if (render_many) {
    cam.lookfrom = point3(5, 7, 237.0);
    cam.lookat   = point3(0, 7,0);
    cam.focus_dist    = 237.0;
  } else {
    cam.lookfrom = point3(26, 7,50);
    cam.lookat = point3(-2, 7, 0);
    cam.focus_dist = 50;
  }
  cam.vup      = vec3(0,1,0);

  cam.defocus_angle = 0;
  
  if (out_path) {
    cam.out_path = out_path;
  }

  cam.render(world);
  
  delete[] nodes;
}

//void cubes(const char* out_path) {
//  hittable_list world;
//  
//  string cube = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/cube.obj";
//  string wooden_box = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/wood-box/wooden-box.obj";
//  string other_wooden_box = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/box/wooden-box.obj";
//  string crate = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/crate/crate.obj";
//
//  string weird_cube = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/weird-cube/weird-cube.obj";
//
//
////  model m{modelPath.c_str()};
////  bvh<triangle> mb{m.primitives, m.primitives_count};
//  camera cam;
//
//  cam.aspect_ratio      = 16.0 / 9.0;
//  cam.image_width       = 400;
//  cam.samples_per_pixel = 100;
//  cam.max_depth         = 50;
//  cam.background        = color(0,0,0);
//
//  cam.vfov     = 20;
//  cam.lookfrom = point3(26,3,6);
//  cam.lookat   = point3(0,2,0);
//  cam.vup      = vec3(0,1,0);
//
//  cam.defocus_angle = 0;
//  
//  if (out_path) {
//    cam.out_path = out_path;
//  }
//
//  cam.render(world);
//}

//void smiley(const char* out_path) {
//  hittable_list world;
//  
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/smiley/smiley.obj";
//
////  model m{modelPath.c_str()};
////  bvh<triangle> mb{m.primitives, m.primitives_count};
//  camera cam;
//
//  cam.aspect_ratio      = 16.0 / 9.0;
//  cam.image_width       = 400;
//  cam.samples_per_pixel = 100;
//  cam.max_depth         = 50;
//  cam.background        = color(0,0,0);
//
//  cam.vfov     = 20;
//  cam.lookfrom = point3(26,3,6);
//  cam.lookat   = point3(0,2,0);
//  cam.vup      = vec3(0,1,0);
//
//  cam.defocus_angle = 0;
//  
//  if (out_path) {
//    cam.out_path = out_path;
//  }
//
//  cam.render(world);
//}

void any_model(const char* out_path, const char* model_path) {
  hittable_list world;
  
//  model m{modelPath.c_str()};
//  bvh<triangle> mb{m.primitives, m.primitives_count};
 
  camera cam;

  cam.aspect_ratio      = 16.0 / 9.0;
  cam.image_width       = 400;
  cam.samples_per_pixel = 100;
  cam.max_depth         = 50;
  cam.background        = color(0,0,0);

  cam.vfov     = 20;
  cam.lookfrom = point3(26,3,6);
  cam.lookat   = point3(0,2,0);
  cam.vup      = vec3(0,1,0);

  cam.defocus_angle = 0;
  
  if (out_path) {
    cam.out_path = out_path;
  }

  cam.render(world);
}

int main(int argc, char* argv[]) {
  char* out_path = nullptr;
  if (argc >= 2 && ends_with(argv[1], ".png")) {
    out_path = argv[1];
  }
  
  char* model_path = nullptr;
  if (argc >= 3 && ends_with(argv[2], ".obj")) {
    model_path = argv[2];
  }
  
  switch (2) {
    case 0:  simple_light(out_path);                break;
    case 1:  dragon(out_path, false);               break;
    case 2:  cow(out_path);                         break;
    case 3:  robot(out_path, false);                break;
//    case 4:  cubes(out_path);                       break;
//    case 5:  smiley(out_path);                      break;
    case 6:  any_model(out_path, model_path);       break;
    default: final_scene(out_path, 1920,   250,  4); break;
  }
}

// robot
//  cam.vfov     = 20;
//  cam.lookfrom = point3(5, 7, 50.0);
//  cam.lookat   = point3(0, 7,0);
//  cam.vup      = vec3(0,1,0);
//  cam.defocus_angle = 0.6;
//  cam.focus_dist    = 50.0;

// dragon
//  cam.vfov     = 20;
//  cam.lookfrom = point3(5, 3.5, 15);
//  cam.lookat   = point3(0, 0.5,0);
//  cam.vup      = vec3(0,1,0);
//  cam.defocus_angle = 0.6;
//  cam.focus_dist    = 15.0;


// spheres
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - test.png" -- no bvh
//Amount of prmiitives: 4882
//Render time: 52936ms
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - test.png" -- simple bvh
//Amount of prmiitives: 4882
//Render time: 896.871ms
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - test.png" -- simple jakko bvh
//Amount of prmiitives: 1
//Render time: 1391.54ms
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - test.png" -- SAH jakko bvh
//Amount of prmiitives: 1
//Render time: 1321.69ms
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - test.png" -- SAH jakko iterative bvh
//Amount of prmiitives: 1
//Render time: 725.51ms

// dragon model
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - test.png" -- no bvh
//Model has: 32938 vertices;
//Model has: 11102 faces; At most: 33306 vertices;
//Model has normals: true
//Amount of prmiitives: 1
//Render time: 250936ms
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - test.png" -- simple bvh
//Model has: 32938 vertices;
//Model has: 11102 faces; At most: 33306 vertices;
//Model has normals: true
//Amount of prmiitives: 1
//Render time: 3904.27ms
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - test.png" -- simple jakko bvh
//Model has: 32938 vertices;
//Model has: 11102 faces; At most: 33306 vertices;
//Model has normals: true
//Amount of prmiitives: 1
//Render time: 1522.3ms
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - test.png" -- SAH jakko bvh
//Model has: 32938 vertices;
//Model has: 11102 faces; At most: 33306 vertices;
//Model has normals: true
//Amount of prmiitives: 1
//Render time: 1298.32ms
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - test.png" -- SAH jakko iterative bvh
//Model has: 32938 vertices;
//Model has: 11102 faces; At most: 33306 vertices;
//Model has normals: true
//Amount of prmiitives: 1
//Render time: 974.8ms
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - test.png" -- SAH jakko iterative bvh code refactor
//Mesh has: 2612235 vertices;
//Mesh has: 871414 faces; At most: 2614242 vertices;
//Mesh has normals: true
//BVH construction time: 9095.76ms
//Amount of prmiitives: 0
//Render time: 161595ms

// two robots
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - robots.png"
//Mesh has: 10860 vertices;
//Mesh has: 8850 faces; At most: 26550 vertices;
//Mesh has normals: true
//Loading texture at: /Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/robo/Texture_1K.jpg
//BVH construction time: 448.981ms
//BVH construction time: 448.272ms
//Render time: 9760.75ms

// BVH bild time comparison
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - test.png"
//Model has: 32938 vertices;
//Model has: 11102 faces; At most: 33306 vertices;
//Model has normals: true
//BVH build time: 8509.01ms
//Amount of prmiitives: 1
//Render time: 1068.17ms
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - test.png"
//Model has: 32938 vertices;
//Model has: 11102 faces; At most: 33306 vertices;
//Model has normals: true
//BVH build time: 35387.9ms
//Amount of prmiitives: 1
//Render time: 1113.28ms
//➜  tiny-ray-tracer git:(main) ✗
