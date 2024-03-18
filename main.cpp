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
  model m{model_path.c_str()};
  bvh<triangle> mb{m.primitives, m.primitives_count};
  auto instance = make_shared<bvh_instance<triangle>>(&mb);
  instance->set_transform(mat4::RotateY(degrees_to_radians(-25)));
  
  world.add(instance);
 
  camera cam;

  cam.aspect_ratio      = 16.0 / 9.0;
  if (high_res) {
    cam.image_width       = 1920;
    cam.samples_per_pixel = 100;
  } else {
    cam.image_width       = 1280;
    cam.samples_per_pixel = 50;
  }
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
    cam.image_width = 640;
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

void eva(const char* out_path, bool render_many) {
  hittable_list world;
  
  string modelPath = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/eva/EVA_01.obj";
 
  model m{modelPath.c_str()};
  bvh<triangle> mb{m.primitives, m.primitives_count};
  bvh<sphere> spheres;
  bvh_instance<triangle>* nodes = nullptr;
  sphere* sphere_list = nullptr;

  if (render_many) {
    // floor
    auto mat = make_shared<lambertian>(color(0.941, 0.878, 0.905));
    
    auto tri_1 = make_shared<triangle>();
    tri_1->v1 = vec3f{-1000, 0, -1000};
    tri_1->v2 = vec3f{-1000, 0, 1000};
    tri_1->v3 = vec3f{1000, 0, -1000};
    tri_1->n1 = vec3f(0,1,0);
    tri_1->n2 = vec3f(0,1,0);
    tri_1->n3 = vec3f(0,1,0);
    tri_1->uv1= vec2{0, 0};
    tri_1->uv2= vec2{0, 1};
    tri_1->uv3= vec2{1, 0};
    tri_1->mat = mat;
    world.add(tri_1);

    auto tri_2 = make_shared<triangle>();
    tri_2->v1 = vec3f{1000, 0, -1000};
    tri_2->v2 = vec3f{-1000, 0, 1000};
    tri_2->v3 = vec3f{1000, 0, 1000};
    tri_2->n1 = vec3f(0,1,0);
    tri_2->n2 = vec3f(0,1,0);
    tri_2->n3 = vec3f(0,1,0);
    tri_2->uv1= vec2{0, 0};
    tri_2->uv2= vec2{0, 1};
    tri_2->uv3= vec2{1, 0};
    tri_2->mat = mat;
    world.add(tri_2);
    
    auto sun_tex = make_shared<image_texture>("/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/8k_sun.jpg");
    auto sun_mat = make_shared<diffuse_light>(sun_tex);
    auto sun = make_shared<sphere>(point3{-100, 120, -100}, 110, sun_mat);
    world.add(sun);
    
    // EVAs
    size_t eva_count = 1024;
    
    nodes = new bvh_instance<triangle>[eva_count];

    point3f origin{-100, 0, -100};
    for (int i  = 0; i < eva_count; i++) {
      auto displacement = random_in_unit_disk_flat(480);
      while (displacement.x() < -150 || displacement.z() > 150 || displacement.length() < 80) {
        displacement = random_in_unit_disk_flat(480);
      }
      
      auto dir = origin - displacement;
      auto angle = atan2(dir.x(), dir.z());

      nodes[i] = bvh_instance<triangle>(&mb);
      
      nodes[i].set_transform(
        mat4::Translate(origin + displacement)
        * mat4::RotateY(angle)
        * mat4::Scale(15.f)
      );
    }

    shared_ptr<tlas<triangle>> models{make_shared<tlas<triangle>>(nodes, eva_count)};
    models->build();

    world.add(models);
    
//     spheres
    sphere_list = new sphere[4096];
    shared_ptr<solid_color> albedo = make_shared<solid_color>(0.921, 0.094, 0.141);
    shared_ptr<pbr> pbr_mat = make_shared<pbr>();
    pbr_mat->albedo = albedo;
    pbr_mat->emit = albedo;
    for (int i = 0; i < 2048; i++) {
      vec3f center(-200.0 + random_double() * 1000.0, 20.0 * random_double(), 200.0 - random_double() * 1000.0);
      sphere_list[i] = sphere{center, random_double(0.01, 0.4), pbr_mat};
    }
    
    shared_ptr<solid_color> albedo_w = make_shared<solid_color>(2.0 * 0.921, 2.0 * 0.794, 2.0 * 0.841);
    shared_ptr<pbr> pbr_mat_w = make_shared<pbr>();
    pbr_mat_w->albedo = albedo_w;
    pbr_mat_w->emit = albedo_w;
    for (int i = 2048; i < 4096; i++) {
      vec3f center(-200.0 + random_double() * 1000.0, 50 + 20.0 * random_double(), -400.0 - random_double() * 1000.0);
      sphere_list[i] = sphere{center, random_double(0.01, 0.1), pbr_mat_w};
    }
    
    world.add(make_shared<bvh<sphere>>(sphere_list, 4096));
    
  } else {
     world.add(make_shared<bvh_instance<triangle>>(&mb));
  }
  
  camera cam;

  cam.aspect_ratio      = 16.0/9.0;
  if (render_many) {
    cam.image_width = 2560;
  } else {
    cam.image_width       = 640;
  }
  cam.samples_per_pixel = 250;
  cam.max_depth         = 50;
//  cam.background        = color(0.1, 0.1, 0.3);
  cam.background = color{0.003, 0.015, 0.074};
// cam.background        = color(0.5, 0.7, 1.0);

  cam.vfov     = 20;
  if (render_many) {
    cam.lookfrom = point3(100, 150, 250);
    cam.lookat = point3(0, 75, 0);
    cam.focus_dist = 200;
  } else {
    cam.lookfrom = point3(26, 7,50);
    cam.lookat = point3(-2, 7, 0);
    cam.focus_dist = 50;
  }
  cam.vup      = vec3(0,1,0);
  cam.vfov = 25;

  cam.defocus_angle = 0;
  
  if (out_path) {
    cam.out_path = out_path;
  }

  cam.render(world);
  
  delete[] nodes;
  delete[] sphere_list;
}


