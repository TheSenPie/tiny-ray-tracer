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

  auto checker = make_shared<checker_texture>(0.32, color(.2,  .3, .1), color(.9, .9, .9));

//  auto ground_material = make_shared<lambertian>(checker);

//  world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));
 
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

//  auto material2 = make_shared<lambertian>(color(0.4, 0.2, 0.1)); // red
//  world.add(make_shared<sphere>(point3(-1, 1, 2), 1.0, material2));
//  
//  auto material2_5 = make_shared<lambertian>(color(0.2, 0.4, 0.6)); // blue
//  world.add(make_shared<sphere>(point3(0, 1, 0), 1.0, material2_5));
  
  // the one that is further away, cuts off the one closer

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
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/dragon-high-res.obj";
  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/robo/robo.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/wheelDefault.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/cube.obj";
//  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/weird-cube/weird-cube.obj";
//  std::cout << sizeof(uint) << std::endl;
  model m{modelPath.c_str()};
  bvh<triangle> b{m.primitives, m.primitives_count};
//  world.add(make_shared<bvh<triangle>>(m.primitives, m.primitives_count));
//  shared_ptr<hittable> model_bvh = make_shared<bvh_node>(m);
//  model_bvh = make_shared<rotate_y>(model_bvh, 90);
//  model_bvh = make_shared<translate>(model_bvh, vec3{-2, -7.5, 0});
//  world.add(model_bvh);
//
//  world = hittable_list(make_shared<bvh_node>(world));
  
  std::clog << "Amount of prmiitives: " << world.objects.size() << std::endl;
  
  camera cam;
 
//  cam.aspect_ratio      = 16.0 / 9.0;
//  cam.aspect_ratio      = 1.0;
//  cam.image_width       = 1200; // prod
//  cam.samples_per_pixel = 500; // prod
  cam.image_width = 1000;
  cam.samples_per_pixel = 10;
  cam.max_depth         = 50;

  cam.vfov     = 20;
  cam.lookfrom = point3(5, 7, 50.0);
  cam.lookat   = point3(0, 7,0);
  cam.vup      = vec3(0,1,0);
  cam.defocus_angle = 0.6;
  cam.focus_dist    = 50.0;
 
  if (argc == 2 && ends_with(argv[1], ".png")) {
    cam.out_path = argv[1];
  }

  cam.render(b);
  
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
