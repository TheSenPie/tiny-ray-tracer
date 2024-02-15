#ifndef BVH_H
#define BVH_H

#include "rtweekend.h"

#include "hittable.h"
#include "hittable_list.h"

#include <algorithm>

class bvh_node : public hittable {
public:
  bvh_node(const hittable_list& list) : bvh_node{list.objects, 0, list.objects.size()} {}
  
  bvh_node(vector<shared_ptr<hittable>> src_objects, size_t start, size_t end) {
    primitives =  src_objects; // Create a modifiable array of the source scene objects
 
    left = right = nullptr;
    first_primitive = start, primitive_count = end - start;
    update_bounds();
    subdivide();

//    int axis = random_int(0,2);
//    auto comparator = (axis == 0) ? box_x_compare
//                    : (axis == 1) ? box_y_compare
//                                  : box_z_compare;
//
//    size_t object_span = end - start;
//
//    if (object_span == 1) {
//      left = right = objects[start];
//    } else if (object_span == 2) {
//      if (comparator(objects[start], objects[start+1])) {
//        left = objects[start];
//        right = objects[start+1];
//      } else {
//        left = objects[start+1];
//        right = objects[start];
//      }
//    } else {
//      std::sort(objects.begin() + start, objects.begin() + end, comparator);
//
//      auto mid = start + object_span/2;
//      left = make_shared<bvh_node>(objects, start, mid);
//      right = make_shared<bvh_node>(objects, mid, end);
//    }
//
//    bbox = aabb(left->bounding_box(), right->bounding_box());
  }

  bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
    if (!bbox.hit(r, ray_t))
      return false;

    if (primitive_count > 0) {
      bool hit_anything = false;
      for (size_t i = 0; i < primitive_count; i++ )
        hit_anything = hit_anything || (primitives)[first_primitive + i]->hit(r, ray_t, rec);
      return hit_anything;
    } else {
      bool hit_left = left->hit(r, ray_t, rec);
      bool hit_right = right->hit(r, interval(ray_t.min, hit_left ? rec.t : ray_t.max), rec);
      return hit_left || hit_right;
    }
  }

  aabb bounding_box() const override { return bbox; }
    
  point3 centroid() const override { return center; }

private:
  shared_ptr<hittable> left;
  shared_ptr<hittable> right;
  aabb bbox;
  point3 center;
  size_t first_primitive, primitive_count;
 
  vector<shared_ptr<hittable>> primitives;
 
  void update_bounds() {
    bbox = aabb();
    for (size_t i = 0; i < primitive_count; i++) {
      bbox = aabb(bbox, (primitives)[first_primitive + i]->bounding_box());
    }
    center = vec3{
      (bbox.x.max + bbox.x.min) / 2.0,
      (bbox.y.max + bbox.y.min) / 2.0,
      (bbox.z.max + bbox.z.min) / 2.0
    };
  }
  
  void subdivide() {
    // terminate recursion
    if (primitive_count <= 2) return;
    // determine split axis and position
    vec3 extent{
      bbox.x.size(),
      bbox.y.size(),
      bbox.z.size()
    };
    int axis = 0;
    if (extent.y() > extent.x()) axis = 1;
    if (extent.z() > extent[axis]) axis = 2;
    double split_pos = bbox.axis(axis).min + extent[axis] * 0.5;
    // in-place partition
    size_t i = first_primitive;
    size_t j = i + primitive_count - 1;
    while (i <= j) {
      if ((primitives)[i]->centroid()[axis] < split_pos)
        i++;
      else
        (primitives)[i].swap((primitives)[j--]);
    }
    // abort split if one of the sides is empty
    size_t left_count = i - first_primitive;
    if (left_count == 0 || left_count == primitive_count) return;
    // create child nodes
    left = make_shared<bvh_node>(primitives, first_primitive,first_primitive + left_count);
    right = make_shared<bvh_node>(primitives, i, i + primitive_count - left_count);
    primitive_count = 0;
  }
  
  static bool box_compare(
    const shared_ptr<hittable> a, const shared_ptr<hittable> b, int axis_index
  ) {
    return a->bounding_box().axis(axis_index).min < b->bounding_box().axis(axis_index).min;
  }

  static bool box_x_compare (const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return box_compare(a, b, 0);
  }

  static bool box_y_compare (const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return box_compare(a, b, 1);
  }

  static bool box_z_compare (const shared_ptr<hittable> a, const shared_ptr<hittable> b) {
    return box_compare(a, b, 2);
  }
};

#endif
