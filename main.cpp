#include "rtweekend.h"

#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "model.h"
#include "bvh.h"

#include <array>

int main(int argc, char* argv[])
{
  hittable_list world;

//  auto checker = make_shared<checker_texture>(0.32, color(.2,  .3, .1), color(.9, .9, .9));
//
//  auto ground_material = make_shared<lambertian>(checker);

//  world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));
// 
//  for (int a = -111; a < 111; a++) {
//    for (int b = -11; b < 11; b++) {
//      auto choose_mat = random_double();
//      point3 center{a + 0.9*random_double(), 0.2, b + 0.9*random_double()};
//      
//      if ((center - point3{4, 0.2, 0}).length() > 0.9) {
//        shared_ptr<material> sphere_material;
//        
//        if (choose_mat < 0.8) {
//          // diffuse
//          auto albedo = color::random() * color::random();
//          sphere_material = make_shared<lambertian>(albedo);
//          world.add(make_shared<sphere>(center, 0.2, sphere_material));
//        } else if (choose_mat < 0.95) {
//            // metal
//            auto albedo = color::random(0.5, 1);
//            auto fuzz = random_double(0, 0.5);
//            sphere_material = make_shared<metal>(albedo, fuzz);
//            world.add(make_shared<sphere>(center, 0.2, sphere_material));
//        } else {
//          // glass
//          sphere_material = make_shared<dielectric>(1.5);
//          world.add(make_shared<sphere>(center, 0.2, sphere_material));
//        }
//      }
//    }
//  }
 
//  auto material1 = make_shared<dielectric>(1.5);
//  world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

//  auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
//  world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

  //auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
//  world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

//  std::vector<double> verticies{
//    1.0, 0.0, -1.0,   0.0, 0.0, 1.0,  1.0, 1.0,
//    0.0, 1.0, -1.0,   0.0, 0.0, 1.0,  0.5, 0.0,
//    -1.0, 0.0, -1.0,  0.0, 0.0, 1.0,  0.0, 1.0
//  };
//  
//  std::vector<int> indicies{0, 1, 2};
//  world.add(make_shared<mesh>(verticies, indicies, ground_material));
  
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/cube.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/backpack/backpack.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/cow/cow.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/box/wooden-box.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/wood-box/wooden-box.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/smiley/smiley.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/crate/crate.obj";
  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/dragon.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/cube.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/weird-cube/weird-cube.obj";
  model model{modelPath.c_str()};
  auto model_bvh = make_shared<bvh_node>(model);
  world.add(model_bvh);
 
  world = hittable_list(make_shared<bvh_node>(world));
  
  std::clog << "Amount of prmiitives: " << world.objects.size() << std::endl;
  
  camera cam;
 
  cam.aspect_ratio      = 16.0 / 9.0;
//  cam.image_width       = 1200; // prod
//  cam.samples_per_pixel = 500; // prod
  cam.image_width = 600;
  cam.samples_per_pixel = 10;
  cam.image_width = 400;
  cam.samples_per_pixel = 12;
  cam.max_depth         = 50;

  cam.vfov     = 20;
  cam.lookfrom = point3(5, 3, 15);
  cam.lookat   = point3(0, 0,0);
  cam.vup      = vec3(0,1,0);
  cam.defocus_angle = 0.6;
  cam.focus_dist    = 15.0;
 
  if (argc == 2 && ends_with(argv[1], ".png")) {
    cam.out_path = argv[1];
  }

  cam.render(world);
  
}

// spheres
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - kaguya.png" -- no bvh
//Amount of prmiitives: 4882
//Render time: 52936ms
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - kaguya.png" -- simple bvh
//Amount of prmiitives: 4882
//Render time: 896.871ms
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - kaguya.png"
//Amount of prmiitives: 1
//Render time: 1391.54ms

// dragon model
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - kaguya.png" -- no bvh
//Model has: 32938 vertices;
//Model has: 11102 faces; At most: 33306 vertices;
//Model has normals: true
//Amount of prmiitives: 1
//Render time: 250936ms
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - kaguya.png" -- simple bvh
//Model has: 32938 vertices;
//Model has: 11102 faces; At most: 33306 vertices;
//Model has normals: true
//Amount of prmiitives: 1
//Render time: 3904.27ms
//➜  tiny-ray-tracer git:(main) ✗ ./out/Debug/tiny-ray-tracer "image - kaguya.png"
//Model has: 32938 vertices;
//Model has: 11102 faces; At most: 33306 vertices;
//Model has normals: true
//Amount of prmiitives: 1
//Render time: 1522.3ms
