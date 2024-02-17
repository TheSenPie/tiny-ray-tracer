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
    const bvh_node *curr = this, *stack[64];
    uint stack_ptr = 0;
    auto closest_so_far = ray_t.max;
    bool hit_anything = false;

    while (true) {
      if (curr->primitive_count > 0) // is leaf
      {
        hit_record temp_rec;
 
        for (size_t i = 0; i < curr->primitive_count; i++) {
          hittable& primitive = *(curr->primitives[curr->first_primitive + i]);
          if (primitive.hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
          }
        }
        if (stack_ptr == 0)
          return hit_anything;
        else curr = stack[--stack_ptr];
      }
      else
      {
        const bvh_node* child1 = curr->left.get();
        interval i_a = interval(ray_t.min, closest_so_far);
        double dist1 = child1->bbox.hit(r, i_a);
        
        const bvh_node* child2 = curr->right.get();
        interval i_b = interval(ray_t.min, closest_so_far);
        double dist2 = child2->bbox.hit(r, i_b);
        if (dist1 > dist2) { std::swap(dist1, dist2); std::swap(child1, child2); }
        if (dist1 == infinity)
        {
          if (stack_ptr == 0)
            break;
          else
            curr = stack[--stack_ptr];
        }
        else
        {
          curr = child1;
          if (dist2 != infinity)
          stack[stack_ptr++] = child2;
        }
      }
    }
    return hit_anything;
  }

  aabb bounding_box() const override { return bbox; }
    
  point3 centroid() const override { return center; }

private:
  shared_ptr<bvh_node> left;
  shared_ptr<bvh_node> right;
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
    // determine split axis using SAH
    int best_axis = -1;
    double best_pos = 0, best_cost = infinity;
    for (int axis = 0; axis < 3; axis++) for (int i = 0; i < primitive_count; i++) {
      hittable &primitive = *(primitives[first_primitive + i]);
      double candidate_pos = primitive.centroid()[axis];
      double cost = evaluate_SAH(*this, axis, candidate_pos);
      if (cost < best_cost)
        best_pos = candidate_pos, best_axis = axis, best_cost = cost;
    }
    int axis = best_axis;
    double split_pos = best_pos;
    vec3 extent{
      bbox.x.size(),
      bbox.y.size(),
      bbox.z.size()
    };
    double parent_area = extent.x() * extent.y() + extent.y() * extent.z() + extent.z() * extent.x();
    float parent_cost = primitive_count * parent_area;
    if (best_cost >= parent_cost) return;
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
  
  double evaluate_SAH(const bvh_node& node, int axis, double pos) {
    // determine triangle counts and bounds for this split candidate
    aabb left_box = aabb(), right_box = aabb();
    int left_count = 0, right_count = 0;
    for (int i = 0; i < node.primitive_count; i++) {
      hittable &primitive = *(node.primitives[first_primitive + i]);
      if (primitive.centroid()[axis] < pos) {
        left_count++;
        left_box = aabb(left_box, primitive.bounding_box());
      } else {
        right_count++;
        right_box = aabb(right_box, primitive.bounding_box());
      }
    }
    double cost = left_count * left_box.area() + right_count * right_box.area();
    return cost > 0 ? cost : infinity;
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