void robo_fight(const char* out_path) {
  hittable_list world;
  
  string light_platforms = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/light_platforms.obj";
  model light_platforms_m{light_platforms.c_str()};
  shared_ptr<pbr> mat1_pbr = std::dynamic_pointer_cast<pbr>(light_platforms_m.materials_loaded["Material.001"]);
  shared_ptr<pbr> mat2_pbr = std::dynamic_pointer_cast<pbr>(light_platforms_m.materials_loaded["Material.002"]);
  mat1_pbr->emission_intensity = 4;
  mat2_pbr->emission_intensity = 4;
  bvh<triangle> light_platforms_bvh{light_platforms_m.primitives, light_platforms_m.primitives_count};
  auto light_platforms_instance = make_shared<bvh_instance<triangle>>(&light_platforms_bvh);
  light_platforms_instance->set_transform(
    mat4::Translate(vec3f(-13, 0, -5))
    * mat4::RotateY(degrees_to_radians(5))
    * mat4::Scale(vec3f{0.8, 100, 0.8})
  );
  world.add(light_platforms_instance);
  

  string eva_path = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/eva/EVA_01.obj";
  model eva_m{eva_path.c_str()};
  bvh<triangle> eva_bvh{eva_m.primitives, eva_m.primitives_count};

  string robo_path = "/Users/senpie/Documents/projects/personal/tiny-ray-tracer/assets/robo/robo.obj";
  model robo_m{robo_path.c_str()};
  bvh<triangle> robo_bvh{robo_m.primitives, robo_m.primitives_count};
  
  sphere* spheres = new sphere[1025];
  int sphere_count = 0;
  
  auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
  spheres[sphere_count++] = sphere{point3(0,-1000,0), 1000, ground_material};
  
  for (int a = -16; a < 16; a++) {
    for (int b = -16; b < 16; b++) {
      auto choose_mat = random_double();
      point3 center(1.5f * a + 0.9*random_double(), 0.2, 1.5f * b + 0.9*random_double());

      if ((center - point3(4, 0.2, 0)).length() > 0.9) {
        if (choose_mat < 0.8) {
            // diffuse
            auto albedo = color::random() * color::random();
            shared_ptr<diffuse_light> sphere_material = make_shared<diffuse_light>(albedo);
            sphere_material->emission_intensity = 4;
            spheres[sphere_count++] = sphere{center, random_double(0.1, 0.2), sphere_material};
        } else if (choose_mat < 0.95) {
            // metal
            auto albedo = color::random(0.5, 1);
            shared_ptr<diffuse_light> sphere_material = make_shared<diffuse_light>(albedo);
            sphere_material->emission_intensity = 2;
            spheres[sphere_count++] = sphere{center, random_double(0.1, 0.2), sphere_material};
        } else {
          // glass
          shared_ptr<dielectric> sphere_material = make_shared<dielectric>(1.5);
          spheres[sphere_count++] = sphere{center, random_double(0.15, 0.3), sphere_material};
        }
      }
    }
  }

  world.add(make_shared<bvh<sphere>>(spheres, sphere_count));
  std::cout << "Rendering " << sphere_count << " spheres" << std::endl;
  
  bvh_instance<triangle>* robots = new bvh_instance<triangle>[1024];
  
  size_t robot_count = 0;
  for (int a = -16; a < 16; a++) {
    for (int b = -16; b < 16; b++) {
      bool robo_eva = random_double() < 0.5;
      auto &robo =  robo_eva ? robo_bvh : eva_bvh;
      float scale = robo_eva ? (3.f * 0.1f) / 4.f: (3.f * .8f) / 4.f;
      float rotation = random_double() * 2 * pi;
      robots[robot_count] = bvh_instance<triangle>(&robo);
      
      point3 center(1.5f * a + 0.9*random_double(), 0.2, 1.5f * b + 0.9*random_double());
      
      robots[robot_count].set_transform(
        mat4::Translate(center)
        * mat4::RotateY(rotation)
        * mat4::Scale(scale)
      );
      robot_count++;
    }
  }
  
  shared_ptr<tlas<triangle>> robots_tlas{make_shared<tlas<triangle>>(robots, robot_count)};
  robots_tlas->build();

  world.add(robots_tlas);

  camera cam;

  cam.aspect_ratio      = 16.0 / 9.0;
  cam.image_width       = 2560; // 1920
  cam.samples_per_pixel = 250; // 200
  cam.max_depth         = 50;
  cam.background        = color(0, 0, 0.01);

  cam.vfov     = 15;
  cam.lookfrom = point3(17,6,7);
  cam.lookat   = point3(0,1.9,0);
  cam.vup      = vec3(0,1,0);

  cam.defocus_angle = 0.3;
  cam.focus_dist    = 25.0;
  
  if (out_path) {
    cam.out_path = out_path;
  }

  cam.render(world);
  
  delete[] spheres;
  delete[] robots;
}

