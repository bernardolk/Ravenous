#pragma once

#include "engine/core/core.h"
#include "engine/collision/collision_mesh.h"
#include "engine/geometry/mesh.h"
#include "engine/collision/primitives/bounding_box.h"


constexpr static size_t EntityWolrdCellOccupationLimit = 50;
const static std::string DefaultEntityShader = "model";
const static std::string EntityShaderMarking = "color";


enum EntityTimerTargetType
{
	EntityTimerTargetType_NotATarget          = 0,
	EntityTimerTargetType_VerticalSlidingDoor = 1,
};

enum EntityType
{
	EntityType_Static       = 0,
	EntityType_Checkpoint   = 1,
	EntityType_TimerTrigger = 2,
	EntityType_TimerTarget  = 3,
	EntityType_TimerMarking = 4,
};

enum EntityFlags
{
	EntityFlags_EmptyEntity        = (1 << 0),
	EntityFlags_InvisibleEntity    = (1 << 1),
	EntityFlags_HiddenEntity       = (1 << 2),
	EntityFlags_RenderTiledTexture = (1 << 3),
	EntityFlags_RenderWireframe    = (1 << 4),
};

struct TimerMarkingData
{
	vec3 color_off = vec3(0.1, 0.1, 0.1);
	vec3 color_on = vec3(0.1, 0.4, 0.85);
	vec3 color;
};

struct TimerTriggerData
{
	Entity* timer_target = nullptr;
	int timer_duration = 0; // Expressed in seconds

	constexpr static size_t size = 16;
	Entity* markings[size]; /* not perfect name, but means the lights that show
                                                           if the player is on track or not towards the timed door */
	u32 time_checkpoints[size];
	bool notification_mask[size];

	TimerTriggerData()
	{
		For(size)
		{
			markings[i] = nullptr;
			notification_mask[i] = false;
			time_checkpoints[i] = 0;
		}
	}
	void AddMarking(Entity* entity, u32 time_checkpoint);
	void DeleteMarking(int i);
};

struct TimerTargetData
{
	EntityTimerTargetType timer_target_type = EntityTimerTargetType_NotATarget;
	u32 timer_start_animation = 0; // animation id of anim to play when timer starts
	u32 timer_stop_animation = 0;  // animation id of anim to play when timer ends
};

// =======================
// >        ENTITY
// =======================
struct Entity
{
	u64 id = -1;
	std::string name = "NONAME";
	EntityType type = EntityType_Static;
	u32 flags = 0;

	// ---------------------------
	//  > render data
	// ---------------------------
	Shader* shader = nullptr;
	Mesh* mesh = nullptr;
	std::vector<Texture> textures;
	mat4 mat_model = Mat4Identity;

	// box UV tile setting
	int uv_tile_wrap[6] = {1, 1, 1, 1, 1, 1};


	// simulation data
	vec3 position = vec3(0.0f);
	vec3 rotation = vec3(0.0f);
	vec3 scale = vec3(1.0f);
	vec3 velocity = vec3(0.0f);
	glm::quat quaternion{};

	//@TODO: Get rid of collider (and include)
	CollisionMesh* collision_mesh = nullptr; // static collision mesh vertex data
	CollisionMesh collider{};                // dynamic collision mesh, obtained by multiplying static collision mesh with model matrix
	BoundingBox bounding_box{};              // computed using the collider mesh, used for fast first pass collision tests

	// collider settings
	bool slidable = false;

	WorldCell* world_cells[EntityWolrdCellOccupationLimit]{};
	int world_cells_count = 0;


	// temp missile fields
	bool dodged = false;
	float inv_period_timer = 0;

	// ---------------------------
	// > event trigger settings
	// ---------------------------
	Mesh* trigger = nullptr;
	vec3 trigger_scale = vec3(1.5f, 1.f, 0.f);
	vec3 trigger_pos = vec3(0.0f);
	mat4 trigger_mat_model{};

	// ---------------------------
	// > entity type data
	// ---------------------------

	union
	{
		TimerTriggerData timer_trigger_data{};
		TimerMarkingData timer_marking_data;
		TimerTargetData timer_target_data;
	};

	// ---------------------------
	// > methods
	// ---------------------------

	Entity() {};

	void Update();
	void UpdateCollider();
	void UpdateModelMatrix();
	void UpdateBoundingBox();
	void UpdateTrigger();
	void RotateY(float angle);
	mat4 GetRotationMatrix();
	CollisionMesh GetTriggerCollider();
	bool IsInteractable();

	void MakeInvisible();
	void MakeVisible();

};