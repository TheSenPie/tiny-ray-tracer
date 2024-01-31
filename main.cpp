#include "rtweekend.h"

#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "mesh.h"
#include "model.h"

#include <array>

int main(int argc, char* argv[])
{
  hittable_list world;
  
  auto checker = make_shared<checker_texture>(0.32, color(.2,  .3, .1), color(.9, .9, .9));
  
  auto ground_material = make_shared<lambertian>(checker);
  
  auto smiley = make_shared<image_texture>("/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/smiley/smiley.png");
  auto triangle_material = make_shared<lambertian>(smiley);
  
//  world.add(make_shared<sphere>(point3(0,-1000,0), 1000, triangle_material));
 
  //for (int a = -11; a < 11; a++) {
  //  for (int b = -11; b < 11; b++) {
  //    auto choose_mat = random_double();
  //    point3 center{a + 0.9*random_double(), 0.2, b + 0.9*random_double()};
  //    
  //    if ((center - point3{4, 0.2, 0}).length() > 0.9) {
  //      shared_ptr<material> sphere_material;
  //      
  //      if (choose_mat < 0.8) {
  //        // diffuse
  //        auto albedo = color::random() * color::random();
  //        sphere_material = make_shared<lambertian>(albedo);
  //        world.add(make_shared<sphere>(center, 0.2, sphere_material));
  //      } else if (choose_mat < 0.95) {
  //          // metal
  //          auto albedo = color::random(0.5, 1);
  //          auto fuzz = random_double(0, 0.5);
  //          sphere_material = make_shared<metal>(albedo, fuzz);
  //          world.add(make_shared<sphere>(center, 0.2, sphere_material));
  //      } else {
  //        // glass
  //        sphere_material = make_shared<dielectric>(1.5);
  //        world.add(make_shared<sphere>(center, 0.2, sphere_material));
  //      }
  //    }
  //  }
  //}
  
  
//  auto material1 = make_shared<dielectric>(1.5);
//  world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material1));

  auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1));
//  world.add(make_shared<sphere>(point3(-4, 1, 0), 1.0, material2));

  //auto material3 = make_shared<metal>(color(0.7, 0.6, 0.5), 0.0);
  //world.add(make_shared<sphere>(point3(4, 1, 0), 1.0, material3));

  std::vector<double> verticies{
    1.0, 0.0, -1.0,   0.0, 0.0, 1.0,  1.0, 1.0,
    0.0, 1.0, -1.0,   0.0, 0.0, 1.0,  0.5, 0.0,
    -1.0, 0.0, -1.0,  0.0, 0.0, 1.0,  0.0, 1.0
  };
  
  std::vector<int> indicies{0, 1, 2};
//  world.add(make_shared<mesh>(verticies, indicies, ground_material));
  
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/cube.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/backpack/backpack.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/cow/cow.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/box/wooden-box.obj";
  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/wood-box/wooden-box.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/smiley/smiley.obj";
  world.add(make_shared<model>(modelPath.c_str()));
  
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
  cam.lookfrom = point3(5,1,8);
  cam.lookat   = point3(0, 1,0);
  cam.vup      = vec3(0,1,0);
  cam.defocus_angle = 0.6;
  cam.focus_dist    = 10.0;
  
  if (argc == 2 && ends_with(argv[1], ".png")) {
    cam.out_path = argv[1];
  }

  cam.render(world);
  
}

