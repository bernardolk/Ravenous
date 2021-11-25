
// --------------
// > Vertex
// --------------
struct Vertex {
    vec3 position;
    vec3 normal;
    vec2 tex_coords;
    vec3 tangent;
    vec3 bitangent;

    Vertex operator*(glm::mat4 mat)
    {
       // should do something for tex_coords ???

       vec3 n_position   = vec3(mat * vec4(position,  1.f));
       vec3 n_normal     = vec3(mat * vec4(normal,    1.f));
       vec3 n_tangent    = vec3(mat * vec4(tangent,   1.f));
       vec3 n_bitangent  = vec3(mat * vec4(bitangent, 1.f));

      return Vertex{n_position, n_normal, tex_coords, n_tangent, n_bitangent};
    };

    Vertex& operator*=(glm::mat4 mat)
    {
       // should do something for tex_coords ???

       position   = vec3(mat * vec4(position,  1.f));
       normal     = vec3(mat * vec4(normal,    1.f));
       tangent    = vec3(mat * vec4(tangent,   1.f));
       bitangent  = vec3(mat * vec4(bitangent, 1.f));

      return *this;
    };
};


// --------------
// > BoundingBox
// --------------
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
};


// --------------
// > Ray
// --------------
struct Ray {
   vec3 origin;
   vec3 direction;

   vec3 get_inv()
   {
      return vec3(1.0 / direction.x, 1.0 / direction.y, 1.0 / direction.z);
   }
};


// --------------
// > CL_Results
// --------------
struct Entity;

struct CL_Results {
   bool collision = false;
   Entity* entity;
   float penetration;
   vec3 normal;
};

struct CL_ResultsArray {
   CL_Results results[10];
   int count = 0;
};