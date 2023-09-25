#pragma once

#include "engine/core/core.h"
#include "engine/collision/CollisionMesh.h"
#include "engine/collision/primitives/BoundingBox.h"
#include "engine/geometry/mesh.h"

constexpr static uint MaxEntityWorldChunks = 20;
const static std::string DefaultEntityShader = "model";
const static std::string EntityShaderMarking = "color";

enum EntityFlags
{
	EntityFlags_EmptyEntity        = (1 << 0),
	EntityFlags_InvisibleEntity    = (1 << 1),
	EntityFlags_HiddenEntity       = (1 << 2),
	EntityFlags_RenderTiledTexture = (1 << 3),
	EntityFlags_RenderWireframe    = (1 << 4),
};

struct VisitorState
{
	bool Visiting = false;
	vec3 ChunkPosition = vec3(0.f);
	RWorldChunk* ChunkPtr = nullptr;

	void Reset() { new(this) VisitorState(); }
};

/**  ---------------------------------------------------------------------
/*		EEntity: Basic Entity type which all entities inherit from.
/*			Represents a rendereable and collidable basic entity.
/**  --------------------------------------------------------------------- */
struct EEntity
{
	// Basic data needed for lower level systems to recognize an Entity type.
	TypeID TypeID;
	uint64 ID = 0;
	string Name;

protected:
	friend RWorldChunk;
	bool Deleted = false;

	// Renderable, Collidable Entity API Begin:
public:
	Flags Flags = 0;

	VisitorState VisitorState;

	/** Simulation data */
	vec3 Position = vec3(0.0f);
	vec3 Rotation = vec3(0.0f);
	vec3 Scale = vec3(1.0f);
	vec3 Velocity = vec3(0.0f);
	//glm::quat quaternion{};

	/** Render data */
	RShader* Shader = nullptr;
	RMesh* Mesh = nullptr;
	vector<RTexture> Textures;
	mat4 MatModel = Mat4Identity;

	// tmp? should probably be part of a texture data struct
	int UvTileWrap[6] = {1, 1, 1, 1, 1, 1};

	//@TODO: Get rid of collider (and include)
	RCollisionMesh* CollisionMesh = nullptr; // static collision mesh vertex data
	RCollisionMesh Collider{};                // dynamic collision mesh, obtained by multiplying static collision mesh with model matrix
	RBoundingBox BoundingBox{};              // computed using the collider mesh, used for fast first pass collision tests

	// collider settings
	bool Slidable = false;

	/** World data */
	// Array<WorldCell*, MaxEntityWorldCells> world_cells{};
	vector<RWorldChunk*> WorldChunks;
	int WorldChunksCount = 0;

	/** Event trigger */
	// TODO: Will only be necessary on I_Interactable
	RMesh* Trigger = nullptr;
	vec3 TriggerScale = vec3(1.5f, 1.f, 0.f);
	vec3 TriggerPos = vec3(0.0f);
	mat4 TriggerMatModel{};

public:
	void Update();
	void UpdateCollider();
	void UpdateModelMatrix();
	void UpdateBoundingBox();
	void UpdateTrigger();

	void RotateY(float Angle);
	mat4 GetRotationMatrix();

	RCollisionMesh GetTriggerCollider();

	void MakeInvisible();
	void MakeVisible();
};
