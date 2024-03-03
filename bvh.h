#ifndef BVH_H
#define BVH_H

#include "rtweekend.h"

#include "hittable.h"
#include "hittable_list.h"

#include <stdlib.h>
#include <algorithm>

#define BINS 128

struct bin { aabb bounds; int primitives_count = 0; };

struct bvh_node // in total 32 bytes
{
  aabb bbox;
  uint left_first; // 4
  uint primitives_count; // 4
	bool is_leaf() const { return primitives_count > 0; }
	float calculate_node_cost()
	{
    vec3f e = bbox.bmax - bbox.bmin;
    return (e.x() * e.y() + e.y() * e.z() + e.z() * e.x()) * primitives_count;
	}
};

template<typename T>
class bvh : public hittable
{
public:
  bvh() = default;
  bvh(T* _primtives, int N) {
    primitives_count = N;
    primitives = _primtives;
    bvh_nodes = (bvh_node*) aligned_alloc(64, sizeof(bvh_node) * N * 2);
    primitives_idx = new uint[N];
    build();
  }
 
  ~bvh() {
    free(bvh_nodes);
    delete[] primitives_idx;
    nodes_used = 0;
  }
 
  void build() {
    // reset node pool
    nodes_used = 2;
    // populate triangle index array
    for (uint i = 0; i < primitives_count; i++)
      primitives_idx[i] = i;
      
    // assign all triangles to root node
    bvh_node& root = bvh_nodes[0];
    root.left_first = 0, root.primitives_count = primitives_count;
    update_node_bounds(0);
    // subdivide recursively
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    auto t1 = high_resolution_clock::now(); // measure render time
    subdivide(0);
    auto t2 = high_resolution_clock::now();
    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> ms_double = t2 - t1;
    std::clog << "BVH construction time: " << ms_double.count() << "ms" << std::endl;
  }
//  void refit() { /* ... */ }
//  void set_transform(mat4& transform) { /* ... */ }
  bool hit(const ray& r, interval ray_t, hit_record& rec) const override
  {
    const bvh_node *node = &bvh_nodes[0], *stack[64];
    uint stack_ptr = 0;
    auto closest_so_far = ray_t.max;
    bool hit_anything = false;
 
    while (true)
    {
      if (node->is_leaf())
      {
        hit_record temp_rec;
 
        for (size_t i = 0; i < node->primitives_count; i++) {
          T& primitive = primitives[primitives_idx[node->left_first + i]];
          if (primitive.hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
            hit_anything = true;
            closest_so_far = temp_rec.t;
            rec = temp_rec;
          }
        }
        if (stack_ptr == 0)
          return hit_anything;
        else node = stack[--stack_ptr];
      } else {
        const bvh_node* child1 = &bvh_nodes[node->left_first];
        interval i_a = interval(ray_t.min, closest_so_far);
        double dist1 = child1->bbox.hit(r, i_a);
 
        const bvh_node* child2 = &bvh_nodes[node->left_first + 1];
        interval i_b = interval(ray_t.min, closest_so_far);
        double dist2 = child2->bbox.hit(r, i_b);
        if (dist1 > dist2) { std::swap( dist1, dist2 ); std::swap( child1, child2 ); }
        if (dist1 == infinity)
        {
          if (stack_ptr == 0)
            break;
          else
            node = stack[--stack_ptr];
        }
        else
        {
          node = child1;
          if (dist2 != infinity)
            stack[stack_ptr++] = child2;
        }
      }
    }
    return hit_anything;
  }

  aabb bounding_box() const override {
    if (nodes_used > 0) {
      bvh_node& node = bvh_nodes[0];
      return aabb();
    }
  }
    
