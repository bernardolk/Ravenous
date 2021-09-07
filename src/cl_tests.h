const float COLLISION_EPSILON = 0.0001f;

struct PrimitivesCollision {
   bool is_collided  = false;
   float overlap     = 0;
   vec2 normal_vec   = vec2(0.0f);
   bool is_inside    = false;
};


PrimitivesCollision CL_circle_vs_square(float cx, float cz, float cr, float x0, float x1, float z0, float z1)
{
   // player is inside rect bounds
   if (x0 <= cx && x1 >= cx && z0 <= cz && z1 >= cz) 
   {
     PrimitivesCollision check;
     check.is_inside       = true;
     check.is_collided     = true;
     return check;  
   }  

   // n_vec = surface-normal vector from circle center to nearest point in rectangle surface
   float nx                = std::max(x0, std::min(x1, cx));
   float nz                = std::max(z0, std::min(z1, cz));
   vec2 n_vec              = vec2(cx, cz) - vec2(nx, nz);
   float distance          = glm::length(n_vec);
   float overlap           = cr - distance;

   return overlap > COLLISION_EPSILON ?
      PrimitivesCollision{true, overlap, glm::normalize(n_vec), false} :
      PrimitivesCollision{false};
}