void any_model(const char* out_path, const char* model_path) {
  hittable_list world;
  
  model m{model_path};
  bvh<triangle> mb{m.primitives, m.primitives_count};
  auto instance = make_shared<bvh_instance<triangle>>(&mb);
  instance->set_transform(mat4::Translate(0.f, 1.f, 0.f) * mat4::RotateY(degrees_to_radians(-90)));
  world.add(instance);

  sphere* spheres = new sphere[1 + 24*24 + 3];
  int sphere_count = 0;
  
  auto ground_material = make_shared<lambertian>(color(0.5, 0.5, 0.5));
  world.add(make_shared<sphere>(point3(0,-1000,0), 1000, ground_material));
 
  camera cam;

  cam.aspect_ratio      = 16.0 / 9.0;
  cam.image_width       = 640;
  cam.samples_per_pixel = 100;
  cam.max_depth         = 50;
  cam.background        = color(0.5, 0.7, 1.0);

  cam.vfov     = 20;
  cam.lookfrom = point3(10,3,6);
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
  
  switch (8) {
    case 0:  simple_light(out_path);                break;
    case 1:  dragon(out_path, false);               break;
    case 2:  cow(out_path);                         break;
    case 3:  robot(out_path, true);                break;
//    case 4:  cubes(out_path);                       break;
//    case 5:  smiley(out_path);                      break;
    case 6:  any_model(out_path, model_path);       break;
    case 7:  eva(out_path, true);                  break;
    case 8:  robo_fight(out_path);                  break;
    default: final_scene(out_path, 1920,   250,  4); break;
  }
}
