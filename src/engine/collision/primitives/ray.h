struct Ray {
   vec3 origin;
   vec3 direction;
   vec3 inv;
   bool inv_set = false;

   vec3 get_inv()
   {
      if(!inv_set)
      {
         inv = vec3(1.0 / direction.x, 1.0 / direction.y, 1.0 / direction.z);
         inv_set = true;
      }
      return inv;
   }
};