  point3f centroid() const override { return center; }
 
private:
  void subdivide(uint node_idx) {
    // terminate recursion
    bvh_node& node = bvh_nodes[node_idx];
    // determine split axis using SAH
    int axis;
    double split_pos;
    double split_cost = find_best_split_plane(node, axis, split_pos);
    double nosplit_cost = node.calculate_node_cost();
    if (split_cost >= nosplit_cost) return;
    // in-place partition
    int i = node.left_first;
    int j = i + node.primitives_count - 1;
    while (i <= j)
    {
      if (primitives[primitives_idx[i]].centroid()[axis] < split_pos)
        i++;
      else
        std::swap(primitives_idx[i], primitives_idx[j--]);
    }
    // abort split if one of the sides is empty
    int left_count = i - node.left_first;
    if (left_count == 0 || left_count == node.primitives_count) return;
    // create child nodes
    int left_child_idx = nodes_used++;
    int right_child_idx = nodes_used++;
    bvh_nodes[left_child_idx].left_first = node.left_first;
    bvh_nodes[left_child_idx].primitives_count = left_count;
    bvh_nodes[right_child_idx].left_first = i;
    bvh_nodes[right_child_idx].primitives_count = node.primitives_count - left_count;
    node.left_first = left_child_idx;
    node.primitives_count = 0;
    update_node_bounds(left_child_idx);
    update_node_bounds(right_child_idx);
    // recurse
    subdivide(left_child_idx);
    subdivide(right_child_idx);
  }
 
  void update_node_bounds(uint node_idx) {
    bvh_node& node = bvh_nodes[node_idx];
    node.bbox.bmin = vec3f(infinity, infinity, infinity);
    node.bbox.bmax = vec3f(-infinity, -infinity, -infinity);
    for (uint first = node.left_first, i = 0; i < node.primitives_count; i++)
    {
      uint leaf_prim_idx = primitives_idx[first + i];
      T& leaf_prim = primitives[leaf_prim_idx];
      node.bbox.bmin = fminf( node.bbox.bmin, leaf_prim.bounding_box().bmin );
      node.bbox.bmax = fmaxf( node.bbox.bmax, leaf_prim.bounding_box().bmax );
    }
    center = (bbox.bmax + bbox.bmin) / 2.0f;
  }

  double find_best_split_plane(bvh_node& node, int& axis, double& split_pos) {
    double best_cost = infinity;
    for (int a = 0; a < 3; a++) {
      double bounds_min = infinity, bounds_max = -infinity;
      for (int i = 0; i < node.primitives_count; i++) {
        T& primitive = primitives[primitives_idx[node.left_first + i]];
        bounds_min = fmin(bounds_min, primitive.centroid()[a]);
        bounds_max = fmax(bounds_max, primitive.centroid()[a]);
      }
      if (bounds_min == bounds_max) continue;
      // populate the bins
      bin bin[BINS];
      double scale = BINS / (bounds_max - bounds_min);
      for (int i = 0; i < node.primitives_count; i++) {
          T& primitive = primitives[primitives_idx[node.left_first + i]];
          int bin_idx = fmin(BINS - 1, (int)((primitive.centroid()[a] - bounds_min) * scale));
          bin[bin_idx].primitives_count++;
          bin[bin_idx].bounds = aabb(bin[bin_idx].bounds, primitive.bounding_box());
      }
      // gather data for the 7 planes between the 8 bins
      double left_area[BINS - 1], right_area[BINS - 1];
      int left_count[BINS - 1], right_count[BINS - 1];
      aabb left_box = aabb(), right_box = aabb();
      int left_sum = 0, right_sum = 0;
      for (int i = 0; i < BINS - 1; i++) {
        left_sum += bin[i].primitives_count;
        left_count[i] = left_sum;
        left_box = aabb(left_box, bin[i].bounds);
        left_area[i] = left_box.area();
        right_sum += bin[BINS - 1 - i].primitives_count;
        right_count[BINS - 2 - i] = right_sum;
        right_box = aabb(right_box, bin[BINS - 1 - i].bounds);
        right_area[BINS - 2 - i] = right_box.area();
      }
      // calculate SAH cost for the 7 planes
      scale = (bounds_max - bounds_min) / BINS;
      for (int i = 0; i < BINS - 1; i++) {
        double plane_cost = left_count[i] * left_area[i] + right_count[i] * right_area[i];
        if (plane_cost < best_cost) {
          axis = a, split_pos = bounds_min + scale * (i + 1), best_cost = plane_cost;
        }
      }
    }
    return best_cost;
  }

  bvh_node* bvh_nodes = nullptr;
  T* primitives = nullptr;
  uint* primitives_idx = nullptr;
  uint nodes_used, primitives_count;
  aabb bbox;
  point3f center;
};

#endif
