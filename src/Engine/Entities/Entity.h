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
	bool visiting = false;
	vec3 chunk_position = vec3(0.f);
	RWorldChunk* chunk_ptr = nullptr;

	void Reset() { new (this) VisitorState(); }
};

/**  ---------------------------------------------------------------------
/*		EEntity: Basic Entity type which all entities inherit from.
/*			Represents a rendereable and collidable basic entity.
/**  --------------------------------------------------------------------- */
struct EEntity
{
	// Basic data needed for lower level systems to recognize an Entity type.
	TypeID type_id;
	uint64 id = 0;
	string name;

protected:
	friend RWorldChunk;
	bool deleted = false;

	// Renderable, Collidable Entity API Begin:
public:
	Flags flags = 0;
	
	VisitorState visitor_state;
	
	/** Simulation data */
	vec3 position = vec3(0.0f);
	vec3 rotation = vec3(0.0f);
	vec3 scale = vec3(1.0f);
	vec3 velocity = vec3(0.0f);
	//glm::quat quaternion{};
	
	/** Render data */
	RShader* shader = nullptr;
	RMesh* mesh = nullptr;
	vector<RTexture> textures;
	mat4 mat_model = Mat4Identity;
	
	// tmp? should probably be part of a texture data struct
	int uv_tile_wrap[6] = {1, 1, 1, 1, 1, 1};
	
	//@TODO: Get rid of collider (and include)
	RCollisionMesh* collision_mesh = nullptr; // static collision mesh vertex data
	RCollisionMesh collider{};                // dynamic collision mesh, obtained by multiplying static collision mesh with model matrix
	RBoundingBox bounding_box{};              // computed using the collider mesh, used for fast first pass collision tests

	// collider settings
	bool slidable = false;

	/** World data */
	// Array<WorldCell*, MaxEntityWorldCells> world_cells{};
	vector<RWorldChunk*> world_chunks;
	int world_chunks_count = 0;
	
	/** Event trigger */
	// TODO: Will only be necessary on I_Interactable
	RMesh* trigger = nullptr;
	vec3 trigger_scale = vec3(1.5f, 1.f, 0.f);
	vec3 trigger_pos = vec3(0.0f);
	mat4 trigger_mat_model{};

public:
	void Update();
	void UpdateCollider();
	void UpdateModelMatrix();
	void UpdateBoundingBox();
	void UpdateTrigger();
	
	void RotateY(float angle);
	mat4 GetRotationMatrix();
	
	RCollisionMesh GetTriggerCollider();
	
	void MakeInvisible();
	void MakeVisible();
};