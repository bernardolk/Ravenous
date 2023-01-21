#pragma once

#include "engine/entity.h"

enum PlayerState
{
	PLAYER_STATE_FALLING,
	PLAYER_STATE_STANDING,
	PLAYER_STATE_WALKING,
	PLAYER_STATE_RUNNING,
	PLAYER_STATE_SPRINTING,
	PLAYER_STATE_JUMPING,
	PLAYER_STATE_SLIDING,
	PLAYER_STATE_SLIDE_FALLING,
	PLAYER_STATE_GRABBING,
	PLAYER_STATE_FALLING_FROM_EDGE,
	PLAYER_STATE_EVICTED_FROM_SLOPE,
	PLAYER_STATE_VAULTING
};

const static std::string PLAYER_NAME = "Player";

// ----------
// Animation
// ----------
enum PlayerAnimationState
{
	PlayerAnimationState_NoAnimation = 999,
	PlayerAnimationState_Jumping     = 0,
	PlayerAnimationState_Landing     = 1,
	PlayerAnimationState_LandingFall = 2,
	PlayerAnimationState_Vaulting    = 3
};

const float PLAYER_ANIMATION_DURATIONS[] = {
	400,                          // 0 - jumping
	200,                          // 1 - landing                  
	400,                          // 2 - landing fall   
	0                             // 3 - vaulting
};

// forward declarations
struct Player;
struct Entity;
struct World;

void AN_p_anim_force_interrupt(Player* player);
bool CL_update_player_world_cells(Player* player, World* world);
void CL_recompute_collision_buffer_entities(Player* player);


struct Player
{
	Entity* entity_ptr;

	// [start] DROP ALL THESE
	Entity* standing_entity_ptr = nullptr;;
	Entity* slope_player_was_ptr = nullptr;;
	Entity* vaulting_entity_ptr = nullptr;;
	Entity* skip_collision_with_floor = nullptr;
	// [end]

	// geometry
	float radius = 0.2;
	float height = 1.75;

	// movement variables
	vec3  v_dir = vec3(0.f);          // intended movement direction
	vec3  v_dir_historic = vec3(0.f);          // last non zero movement direction
	float speed = 0;                           // accumulated speed scalar

	// movement constants
	float acceleration = 7.5;
	float air_delta_speed = 0.05;
	float run_speed = 4.0;
	float dash_speed = 6.0;
	float walk_speed = 0.92;
	float fall_speed = 0.01;
	float fall_acceleration = 0.2;
	float air_speed = 1.00;
	float jump_initial_speed = 5.0;
	float jump_horz_thrust = 3.0;
	float jump_horz_dash_thrust = 5.0;
	float slide_jump_speed = 6.7;
	float slide_speed = 2.0;
	float fall_from_edge_push_speed = 1.5;
	vec3  gravity = vec3(0, -9.0, 0);


	// movement states
	bool dashing = false;
	bool walking = false;
	bool jumping_upwards = false;
	bool landing = false;
	bool jumping_from_slope = false;
	bool action = false;
	bool want_to_grab = false;
	bool dodge_btn = false;

	PlayerState player_state;
	PlayerState initial_player_state;

	vec3 prior_position = vec3(0);
	vec3 initial_velocity = vec3(0);

	vec3 orientation;

	// gameplay system variables
	vec3    last_terrain_contact_normal = vec3(0, 1.f, 0);
	Entity* grabbing_entity = nullptr;
	float   grab_reach = 0.9;         // radius + arms reach, 0.5 + 0.4  

	// sliding
	vec3 sliding_direction = vec3(0);
	vec3 sliding_normal = vec3(0);

	// health and hurting
	int   initial_lives = 2;
	int   lives = 2;
	float hurt_height_1 = 5.0;
	float hurt_height_2 = 8.0;
	float height_before_fall;
	float fall_height_log = 0;                        // set when checking for fall, read-only!

	// checkpoints
	Entity* checkpoint = nullptr;
	vec3    checkpoint_pos;

	// animation
	float                anim_t = 0;                                         // animation timer
	PlayerAnimationState anim_state = PlayerAnimationState_NoAnimation;         // animation state
	vec3                 anim_final_pos = vec3(0);                           // final position after translation animation
	vec3                 anim_orig_pos = vec3(0);                           // original position
	vec3                 anim_final_dir = vec3(0);                           // final player orientation
	vec3                 anim_orig_dir = vec3(0);                           // original player orientation
	bool                 anim_finished_turning = false;                       // player has finished turning his camera

	void update(World* world, bool update_collider = false)
	{
		// perform updates to bounding boxes, colliders etc
		entity_ptr->update_model_matrix();
		if(update_collider)
		{
			entity_ptr->update_collider();
			entity_ptr->update_bounding_box();
		}

		bool cells_updated = CL_update_player_world_cells(this, world);
		if(cells_updated)
		{
			CL_recompute_collision_buffer_entities(this);
		}
	}

	vec3 feet()
	{
		return entity_ptr->position;
	}

	vec3 top()
	{
		return entity_ptr->position + vec3(0.0f, height, 0.0f);
	}

	vec3 eye()
	{
		return entity_ptr->position + vec3(0, height - 0.1, 0);
	}

	vec3 last_terrain_contact_point()
	{
		vec3 player_btm_sphere_center = entity_ptr->position + vec3(0, radius, 0);
		return player_btm_sphere_center + -last_terrain_contact_normal * radius;
	}

	bool maybe_hurt_from_fall()
	{
		float fall_height = height_before_fall - entity_ptr->position.y;
		fall_height_log = fall_height;
		if(fall_height >= hurt_height_2)
		{
			lives -= 2;
			return true;
		}
		if(fall_height >= hurt_height_1)
		{
			lives -= 1;
			return true;
		}
		return false;
	}

	void restore_health()
	{
		lives = initial_lives;
	}

	void set_checkpoint(Entity* entity)
	{
		if(entity->type != EntityType_Checkpoint)
			assert(false);

		checkpoint_pos = entity_ptr->position;
		checkpoint = entity;
	}

	void goto_checkpoint()
	{
		entity_ptr->position = checkpoint_pos;
	}

	void die()
	{
		lives = initial_lives;
		entity_ptr->velocity = vec3(0);
		player_state = PLAYER_STATE_STANDING;
		AN_p_anim_force_interrupt(this);
		goto_checkpoint();
	}

	void brute_stop()
	{
		// bypass deaceleration steps. Stops player right on his tracks.
		speed = 0;
	}

	void start_jump_animation()
	{ }


	// // pass through methods
	// vec3 position()
	// {
	//    return entity_ptr->position;
	// }

	// vec3 rotation()
	// {
	//    return entity_ptr->rotation;
	// }

	// vec3 scale()
	// {
	//    return entity_ptr->scale;
	// }

	// vec3 velocity()
	// {
	//    return entity_ptr->velocity;
	// }
};
