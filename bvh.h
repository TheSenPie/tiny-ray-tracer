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
  aabb bbox; // 24
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

  bvh(bvh&& a)
    : inv_transform{a.inv_transform},
      bounds{a.bounds},
      bvh_nodes{a.bvh_nodes},
      primitives{a.primitives},
      primitives_idx(a.primitives_idx),
      nodes_used{a.nodes_used}, primitives_count{a.primitives_count},
      center{a.center} {
        bvh_nodes = nullptr;
        primitives_idx = nullptr;
        primitives = nullptr;
        nodes_used = 0;
        primitives_count = 0;
  }
  
  bvh<T>& operator=(bvh&& a) {
    std::swap(bvh_nodes, a.bvh_nodes);
    std::swap(primitives_idx, a.primitives_idx);
    std::swap(primitives, a.primitives);
    return *this;
  }
 
  ~bvh() {
    if (bvh_nodes != nullptr) {
      free(bvh_nodes);
      bvh_nodes = nullptr;
    }
    delete[] primitives_idx;
    primitives_idx = nullptr;
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
    using std::chrono::high_resolution_clock;
    using std::chrono::duration_cast;
    using std::chrono::duration;
    using std::chrono::milliseconds;
    auto t1 = high_resolution_clock::now(); // measure render time
    update_node_bounds(0);
    // subdivide recursively
    subdivide(0);
    auto t2 = high_resolution_clock::now();
    /* Getting number of milliseconds as a double. */
    duration<double, std::milli> ms_double = t2 - t1;
    std::clog << "BVH construction time: " << ms_double.count() << "ms" << std::endl;

    bounds = aabb(bvh_nodes[0].bbox.bmin, bvh_nodes[0].bbox.bmax);
    center = (bounds.bmax + bounds.bmin) / 2.0f;
  }
  
  void refit() {
    for (int i = nodes_used - 1; i >= 0; i--) if (i != 1)
    {
      bvh_node& node = bvh_nodes[i];
      if (node.is_leaf()) {
        // leaf node: adjust bounds to contained triangles
        update_node_bounds(i);
        continue;
      }
      // interior node: adjust bounds to child node bounds
      bvh_node& left_child = bvh_nodes[node.left_first];
      bvh_node& right_child = bvh_nodes[node.left_first + 1];
      node.bbox.bmin = fminf( left_child.bbox.bmin, right_child.bbox.bmin );
      node.bbox.bmax = fmaxf( left_child.bbox.bmax, right_child.bbox.bmax );
    }
  }
  


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

  aabb bounding_box() const override { return bounds; }
    
  point3f centroid() const override { return center; }
 
private:
  mat4 inv_transform; // inverse transform
  aabb bounds; // in world space
  
  bvh_node* bvh_nodes = nullptr;
  T* primitives = nullptr;
  uint* primitives_idx = nullptr;
  uint nodes_used, primitives_count;
  point3f center;

 
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
};

// instance of a BVH, with transform and world bounds
template<typename T>
class bvh_instance : public hittable
{
public:
  bvh_instance() = default;
  bvh_instance(bvh<T>* blas) : bvh(blas) { set_transform( mat4() ); }
  ~bvh_instance() {
    bvh = nullptr;
  }

   void set_transform(const mat4& _tranform)
  {
    transform = _tranform;
    inv_transform = transform.Inverted();
    // calculate world-space bounds using the new matrix
     
    point3 min( infinity,  infinity,  infinity);
    point3 max(-infinity, -infinity, -infinity);

    for (int i = 0; i < 2; i++) {
      for (int j = 0; j < 2; j++) {
        for (int k = 0; k < 2; k++) {
          vec3f tester{
            i*bvh->bounding_box().bmax.x() + (1-i)*bvh->bounding_box().bmin.x(),
            j*bvh->bounding_box().bmax.y() + (1-j)*bvh->bounding_box().bmin.y(),
            k*bvh->bounding_box().bmax.z() + (1-k)*bvh->bounding_box().bmin.z()
          };

          TransformPosition(tester, transform);

          for (int c = 0; c < 3; c++) {
            min[c] = fmin(min[c], tester[c]);
            max[c] = fmax(max[c], tester[c]);
          }
        }
      }
    }
    bounds = aabb(min, max);
    center = (bounds.bmax + bounds.bmin) / 2.0f;
  }

  bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
    // Change the ray from world space to object space
    auto origin = TransformPosition( r.origin(), inv_transform );
    auto direction = TransformVector( r.direction(), inv_transform );
    ray rotated_r(origin, direction);
    
    // Determine where (if any) an intersection occurs in object space
    if (!bvh->hit(rotated_r, ray_t, rec))
        return false;
        
    // Change the intersection point from object space to world space
    auto p = TransformPosition(rec.p, transform);
    
    // Change the normal from object space to world space
    auto normal = TransformVector(rec.normal, transform);
    
    rec.p = p;
    rec.normal = normal;
    
    return true;
  }

  aabb bounding_box() const override { return bounds; }
  
  point3f centroid() const override { return center; }
  
private:
  bvh<T>* bvh = nullptr;
  mat4 transform; // inverse transform
  mat4 inv_transform; // inverse transform
  aabb bounds; // in world space
  point3f center; // in world space
};

#endif
