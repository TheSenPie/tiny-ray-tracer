#ifndef CAMERA_H
#define CAMERA_H

#include "rtweekend.h"

#include "color.h"
#include "hittable.h"
#include "material.h"

#include <iostream>
#include <vector>
#include <future>
#include <functional>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

class camera {
public:
  double aspect_ratio{ 1.0 };   // Ratio of image width over height
  int image_width{ 100 };       // Rendered image width in pixel count
  int samples_per_pixel{ 10 };  // Count of random samples for each pixel
  int max_depth{ 10 };          // Maximum number of ray bounces into scene
  color background;             // Scene background color
  
  double vfov{90};  // Vertical view angle (field of view)
  
  point3 lookfrom = point3{0,0,-1};  // Point camera is looking from
  point3 lookat   = point3{0,0,0};   // Point camera is looking at
  vec3   vup      = vec3{0,1,0};     // Camera-relative "up" direction
  
  double defocus_angle{0};   // Variation angle of rays through each pixel
  double focus_dist{10};     // Distance from camera lookfrom to plane of perfect focus
  
  const char* out_path;
  
  void render(const hittable& world) {
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    auto t1 = high_resolution_clock::now(); // measure render time
    
    initialize();
    
    std::vector<std::future<std::vector<color>>> buffers;
    
    for (auto i = samples_per_pixel; i > 0; i--) {
      buffers.push_back(std::async(std::bind(&camera::compute, this, std::cref(world))));
    }
    
    auto samples = wait_for_all(buffers);
 
    std::vector<color> buffer(image_width * image_height);
    for (auto& sample : samples) {
      for (int p_sample = 0; p_sample < image_width * image_height; p_sample++) {
        buffer[p_sample] += sample[p_sample];
      }
    }
    
    auto t2 = high_resolution_clock::now();
    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> ms_double = t2 - t1;
    std::clog << "Render time: " << ms_double.count() << "ms" << std::endl;
    
    if (!out_path) { // Write to standart output.
      std::cout << "P3\n" << image_width << ' ' << image_height << "\n255\n";

      for (int j = 0; j < image_height; j++) {
        for (int i = 0; i < image_width; i++) {
          write_color(std::cout, buffer[i * image_height + j], samples_per_pixel);
        }
      }
    } else { // Write to png.
      unsigned char* data = (unsigned char*) malloc(sizeof(unsigned char) * 3 * image_width * image_height);
      int idx = 0;
      for (int j = 0; j < image_height; j++) {
        for (int i = 0; i < image_width; i++) {
          unsigned char r,g,b;
          write_color(r, g, b, buffer[i * image_height + j], samples_per_pixel);
          data[idx++] = r;
          data[idx++] = g;
          data[idx++] = b;
        }
      }
      stbi_write_png(out_path, image_width, image_height, 3, data, 3 * image_width);
      free(data);
    }
    
  }
  
  std::vector<color> compute(const hittable& world) {
    std::vector<color> buffer(image_width * image_height);
    
    for (int tile = 0; tile < image_width * image_height / 64; tile++) {
      for (int v = 0; v < 8; v++) for (int u = 0; u < 8; u++) {
        int x = tile % (image_width / 8), y = tile / (image_width / 8);
        color pixel_color{0, 0, 0};
        
        int a = x * 8 + u;
        int b = y * 8 + v;
        
        ray r = get_ray(a, b);
        pixel_color += ray_color(r, max_depth, world);
 
        buffer[a * image_height + b] = pixel_color;
      }
    }
 
    return buffer;
  }
  
private:
  int image_height;       // Rendered image height
  point3 center;          // Camera center
    point3 pixel00_loc;   // Location of pixel 0, 0
    vec3 pixel_delta_u;   // Offset to the pixel to the right
    vec3 pixel_delta_v;   // Offset to the pixel below
    vec3 u, v, w;         // Camera frame basis vectors
    vec3 defocus_disk_u;  // Defocus disk horiznotal radius
    vec3 defocus_disk_v;  // Defocus disk vertical radius
  
  void initialize() {
    image_height = static_cast<int>(image_width / aspect_ratio);
    image_height = (image_height < 1) ? 1 : image_height;
    
    center = lookfrom;
    
    // Determine viewport dimensions.
    auto theta = degrees_to_radians(vfov);
    auto h = tan(theta/2);
    auto viewport_height = 2 * h * focus_dist;
    auto viewport_width = viewport_height * (static_cast<double>(image_width) / image_height);
    
     // Calculate the u,v,w unit basis vectors for the camera coordinate frame.
     w = unit_vector(lookfrom - lookat);
     u = unit_vector(cross(vup, w));
     v = cross(w, u);
     
     // Calculate the vectors across the horizontal and down the vertical viewport edges.
     vec3 viewport_u = viewport_width * u;    // Vector across viewport horizontal edge
     vec3 viewport_v = viewport_height * -v;  // Vector down viewport vertical edge
    
    // Calculate the horizontal and vertical delta vectors from the pixel to pixel.
    pixel_delta_u = viewport_u / image_width;
    pixel_delta_v = viewport_v / image_height;
    
    // Calculate the location of the upper left pixel.
    auto viewport_upper_left = center - (focus_dist * w) - viewport_u/2 - viewport_v/2;
    pixel00_loc = viewport_upper_left + 0.5 * (pixel_delta_u + pixel_delta_v);
    
    // Calculate the camera defocus disk basis vectors.
    auto defocus_radius = focus_dist * tan(degrees_to_radians(defocus_angle / 2));
    defocus_disk_u = defocus_radius * u;
    defocus_disk_v = defocus_radius * v;
  }
  
  ray get_ray(int i, int j) const {
    // Get a randomly-sampled camera ray for the pixel at location i,j originating from
    // the camera defocus disk.
    
    auto pixel_center = pixel00_loc + (i * pixel_delta_u) + (j * pixel_delta_v);
    auto pixel_sample = pixel_center + pixel_sample_square();
 
    auto ray_origin = (defocus_angle <= 0) ? center : defocus_disk_sample();
    auto ray_directioin = pixel_sample - ray_origin;
 
    return ray{ray_origin, ray_directioin};
  }
  
  vec3 pixel_sample_square() const {
    // Returns a random point in the square surrounding a pixel at the origin.
    auto px = -0.5 + random_double();
    auto py = -0.5 + random_double();
    return (px * pixel_delta_u) + (py * pixel_delta_v);
  }
  
  point3 defocus_disk_sample() const {
    // Returns a random point in the camera defocus disk.
    auto p = random_in_unit_disk();
    return center + (p[0] * defocus_disk_u) + (p[1] * defocus_disk_v);
  }
    
  color ray_color(const ray& r, int depth, const hittable& world) {
    hit_record rec;
    
    // If we've exceeded the ray bounce limit, no more light is gathered.
    if (depth <= 0)
        return color(0, 0, 0);
    
    // If the ray hits nothing, return the background color.
    if (!world.hit(r, interval(0.001, infinity), rec))
      return background;
            
    ray scattered;
    color attenuation;
    color color_from_emission = rec.mat->emitted(rec.u, rec.v, rec.p);
    
    if (!rec.mat->scatter(r, rec, attenuation, scattered))
      return color_from_emission;

    color color_from_scatter = attenuation * ray_color(scattered, depth-1, world);

    return color_from_emission + color_from_scatter;
  }
};

#endif
