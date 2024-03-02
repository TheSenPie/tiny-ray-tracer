#ifndef HITTABLE_LIST_H
#define HITTABLE_LIST_H

#include "hittable.h"
#include "aabb.h"

#include <memory>
#include <vector>

using std::shared_ptr;
using std::make_shared;

class hittable_list : public hittable {
public:
    std::vector<shared_ptr<hittable>> objects;

    hittable_list() {}
    hittable_list(shared_ptr<hittable> object) { add(object); }

    void clear() {
      objects.clear();
      bbox = aabb();
      center = point3f();
    }

    void add(const shared_ptr<hittable> object) {
      objects.push_back(object);
      bbox = aabb(bbox, object->bounding_box());
      center = (bbox.bmax + bbox.bmin) / 2.0f;
    }

    bool hit(const ray& r, interval ray_t, hit_record& rec) const override {
        hit_record temp_rec;
        bool hit_anything = false;
        auto closest_so_far = ray_t.max;

        for (const auto& object : objects) {
            if (object->hit(r, interval(ray_t.min, closest_so_far), temp_rec)) {
                hit_anything = true;
                closest_so_far = temp_rec.t;
                rec = temp_rec;
            }
        }

        return hit_anything;
    }
    
    aabb bounding_box() const override { return bbox; }
    
    point3f centroid() const override { return center; }
private:
  aabb bbox;
  point3f center;
};

#endif
