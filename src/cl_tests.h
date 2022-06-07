

auto CL_test_ray_vs_aabb(Ray ray, BoundingBox box)
{
   struct {
      bool hit;
      float tmin;
      float tmax;
   } result;

   vec3 ray_inv = ray.get_inv();

   float tx1 = (box.minx - ray.origin.x) * ray_inv.x;
   float tx2 = (box.maxx - ray.origin.x) * ray_inv.x;

   float tmin = std::min(tx1, tx2);
   float tmax = std::max(tx1, tx2);

   float ty1 = (box.miny - ray.origin.y) * ray_inv.y;
   float ty2 = (box.maxy - ray.origin.y) * ray_inv.y;

   tmin = std::max(tmin, std::min(ty1, ty2));
   tmax = std::min(tmax, std::max(ty1, ty2));

   float tz1 = (box.minz - ray.origin.z) * ray_inv.z;
   float tz2 = (box.maxz - ray.origin.z) * ray_inv.z;

   tmin = std::max(tmin, std::min(tz1, tz2));
   tmax = std::min(tmax, std::max(tz1, tz2));

   result.hit  = tmax >= tmin;
   result.tmin = tmin; 
   result.tmax = tmax;
   return result;
}


bool CL_test_bounding_boxes(BoundingBox a, BoundingBox b)
{
   // Exit with no intersection if separated along an axis
   if (a.maxx < b.minx || a.minx > b.maxx) return false;
   if (a.maxy < b.miny || a.miny > b.maxy) return false;
   if (a.maxz < b.minz || a.minz > b.maxz) return false;
   // Overlapping on all axes means AABBs are intersecting
   return true;
}