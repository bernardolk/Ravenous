struct BoundingBox {
   float minx;
   float maxx;
   float minz;
   float maxz;
   float miny;
   float maxy;

   auto bounds()
   {
      struct {
         vec3 min, max;
      } bounds;

      bounds.min = vec3(minx, miny, minz);
      bounds.max = vec3(maxx, maxy, maxz);
      return bounds;
   }

   void set(vec3 min, vec3 max)
   {
      minx = min.x;
      maxx = max.x;
      miny = min.y;
      maxy = max.y;
      minz = min.z;
      maxz = max.z;
   }
};


bool CL_test_bounding_boxes(BoundingBox a, BoundingBox b)
{
   // Exit with no intersection if separated along an axis
   if (a.maxx < b.minx || a.minx > b.maxx) return false;
   if (a.maxy < b.miny || a.miny > b.maxy) return false;
   if (a.maxz < b.minz || a.minz > b.maxz) return false;
   // Overlapping on all axes means AABBs are intersecting
   return true;
}