#ifndef BVH_H
#define BVH_H

#include "rtweekend.h"
#include "aabb.h"
#include "hittable_list.h"

//__declspec(align(32))
class bvh_node {
public:
  aabb bbox;
  uint left_first, hittable_count;
  bool isLeaf() { return hittable_count > 0; }
};

vector<bvh_node> nodes;
vector<uint> primitives_idx;
uint root_node_idx = 0, nodes_used = 1;


void update_node_bounds(const hittable_list &world, uint node_idx) {
  bvh_node& node = nodes[node_idx];
  node.bbox = aabb{}; // remove this line
  for (uint first = node.left_first, i = 0; i < node.hittable_count; i++) {
    uint leaf_idx = primitives_idx[first + i];
    auto primitive_bbox = world.objects[leaf_idx]->bounding_box();
    node.bbox = aabb(node.bbox, primitive_bbox);
  }
}

void subdivide(const hittable_list &world, uint node_idx) {
  // terminate recursion
  bvh_node& node = nodes[node_idx];
  if (node.hittable_count <= 2) return;
  // determine split axis and position
  vec3 extent{
    node.bbox.x.size(),
    node.bbox.y.size(),
    node.bbox.z.size(),
  };
  int axis = 0;
  if (extent.y() > extent.x()) axis = 1;
  if (extent.z() > extent[axis]) axis = 2;
  float split_pos = node.bbox.axis(axis).min + extent[axis] * 0.5;
  // in-place partition
  int i = node.left_first;
  int j = i + node.hittable_count - 1;
  while (i <= j) {
    if (world.objects[primitives_idx[i]]->centroid()[axis] < split_pos)
      i++;
    else
      std::swap(primitives_idx[i], primitives_idx[j--]);
  }
  // abort split if one of the sides is empty
  int left_count = i - node.left_first;
  if (left_count == 0 || left_count == node.hittable_count) return;
  // create child nodes
  int left_child_idx = nodes_used++;
  int right_child_idx = nodes_used++;
  nodes[left_child_idx].left_first = node.left_first;
  nodes[left_child_idx].hittable_count = left_count;
  nodes[right_child_idx].left_first = i;
  nodes[right_child_idx].hittable_count = node.hittable_count - left_count;
  node.left_first = left_child_idx;
  node.hittable_count = 0;
  update_node_bounds(world, left_child_idx);
  update_node_bounds(world, right_child_idx);
  // recurse
  subdivide(world, left_child_idx);
  subdivide(world, right_child_idx);
}

bool intersect_bvh(const hittable_list &world, const ray& r, interval ray_t,  hit_record& rec, const uint node_idx) {
    bvh_node& node = nodes[node_idx];
    if (!node.bbox.hit(r, ray_t)) return false;
    if (node.isLeaf()) {
      hit_record temp_rec;
      bool hit_anything = false;
      auto closest_so_far = ray_t.max;
 
      for (uint i = 0; i < node.hittable_count; i++ ) {
        uint leaf_idx = primitives_idx[node.left_first + i];
        auto leaf_hittable = world.objects[leaf_idx];
        if (leaf_hittable->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
        }
      }
      
      return hit_anything;
    } else {
        bool hit_left = intersect_bvh(world, r, ray_t, rec, node.left_first);
        bool hit_right = intersect_bvh(world, r, ray_t, rec, node.left_first + 1);
        return hit_left || hit_right;
    }
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
  root.left_first = 0, root.hittable_count = static_cast<uint>(world.objects.size());
  update_node_bounds(world, root_node_idx);
  //subdivide recursively
  subdivide(world, root_node_idx);
}

#endif
