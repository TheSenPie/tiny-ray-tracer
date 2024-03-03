#include "hittable.h"

struct tlas_node // TLAS - Top-level Acceleration Structure
{
  aabb bbox;
  uint left_blas; // BLAS - Bottom-level Acceleration Structure, referes to BVH
  bool is_leaf;
};

class tlas : public hittable {
public:
  tlas() = default;
  tlas(bvh* bvh_list, int N);
  void build();
  bool hit(const ray& r, interval ray_t, hit_record& rec) const override { /* ... */ }
private:
  tlas_node* tlas_node = nullptr;
  bvh* blas = nullptr;
  uint nodes_used, blas_count;
};
