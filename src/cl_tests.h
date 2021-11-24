

bool CL_test_ray_vs_aabb(Ray ray, BoundingBox box)
{
   vec3 ray_inv = ray.get_inv();

   float tx1 = (box.minx - ray.origin.x) * ray_inv.x;
   float tx2 = (box.maxx - ray.origin.x) * ray_inv.x;

   float tmin = min(tx1, tx2);
   float tmax = max(tx1, tx2);

   float ty1 = (box.miny - ray.origin.y) * ray_inv.y;
   float ty2 = (box.maxy - ray.origin.y) * ray_inv.y;

   tmin = max(tmin, min(ty1, ty2));
   tmax = min(tmax, max(ty1, ty2));

   float tz1 = (box.minz - ray.origin.z) * ray_inv.z;
   float tz2 = (box.maxz - ray.origin.z) * ray_inv.z;

   tmin = max(tmin, min(tz1, tz2));
   tmax = min(tmax, max(tz1, tz2));

   return tmax >= tmin;
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