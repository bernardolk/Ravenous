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

   auto get_pos_and_scale()
   {
      struct {
         vec3 pos;
         vec3 scale;
      } result;

      result.pos     = vec3(minx, miny, minz);
      result.scale   = vec3(maxx - minx, maxy - miny, maxz - minz); 

      return result;
   }

   vec3 get_centroid()
   {
      return {
         (maxx + minx) / 2,
         (maxy + miny) / 2,
         (maxz + minz) / 2,
      };
   }
};