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
#define IM_R_FIND_SLOT() FindElementOrEmptySlot(_hash); if(Slot.Empty && Slot.Index == -1) return;


struct RenderOptions;
constexpr inline std::hash<std::string> ImHasher;

struct RImDrawElement
{
	uint Hash;
	bool Empty;
	RMesh Mesh;
	RenderOptions RenderOptions;
	int Duration;
	bool IsMesh;
	vec3 Position;
	vec3 Rotation;
	vec3 Scale;
	bool IsMultplByMatmodel;
};

struct RImDrawSlot
{
	bool Empty;
	int Index;
};

struct RImDraw
{
	static constexpr int ImBufferSize = 200;
	inline static RImDrawElement* List;

	static void Init();
	static void Update(float FrameDuration);
	static void Render(RCamera* Camera);
	static auto Add(uint Hash, vector<RVertex> VertexVec, GLenum DrawMethod, RenderOptions Opts = RenderOptions{}) -> void;
	static void Add(uint Hash, vector<RTriangle> Triangles, GLenum DrawMethod, RenderOptions);
	static void AddLine(uint Hash, vec3 PointA, vec3 PointB, vec3 Color);
	static void AddLine(uint Hash, vec3 PointA, vec3 PointB, float LineWidth = 1.0, bool AlwaysOnTop = false, vec3 Color = vec3(0), float Duration = 0);
	static void AddLineLoop(uint Hash, vector<vec3> Points, float LineWidth = 1.0, bool AlwaysOnTop = false);
	static void AddPoint(uint Hash, vec3 Point, float PointSize = 1.0, bool AlwaysOnTop = false, vec3 Color = vec3(0), float Duration = 0);
	static void AddPoint(uint Hash, vec3 Point, vec3 Color = vec3(0));
	static void AddTriangle(uint Hash, RTriangle Triangle, float LineWidth = 1.0, bool AlwaysOnTop = false, vec3 Color = vec3{0.8, 0.2, 0.2});
	static void AddMesh(uint Hash, RMesh* Mesh, vec3 Position, vec3 Rotation, vec3 Scale, vec3 Color = COLOR_BLUE_1, int Duration = 2000);
	static void AddMesh(uint Hash, RMesh* Mesh, vec3 Color = COLOR_BLUE_1, float Duration = 2000);
	static void AddMesh(uint Hash, EEntity* Entity, int Duration);
	static void AddMesh(uint Hash, EEntity* Entity);
	static void AddMesh(uint Hash, EEntity* Entity, vec3 Position);

private:
	static void SetMesh(int Index, vector<RVertex> Vertices, GLenum DrawMethod, RenderOptions Opts);
	static void SetMesh(int Index, RMesh* Mesh, RenderOptions Opts);
	static void UpdateMesh(int Index, vec3 Position, vec3 Rotation, vec3 Scale, vec3 Color, int Duration);
	static void UpdateMesh(int Index, vec3 Color, int Duration);
	static mat4 GetMatModel(vec3 Position, vec3 Rotation, vec3 Scale);
	static void SetIndices(int Index, vector<uint> Indices);
	static void EmptySlot(int Index);

	static RImDrawSlot FindElementOrEmptySlot(uint Hash);
};
