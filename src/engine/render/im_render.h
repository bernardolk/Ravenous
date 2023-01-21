/* --------------------------- 
   > Instructions         
/* --------------------------- */
/* 
   This module allows for adding geometric primitives to a buffer and render them each frame from anywhere in the code, mostly
   for debugging purposes. To use, simply add ImDraw::<function>(IMHASH, <args>) to your code. The IMHASH macro will expand to
   a hash calculation based on file and line of the function call. This way we can 'keep alive' the obj in the buffer if it is
   being requested to be updated, instead of clearing it and reseting it.

   use IMCUSTOMHASH when you need to run ImDraw in a loop. In that case, __FILE__ and __LINE__ doesn't cut it,
   as in every iteration the item being added would be replaced because it would have the same hash always.
   Put a prefix + i from the loop in it and you should be fine.
*/

#define IMCUSTOMHASH(x) im_hasher(x)
#define IM_ITERHASH(x) im_hasher(std::string(__FILE__) + "-" + std::to_string(__LINE__) + "-" + std::to_string(x))
#define IMHASH im_hasher(std::string(__FILE__) + "-" + std::to_string(__LINE__))
#define IM_R_FIND_SLOT() ImDrawSlot slot = _find_element_or_empty_slot(_hash); \
                         if(slot.empty && slot.index == -1) return;

struct Mesh;
struct Vertex;
struct RenderOptions;
struct Camera;
struct ImDraw;
struct Entity;

using GLenum = unsigned int;

const std::hash<std::string> im_hasher;

struct ImDrawElement
{
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

struct ImDrawSlot
{
	bool empty;
	int index;
};

struct ImDraw
{
	static const int IM_BUFFER_SIZE = 200;
	inline static ImDrawElement* list;

	static void init();
	static void update(float frame_duration);
	static void render(Camera* camera);
	static void add(size_t _hash, std::vector<Vertex> vertex_vec, GLenum draw_method, RenderOptions opts = RenderOptions{});
	static void add(size_t _hash, std::vector<Triangle> triangles, GLenum draw_method, RenderOptions);
	static void add_line(size_t _hash, vec3 pointA, vec3 pointB, vec3 color);
	static void add_line(size_t _hash, vec3 pointA, vec3 pointB, float line_width = 1.0,
	                     bool always_on_top = false, vec3 color = vec3(0), float duration = 0);
	static void add_line_loop(size_t _hash, std::vector<vec3> points, float line_width = 1.0, bool always_on_top = false);
	static void add_point(size_t _hash, vec3 point, float point_size = 1.0,
	                      bool always_on_top = false, vec3 color = vec3(0), float duration = 0);
	static void add_point(size_t _hash, vec3 point, vec3 color = vec3(0));
	static void add_triangle(size_t _hash, Triangle t, float line_width = 1.0,
	                         bool always_on_top = false, vec3 color = vec3{0.8, 0.2, 0.2});
	static void add_mesh(size_t _hash, Mesh* mesh, vec3 pos, vec3 rot, vec3 scale, vec3 color = COLOR_BLUE_1, int duration = 2000);
	static void add_mesh(size_t _hash, Mesh* mesh, vec3 color = COLOR_BLUE_1, float duration = 2000);
	static void add_mesh(size_t _hash, Entity* entity, int duration);
	static void add_mesh(size_t _hash, Entity* entity);
	static void add_mesh(size_t _hash, Entity* entity, vec3 pos);

private:
	static void _set_mesh(int i, std::vector<Vertex> vertices, GLenum draw_method, RenderOptions opts);
	static void _set_mesh(int i, Mesh* mesh, RenderOptions opts);
	static void _update_mesh(int i, vec3 pos, vec3 rot, vec3 scale, vec3 color, int duration);
	static void _update_mesh(int i, vec3 color, int duration);
	static mat4 _get_mat_model(vec3 pos, vec3 rot, vec3 scale);
	static void _set_indices(int i, std::vector<u32> indices);
	static void _empty_slot(int i);

	static ImDrawSlot _find_element_or_empty_slot(size_t hash);
};
