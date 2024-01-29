#pragma once

#include "engine/core/core.h"
#include "engine/collision/CollisionMesh.h"
#include "engine/collision/primitives/BoundingBox.h"
#include "Engine/Entities/Traits/EntityTraits.h"
#include "engine/geometry/mesh.h"

constexpr static uint MaxEntityWorldChunks = 20;
const static string DefaultEntityShader = "model";
const static string EntityShaderMarking = "color";
const static string DefaultEntityTexture = "pink";

enum NEntityFlags
{
	EntityFlags_EmptyEntity = 1 << 0,
	EntityFlags_InvisibleEntity = 1 << 1,
	EntityFlags_HiddenEntity = 1 << 2,
	EntityFlags_RenderWireframe = 1 << 3
};

struct VisitorState
{
	bool Visiting = false;
	vec3 ChunkPosition = vec3(0.f);
	RWorldChunk* ChunkPtr = nullptr;

	void Reset() { new(this) VisitorState; }
};

/*  =====================================================================
 *		EEntity: Basic Entity type which all entities inherit from.
 *			Represents a rendereable and collidable basic entity.
 *  ===================================================================== */
struct BaseEntityType(EEntity)
{
	Reflected_BaseEEntity(EEntity)

	friend RWorldChunk;

public:
	// Basic data needed for lower level systems to recognize an Entity type.
	REntityTypeID TypeID = 0;
	Field(RUUID, ID) = 0;
	string Name = "NoName";

	//	Entity flags
	Flags Flags = 0;

	// Simulation Data
	Field(vec3, Position) = vec3(0.0f);
	Field(vec3, Rotation) = vec3(0.0f);
	Field(vec3, Scale) = vec3(1.0f);
	vec3 Velocity = vec3(0.0f);
	//glm::quat quaternion{};

	// Render Data
	Field(RMesh*, Mesh) = nullptr;
	Field(RShader*, Shader) = nullptr;
	Field(RTexture, TextureDiffuse){};
	Field(RTexture, TextureSpecular){};
	Field(RTexture, TextureNormal){};
	mat4 MatModel = Mat4Identity;
	
	//@TODO: Get rid of collider (and include)
	Field(RCollisionMesh*, CollisionMesh) = nullptr;	// static collision mesh vertex data
	RCollisionMesh Collider{};							// dynamic collision mesh, obtained by multiplying static collision mesh with model matrix
	RBoundingBox BoundingBox{};							// computed using the collider mesh, used for fast first pass collision tests

	// @TODO temp
	bool Slidable = false;						// collider settings

	// World Data
	// Array<WorldCell*, MaxEntityWorldCells> world_cells{};
	vector<RWorldChunk*> WorldChunks{};
	int WorldChunksCount = 0;
	VisitorState VisitorState;

	// Event Trigger Data
	// TODO: Will only be necessary on I_Interactable
	RMesh* Trigger = nullptr;
	vec3 TriggerScale = vec3(1.5f, 1.f, 0.f);
	vec3 TriggerPos = vec3(0.0f);
	mat4 TriggerMatModel{};
	
	// Methods
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

	vec3 GetForwardVector() const;

protected:
	bool Deleted = false;

	// You shouldn't instantiate an EEntity directly. For a basic entity type, use EStaticMesh.
	EEntity() = default;

	void SetTypeID(REntityTypeID ID) { TypeID = ID; }
};
