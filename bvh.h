#ifndef BVH_H
#define BVH_H

#include "rtweekend.h"
#include "aabb.h"
#include "hittable_list.h"

//__declspec(align(32))
class bvh_node {
public:
  aabb bbox{};
  uint left_first, primitive_count;
  bool isLeaf() { return primitive_count > 0; }
};

vector<bvh_node> nodes;
vector<uint> primitives_idx;
uint root_node_idx = 0, nodes_used = 1;


void update_node_bounds(const hittable_list &world, uint node_idx) {
  bvh_node& node = nodes[node_idx];
  node.bbox = aabb{}; // remove this line
  for (uint first = node.left_first, i = 0; i < node.primitive_count; i++) {
    uint leaf_idx = primitives_idx[first + i];
    auto primitive_bbox = world.objects[leaf_idx]->bounding_box();
    node.bbox = aabb(node.bbox, primitive_bbox);
  }
}

double evaluate_SAH(const hittable_list &world, bvh_node& node, int axis, float pos) {
    // determine triangle counts and bounds for this split candidate
    aabb left_box{}, right_box{};
    int left_count = 0, right_count = 0;
    for(uint i = 0; i < node.primitive_count; i++ )
    {
        auto hittable = world.objects[primitives_idx[node.left_first + i]];
        if (hittable->centroid()[axis] < pos) {
            left_count++;
            left_box = aabb{left_box, hittable->bounding_box()};
        } else {
            right_count++;
            right_box = aabb{right_box, hittable->bounding_box()};
        }
    }
    double cost{left_count * left_box.area() + right_count * right_box.area()};
    return cost > 0 ? cost : infinity;
}

void subdivide(const hittable_list &world, uint node_idx) {
  // terminate recursion
  bvh_node& node = nodes[node_idx];
  // determine split axis using SAH
  int best_axis{-1};
  double best_pos{0}, best_cost{infinity};
  for (int axis = 0; axis < 3; axis++) for (uint i = 0; i < node.primitive_count; i++) {
    auto hittable = world.objects[primitives_idx[node.left_first + i]];
    float candidate_pos = hittable->centroid()[axis];
    float cost = evaluate_SAH(world, node, axis, candidate_pos);
    if (cost < best_cost)
        best_pos = candidate_pos, best_axis = axis, best_cost = cost;
  }
  int axis = best_axis;
  float split_pos = best_pos;
  vec3 e {
    node.bbox.x.size(),
    node.bbox.y.size(),
    node.bbox.z.size(),
  };
  float parent_area = e.x() * e.y() + e.y() * e.z() + e.z() * e.x();
  float parent_cost = node.primitive_count * parent_area;
  if (best_pos >= parent_cost) return;
  // in-place partition
  int i = node.left_first;
  int j = i + node.primitive_count - 1;
  while (i <= j) {
    if (world.objects[primitives_idx[i]]->centroid()[axis] < split_pos)
      i++;
    else
      std::swap(primitives_idx[i], primitives_idx[j--]);
  }
  // abort split if one of the sides is empty
  int left_count = i - node.left_first;
  if (left_count == 0 || left_count == node.primitive_count) return;
  // create child nodes
  int left_child_idx = nodes_used++;
  int right_child_idx = nodes_used++;
  nodes[left_child_idx].left_first = node.left_first;
  nodes[left_child_idx].primitive_count = left_count;
  nodes[right_child_idx].left_first = i;
  nodes[right_child_idx].primitive_count = node.primitive_count - left_count;
  node.left_first = left_child_idx;
  node.primitive_count = 0;
  update_node_bounds(world, left_child_idx);
  update_node_bounds(world, right_child_idx);
  // recurse
  subdivide(world, left_child_idx);
  subdivide(world, right_child_idx);
}

//bool intersect_bvh(const hittable_list &world, const ray& r, interval ray_t,  hit_record& rec, const uint node_idx) {
//    bvh_node& node = nodes[node_idx];
//    if (!node.bbox.hit(r, ray_t)) return false;
//    if (node.isLeaf()) {
//      hit_record temp_rec;
//      bool hit_anything = false;
//      auto closest_so_far = ray_t.max;
// 
//      for (uint i = 0; i < node.primitive_count; i++ ) {
//        uint leaf_idx = primitives_idx[node.left_first + i];
//        auto leaf_hittable = world.objects[leaf_idx];
//        if (leaf_hittable->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
//            hit_anything = true;
//            closest_so_far = temp_rec.t;
//            rec = temp_rec;
//        }
//      }
//      
//      return hit_anything;
//    } else {
//        bool hit_left = intersect_bvh(world, r, ray_t, rec, node.left_first);
//        bool hit_right = intersect_bvh(world, r, ray_t, rec, node.left_first + 1);
//        return hit_left || hit_right;
//    }
//}

bool intersect_bvh(const hittable_list &world, const ray& r, interval ray_t,  hit_record& rec) {
    bvh_node* node = &nodes[root_node_idx], *stack[64];
    uint stack_ptr = 0;
 
    hit_record temp_rec;
    bool hit_anything = false;
    auto closest_so_far = ray_t.max;

    while (1) {
      if (node->isLeaf()) {
        for (uint i = 0; i < node->primitive_count; i++) {
          uint leaf_idx = primitives_idx[node->left_first + i];
          auto leaf_hittable = world.objects[leaf_idx];
          if (leaf_hittable->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
          }
        }
        if (stack_ptr == 0) break; else node = stack[--stack_ptr];

        continue;
      }
      bvh_node* child1 = &nodes[node->left_first];
      bvh_node* child2 = &nodes[node->left_first + 1];
      double dist1 = child1->bbox.hit(r, ray_t);
      double dist2 = child2->bbox.hit(r, ray_t);
      if (dist1 > dist2) { std::swap( dist1, dist2 ); std::swap( child1, child2 ); }
      if (dist1 == infinity)
      {
          if (stack_ptr == 0) break; else node = stack[--stack_ptr];
      }
      else
      {
          node = child1;
          if (dist2 != infinity) stack[stack_ptr++] = child2;
      }
    }
    return hit_anything;
}

void build_bvh(const hittable_list &world) {
  primitives_idx = vector<uint>(world.objects.size());
  nodes = vector<bvh_node>(2 * world.objects.size());
  
  // populate triangle index array
  for (int i = 0; i < world.objects.size(); i++) primitives_idx[i] = i;
 
//  do not need this step, already calculated
//  // calculate triangle centroids for partitioning
//	for (int i = 0; i < N; i++)
//		tri[i].centroid = (tri[i].vertex0 + tri[i].vertex1 + tri[i].vertex2) * 0.3333f;
  
  bvh_node& root = nodes[root_node_idx];
  root.left_first = 0, root.primitive_count = static_cast<uint>(world.objects.size());
  update_node_bounds(world, root_node_idx);
  //subdivide recursively
  subdivide(world, root_node_idx);
}

#endif
