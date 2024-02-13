#include "rtweekend.h"

#include "bvh.h"
#include "camera.h"
#include "color.h"
#include "hittable_list.h"
#include "material.h"
#include "sphere.h"
#include "model.h"

#include <array>
#include <fstream>

int main(int argc, char* argv[])
{
//  std::ofstream out("debug-info.txt");
//
//  // Get the rdbuf of clog.
//  // We need it to reset the value before exiting.
//  auto old_rdbuf = std::clog.rdbuf();
//
//  // Set the rdbuf of clog.
//  std::clog.rdbuf(out.rdbuf());
  
  hittable_list world;
  
  auto checker = make_shared<checker_texture>(0.32, color(.2,  .3, .1), color(.9, .9, .9));
  
  auto ground_material = make_shared<lambertian>(checker);
  
  // intialize a scene with N random triangles
//	for (int i = 0; i < 64; i++)
//	{
//		vec3 r0 = vec3( random_double(), random_double(), random_double() );
//		vec3 r1 = vec3( random_double(), random_double(), random_double() );
//		vec3 r2 = vec3( random_double(), random_double(), random_double() );
// 
//    vec3 v1{9* r0 - vec3(5,5,5)};
//		vec3 v2{v1 + r1};
//    vec3 v3{v1 + r2};
//    auto tri = make_shared<triangle>(
//      v1, v2, v3,
//      vec3{0, 0, -1},vec3{0, 0, -1},vec3{0, 0, -1},
//      vec2{0, 0}, vec2{1, 0}, vec2{0, 1});
//    tri->mat = ground_material;
//    world.add(tri);
//	}
  
//  world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));
 
  for (int a = -111; a < 111; a++) {
    for (int b = -11; b < 11; b++) {
      auto choose_mat = random_double();
      point3 center{a + 0.9*random_double(), 0.2, b + 0.9*random_double()};
      
      if ((center - point3{4, 0.2, 0}).length() > 0.9) {
        shared_ptr<material> sphere_material;
        
        if (choose_mat < 0.8) {
          // diffuse
          auto albedo = color::random() * color::random();
          sphere_material = make_shared<lambertian>(albedo);
          world.add(make_shared<sphere>(center, 0.2, sphere_material));
        } else if (choose_mat < 0.95) {
            // metal
            auto albedo = color::random(0.5, 1);
            auto fuzz = random_double(0, 0.5);
            sphere_material = make_shared<metal>(albedo, fuzz);
            world.add(make_shared<sphere>(center, 0.2, sphere_material));
        } else {
          // glass
          sphere_material = make_shared<dielectric>(1.5);
          world.add(make_shared<sphere>(center, 0.2, sphere_material));
        }
      }
    }
  }
  
  
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
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/dragon.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/cube.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/weird-cube/weird-cube.obj";
//  world.add(make_shared<model>(modelPath.c_str()));
//  world = hittable_list(make_shared<bvh_node>(world));
 
  std::clog << "Amount of prmiitives: " << world.objects.size() << std::endl;
  build_bvh(world);
    
//  for (unsigned int i = 0; i < nodes_used; i++) {
//    if (!nodes[i].isLeaf()) {
//      std::clog << i << " left: " << nodes[i].left_first << " right: " << nodes[i].left_first + 1 << " " << nodes[i].bbox.x << nodes[i].bbox.y << nodes[i].bbox.z
//      << " " << nodes[i].hittable_count << std::endl;
//    }
//  }

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
  
  // Reset the rdbuf of clog.
//  std::clog.rdbuf(old_rdbuf);
  return 0;
}


/**
  Render sample - dragon.obj
  Model has: 11102 faces;
  Render time: 237024ms ~ 4 minutes
*/


// bvh
//➜  tiny-ray-tracer git:(dev) ✗ ./out/Debug/tiny-ray-tracer "image - bvh speheres.png" -- single sphere
//Render time: 754.14ms
//➜  tiny-ray-tracer git:(dev) ✗ ./out/Debug/tiny-ray-tracer "image - bvh speheres.png" -- many spheres 11 11
//Render time: 29067.5ms
//➜  tiny-ray-tracer git:(dev) ✗ ./out/Debug/tiny-ray-tracer "image - bvh speheres.png" -- many spheres 11 11
//Render time: 8972.48ms
//➜  tiny-ray-tracer git:(dev) ✗ ./out/Debug/tiny-ray-tracer "image - bvh speheres.png" -- many spheres 111 111
//Render time: 56920.6ms
//Amount of prmiitives: 4882
//➜  tiny-ray-tracer git:(dev) ✗ ./out/Debug/tiny-ray-tracer "image - bvh speheres.png" -- many spheres 111 111
//Amount of prmiitives: 4882
//Render time: 116749ms
//➜  tiny-ray-tracer git:(dev) ./out/Debug/tiny-ray-tracer "image - sah speheres.png"
//Amount of prmiitives: 4882
//Render time: 50669ms
// no bvh
//➜  tiny-ray-tracer git:(dev) ✗ ./out/Debug/tiny-ray-tracer "image - bvh speheres.png"
//Render time: 24297.1ms
//➜  tiny-ray-tracer git:(dev) ✗ ./out/Debug/tiny-ray-tracer "image - bvh tris.png"
//Render time: 4262.82ms
