/* --------------------------- 
   > Instructions         
/* --------------------------- */
/* 
   This module allows for adding geometric primitives to a buffer and render them each frame from anywhere in the code, mostly
   for debugging purposes. To use, simply add IM_RENDER.<function>(IMHASH, <args>) to your code. The IMHASH macro will expand to
   a hash calculation based on file and line of the function call. This way we can 'keep alive' the obj in the buffer if it is
   being requested to be updated, instead of clearing it and reseting it.

   use IMCUSTOMHASH when you need to run IM_RENDER in a loop. In that case, __FILE__ and __LINE__ doesn't cut it,
   as in every iteration the item being added would be replaced because it would have the same hash always.
   Put a prefix + i from the loop in it and you should be fine.
*/

#define IMCUSTOMHASH(x) im_hasher(x)
#define IM_ITERHASH(x) im_hasher(std::string(__FILE__) + "-" + std::to_string(__LINE__) + "-" + std::to_string(x))
#define IMHASH im_hasher(std::string(__FILE__) + "-" + std::to_string(__LINE__))
#define IM_R_FIND_SLOT() ImmediateDrawElementSlot slot = this->_find_element_or_empty_slot(_hash); \
                         if(slot.empty && slot.index == -1) return;

struct Mesh;
struct Vertex;
struct RenderOptions;
struct Camera;
struct ImDraw;
struct Entity;

using GLenum = unsigned int;

extern ImDraw IM_RENDER;
std::hash<std::string> im_hasher;

struct ImmediateDrawElement {
   size_t hash;
   bool empty;
   Mesh mesh;
   RenderOptions render_options;
   int duration;
   bool is_mesh;
   vec3 pos;
   vec3 rot;
   vec3 scale;
   bool is_multpl_by_matmodel;
};

struct ImmediateDrawElementSlot {
   bool empty;
   int index;
};

struct ImDraw {
   const static int IM_BUFFER_SIZE = 200;
   ImmediateDrawElement* list;

   void init();
   void update(float frame_duration);
   void render(Camera* camera);

   void add(size_t _hash, std::vector<Vertex> vertex_vec, GLenum draw_method, RenderOptions opts = RenderOptions{});
   void add(size_t _hash, std::vector<Triangle> triangles, GLenum draw_method, RenderOptions);

   void add_line(size_t _hash, vec3 pointA, vec3 pointB, vec3 color);
   void add_line(
      size_t _hash, vec3 pointA, vec3 pointB, float line_width = 1.0, 
      bool always_on_top = false, vec3 color = vec3(0), float duration = 0);
   void add_line_loop(size_t _hash, std::vector<vec3> points, float line_width = 1.0, bool always_on_top = false);

   void add_point(size_t _hash, vec3 point, float point_size = 1.0, bool always_on_top = false, vec3 color = vec3(0), float duration = 0);
   void add_point(size_t _hash, vec3 point, vec3 color = vec3(0));

   void add_triangle(size_t _hash, Triangle t, float line_width = 1.0, bool always_on_top = false, vec3 color = vec3{0.8, 0.2, 0.2});

   void add_mesh(size_t _hash, Mesh* mesh, vec3 pos, vec3 rot, vec3 scale, vec3 color = COLOR_BLUE_1, int duration = 2000);
   void add_mesh(size_t _hash, Mesh* mesh, vec3 color = COLOR_BLUE_1, float duration = 2000);
   void add_mesh(size_t _hash, Entity* entity, int duration);
   void add_mesh(size_t _hash, Entity* entity);
   void add_mesh(size_t _hash, Entity* entity, vec3 pos);

   ImmediateDrawElementSlot _find_element_or_empty_slot(size_t hash);
   void _set_mesh(int i, std::vector<Vertex> vertices, GLenum draw_method, RenderOptions opts);
   void _set_mesh(int i, Mesh* mesh, RenderOptions opts);
   void _update_mesh(int i, vec3 pos, vec3 rot, vec3 scale, vec3 color, int duration);
   void _update_mesh(int i, vec3 color, int duration);
   mat4 _get_mat_model(vec3 pos, vec3 rot, vec3 scale);
   void _set_indices(int i, std::vector<u32> indices);
   void _empty_slot(int i);
};
