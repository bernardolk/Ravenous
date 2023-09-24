#pragma once

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

#include "engine/core/core.h"
#include "renderer.h"
#include "engine/geometry/mesh.h"
#include "engine/utils/colors.h"

#define IMCUSTOMHASH(x) ImHasher(x)
#define IM_ITERHASH(x) ImHasher(std::string(__FILE__) + "-" + std::to_string(__LINE__) + "-" + std::to_string(x))
#define IMHASH ImHasher(std::string(__FILE__) + "-" + std::to_string(__LINE__))
#define IM_R_FIND_SLOT() ImDrawSlot slot = FindElementOrEmptySlot(_hash); \
                         if(slot.empty && slot.index == -1) return;


struct RenderOptions;
constexpr inline std::hash<std::string> ImHasher;

struct ImDrawElement
{
	u32 hash;
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
	static constexpr int im_buffer_size = 200;
	inline static ImDrawElement* list;

	static void Init();
	static void Update(float frame_duration);
	static void Render(Camera* camera);
	static auto Add(u32 _hash, vector<Vertex> vertex_vec, GLenum draw_method, RenderOptions opts = RenderOptions{}) -> void;
	static void Add(u32 _hash, vector<Triangle> triangles, GLenum draw_method, RenderOptions);
	static void AddLine(u32 _hash, vec3 point_a, vec3 point_b, vec3 color);
	static void AddLine(u32 _hash, vec3 point_a, vec3 point_b, float line_width = 1.0,
	                    bool always_on_top = false, vec3 color = vec3(0), float duration = 0);
	static void AddLineLoop(u32 _hash, vector<vec3> points, float line_width = 1.0, bool always_on_top = false);
	static void AddPoint(u32 _hash, vec3 point, float point_size = 1.0,
	                     bool always_on_top = false, vec3 color = vec3(0), float duration = 0);
	static void AddPoint(u32 _hash, vec3 point, vec3 color = vec3(0));
	static void AddTriangle(u32 _hash, Triangle t, float line_width = 1.0,
	                        bool always_on_top = false, vec3 color = vec3{0.8, 0.2, 0.2});
	static void AddMesh(u32 _hash, Mesh* mesh, vec3 pos, vec3 rot, vec3 scale, vec3 color = COLOR_BLUE_1, int duration = 2000);
	static void AddMesh(u32 _hash, Mesh* mesh, vec3 color = COLOR_BLUE_1, float duration = 2000);
	static void AddMesh(u32 _hash, EEntity* entity, int duration);
	static void AddMesh(u32 _hash, EEntity* entity);
	static void AddMesh(u32 _hash, EEntity* entity, vec3 pos);

private:
	static void SetMesh(int i, vector<Vertex> vertices, GLenum draw_method, RenderOptions opts);
	static void SetMesh(int i, Mesh* mesh, RenderOptions opts);
	static void UpdateMesh(int i, vec3 pos, vec3 rot, vec3 scale, vec3 color, int duration);
	static void UpdateMesh(int i, vec3 color, int duration);
	static mat4 GetMatModel(vec3 pos, vec3 rot, vec3 scale);
	static void SetIndices(int i, vector<u32> indices);
	static void EmptySlot(int i);

	static ImDrawSlot FindElementOrEmptySlot(u32 hash);
};
