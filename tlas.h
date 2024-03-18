#include "hittable.h"

#include "bvh.h"

struct tlas_node // TLAS - Top-level Acceleration Structure
{
  aabb bbox; // 24
  uint left_right; // 2x16 bits
  uint blas; // 4 // BLAS - Bottom-level Acceleration Structure, referes to BVH
  bool is_leaf() const { return left_right == 0; }
};

template<typename T>
class tlas : public hittable {
public:
  tlas() = default;
  tlas(bvh_instance<T>* bvh_list, int N) {
    // copy a pointer to the array of bottom level accstructs instances
    blas = bvh_list;
    blas_count = N;
    // allocate TLAS nodes
    tlas_nodes = (tlas_node*) aligned_alloc(64, sizeof(tlas_node) * N * 2);
    nodes_used = 2;
  }

  ~tlas() {
      if (tlas_nodes) {
        free(tlas_nodes);
        tlas_nodes = nullptr;
      }
  }
  
  tlas(tlas&& a)
  : bounds{a.bounds},
    center{a.center},
    tlas_nodes{a.tlas_nodes},
    blas{a.blas},
    nodes_used{a.nodes_used}, blas_count{a.primitives_count} {
      tlas_nodes = nullptr;
      blas = nullptr;

      nodes_used = 0;
      blas_count = 0;
  }
  
  tlas<T>& operator=(tlas&& a) {
    std::swap(tlas_nodes, a.tlas_nodes);
    std::swap(blas, a.blas);
    return *this;
  }
 
  void build() {
    // assign a TLASleaf node to each BLAS
    int node_idx[1024], node_indices = blas_count;
    nodes_used = 1;
    for (uint i = 0; i < blas_count; i++) {
      node_idx[i] = nodes_used;
      tlas_nodes[nodes_used].bbox.bmin = blas[i].bounding_box().bmin;
      tlas_nodes[nodes_used].bbox.bmax = blas[i].bounding_box().bmax;
      tlas_nodes[nodes_used].blas = i;
      tlas_nodes[nodes_used++].left_right = 0; // make it a leaft
    }
    // use agglomerative clustering to build the TLAS
    int A = 0, B = find_best_match(node_idx, node_indices, A);
    while (node_indices > 1) {
      int C = find_best_match(node_idx, node_indices, B);
      if (A == C) {
        int node_idx_A = node_idx[A], node_idx_B = node_idx[B];
        tlas_node& node_A = tlas_nodes[node_idx_A];
        tlas_node& node_B = tlas_nodes[node_idx_B];
        tlas_node& new_node = tlas_nodes[nodes_used];
        new_node.left_right = node_idx_A + (node_idx_B << 16);
        new_node.bbox.bmin = fminf(node_A.bbox.bmin, node_B.bbox.bmin);
        new_node.bbox.bmax = fmaxf(node_A.bbox.bmax, node_B.bbox.bmax);
        node_idx[A] = nodes_used++;
        node_idx[B] = node_idx[node_indices - 1];
        B = find_best_match(node_idx, --node_indices, A);
      }
      else A = B, B = C;
    }
    tlas_nodes[0] = tlas_nodes[node_idx[A]];
    
    bounds = aabb(tlas_nodes[0].bbox.bmin, tlas_nodes[0].bbox.bmax);
    center = (bounds.bmax + bounds.bmin) / 2.0f;
  }
 
  bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
    const tlas_node *node = &tlas_nodes[0], *stack[64];
    uint stack_ptr = 0;
    auto closest_so_far = ray_t.max;
    bool hit_anything = false;
    
    while (true)
    {
      if(node->is_leaf()) {
        hit_record temp_rec;
        
        if (blas[node->blas].hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
          hit_anything = true;
          closest_so_far = temp_rec.t;
          rec = temp_rec;
        }
        if (stack_ptr == 0)
          return hit_anything;
        else
          node = stack[--stack_ptr];
      } else {
        tlas_node* child1 = &tlas_nodes[node->left_right & 0xffff];
        interval i_a = interval(ray_t.min, closest_so_far);
        float dist1 = child1->bbox.hit(r, i_a);
        
        tlas_node* child2 = &tlas_nodes[node->left_right >> 16];
        interval i_b = interval(ray_t.min, closest_so_far);
        float dist2 = child2->bbox.hit(r, i_b);
 
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
    return bounds;
  }
    
  point3f centroid() const override { return center; }
private:
  int find_best_match( int* list, int N, int A) {
    float smallest = 1e30f;
    int best_b = -1;
    for (int B = 0; B < N; B++) if (B != A)
    {
      vec3f bmax = fmaxf( tlas_nodes[list[A]].bbox.bmax, tlas_nodes[list[B]].bbox.bmax );
      vec3f bmin = fminf( tlas_nodes[list[A]].bbox.bmin, tlas_nodes[list[B]].bbox.bmin );
      vec3f e = bmax - bmin;
      float surface_area = e.x() * e.y() + e.y() * e.z() + e.z() * e.x();
      if (surface_area < smallest) smallest = surface_area, best_b = B;
    }
    return best_b;
  }
  
  aabb bounds;
  point3f center; 
  
  tlas_node* tlas_nodes = nullptr;
  bvh_instance<T>* blas = nullptr; // array of BLASs
  uint nodes_used, blas_count;
};
