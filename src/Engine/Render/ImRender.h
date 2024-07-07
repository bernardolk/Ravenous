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
#include "Engine/Geometry/Quad.h"
#include "engine/geometry/mesh.h"
#include "engine/utils/colors.h"

#define IMCUSTOMHASH(x) ImHasher(x)
#define IM_ITERHASH(x) ImHasher(std::string(__FILE__) + "-" + std::to_string(__LINE__) + "-" + std::to_string(x))
#define IMHASH ImHasher(string(__FILE__) + "-" + to_string(__LINE__))

struct RRenderOptions;
constexpr inline std::hash<std::string> ImHasher;

struct RImDrawElement
{
	uint Hash;
	bool Empty;
	RMesh Mesh;
	RRenderOptions RRenderOptions;
	int Duration;
	bool IsMesh;
	vec3 Position;
	vec3 Rotation;
	vec3 Scale;
	bool IsMultplByMatmodel;
};

struct RImDraw
{
	static constexpr int ImBufferSize = 200;
	inline static RImDrawElement* List;

	static void Init();
	static void Update(float FrameDuration);
	static void Render(RCamera* Camera);

	// Entity
	static void AddEntity(uint Hash, EEntity* Entity, int Duration = DefaultDuration, RRenderOptions Opts = {});

	// Meshes
	static void AddMeshWithTransform(uint Hash, RMesh* Mesh, vec3 Position, vec3 Rotation, vec3 Scale, int Duration = DefaultDuration, RRenderOptions Opts = {.Wireframe =  true});
	static void AddMeshAtPosition(uint Hash, RMesh* Mesh, vec3 Position, int Duration = DefaultDuration, RRenderOptions Opts = {.Wireframe =  true});
	static void AddCollisionMesh(uint _hash, RCollisionMesh* CollisionMesh, int Duration = DefaultDuration, RRenderOptions Opts = {.Wireframe =  true, .AlwaysOnTop = true, .DontCullFace = true});
	static void AddBoundingBox(uint _hash, RBoundingBox& BoundingBox, int Duration = DefaultDuration, RRenderOptions Opts = {.Wireframe = true, .AlwaysOnTop = true, .DontCullFace = true});
	
	// Lines
	static void AddLine(uint Hash, vec3 PointA, vec3 PointB, int Duration = DefaultDuration, vec3 Color = vec3(0.f), float LineWidth = 2.f, bool AlwaysOnTop = true);
	static void AddLineLoop(uint _hash, vector<RVertex>& Vertices, int Duration = DefaultDuration, vec3 Color = vec3{0.f}, RRenderOptions Opts = {});

	// Points
	static void AddPoint(uint Hash, vec3 Point, int Duration = DefaultDuration, vec3 Color = vec3{0.f}, float PointSize = 1.0, bool AlwaysOnTop = false);

	// Quads
	static void AddQuad(uint _hash, RQuad Quad, int Duration = DefaultDuration, vec3 Color = COLOR_BLACK, RRenderOptions Opts = RRenderOptions{});
	
	// Low level vertices
	static void AddVertexList(uint _hash, vector<RVertex>& VertexVec, int Duration = DefaultDuration, RRenderOptions Opts = {}, GLenum DrawMethod = 4);
	
private:
	static inline int DefaultDuration = 40;
	
	static void AddOrUpdateDrawElement(uint _hash, vector<RVertex>& Vertices, int Duration, RRenderOptions Opts, uint DrawMethod);
	static void SetMeshFromVertices(int Index, vector<RVertex>& Vertices, GLenum DrawMethod, RRenderOptions Opts);
	static void SetMesh(int Index, RMesh* Mesh, RRenderOptions Opts);
	static void UpdateMeshTransform(int Index, vec3 Position, vec3 Rotation, vec3 Scale, vec3 Color, int Duration);
	static void UpdateMeshDuration(int Index, int Duration);
	static void UpdateMeshColor(int Index, vec3 Color);
	static mat4 GetMatModel(vec3 Position, vec3 Rotation, vec3 Scale);
	static void EmptySlot(int Index);
	static int GetNewSlotIndex();
	static int FindDrawElement(uint Hash);
};
