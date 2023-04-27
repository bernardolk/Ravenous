#pragma once

#include "engine/core/core.h"
#include "engine/entities/entity.h"
#include "engine/entities/traits/entity_traits.h"

enum class PlayerState
{
	Falling,
	Standing,
	Walking,
	Running,
	Sprinting,
	Jumping,
	Sliding,
	SlideFalling,
	Grabbing,
	FallingFromEdge,
	EvictedFromSlope,
	Vaulting
};

const static std::string PlayerName = "Player";

enum class PlayerAnimationState
{
	NoAnimation,
	Jumping,
	Landing,
	LandingFall,
	Vaulting
};

void ForceInterruptPlayerAnimation(Player* player);


struct EntityDecl(Player)
{
	// [start] DROP ALL THESE
	E_Entity* standing_entity_ptr = nullptr;;
	E_Entity* slope_player_was_ptr = nullptr;;
	E_Entity* vaulting_entity_ptr = nullptr;;
	E_Entity* skip_collision_with_floor = nullptr;
	// [end]

	// geometry
	float radius = 0.2;
	float height = 1.75;

	// movement variables
	vec3 v_dir = vec3(0.f);          // intended movement direction
	vec3 v_dir_historic = vec3(0.f); // last non zero movement direction

	// movement constants
	static constexpr float acceleration = 12.0;
	static constexpr float air_speed_acceleration = 6;
	static constexpr float run_speed = 4.0;
	static constexpr float dash_speed = 8.0;
	static constexpr float walk_speed = 0.95;
	static constexpr float fall_speed = 0.01;
	static constexpr float max_air_speed = 2.70;
	static constexpr float jump_initial_speed = 10.0;
	static constexpr float jump_horz_thrust = 1.5;
	static constexpr float jump_horz_dash_thrust = 3.5;
	static constexpr float slide_jump_speed = 6.7;
	static constexpr float slide_speed = 2.0;
	static constexpr float fall_from_edge_push_speed = 1.5;
	static constexpr vec3 gravity = vec3(0, -20.0, 0);


	// movement states
	// TODO: Turn into flags
	bool dashing = false;
	bool walking = false;
	bool jumping_upwards = false;
	bool landing = false;
	bool jumping_from_slope = false;
	bool action = false;
	bool want_to_grab = false;
	bool dodge_btn = false;
	bool interact_btn = false;

	PlayerState player_state;
	PlayerState initial_player_state;

	vec3 prior_position = vec3(0);
	vec3 initial_velocity = vec3(0);

	vec3 orientation;

	// gameplay system variables
	vec3 last_terrain_contact_normal = vec3(0, 1.f, 0);
	E_Entity* grabbing_entity = nullptr;
	float grab_reach = 0.9; // radius + arms reach, 0.5 + 0.4  

	// sliding
	vec3 sliding_direction = vec3(0);
	vec3 sliding_normal = vec3(0);

	// health and hurting
	int initial_lives = 2;
	int lives = 2;
	float hurt_height_1 = 5.0;
	float hurt_height_2 = 8.0;
	float height_before_fall;
	float fall_height_log = 0; // set when checking for fall, read-only!

	// checkpoints
	E_Entity* checkpoint = nullptr;
	vec3 checkpoint_pos;

	// animation
	float anim_t = 0;                                                    // animation timer
	PlayerAnimationState anim_state = PlayerAnimationState::NoAnimation; // animation state
	vec3 anim_final_pos = vec3(0);                                       // final position after translation animation
	vec3 anim_orig_pos = vec3(0);                                        // original position
	vec3 anim_final_dir = vec3(0);                                       // final player orientation
	vec3 anim_orig_dir = vec3(0);                                        // original player orientation
	bool anim_finished_turning = false;                                  // player has finished turning his camera

	static Player* Get()
	{
		static Player instance;
		return &instance;
	}

	void Update(T_World* world, bool update_collider = false);

	vec3 GetFeetPosition() const { return position; }

	vec3 GetUpperBoundPosition() const { return -position + vec3(0.0f, height, 0.0f); }

	vec3 GetEyePosition() const { return position + vec3(0, height - 0.1, 0); }

	float GetSpeed() const { return length(velocity); }

	float GetHorizontalSpeed() const { vec2 vh = velocity.xz ; return length(vh); }

	void MultiplySpeed(float multiplier) { velocity = length(velocity) * multiplier * v_dir; }
	
	void SetSpeed(float new_speed) { velocity = new_speed * v_dir; }

	void SetHorizontalSpeed(float new_speed) { vec2 hv = new_speed * normalize(vec2(v_dir.xz)); velocity.x = hv.x; velocity.z = hv.y; }
	
	vec3 GetLastTerrainContactPoint() const;

	bool MaybeHurtFromFall();
	void RestoreHealth();
	void SetCheckpoint(E_Entity* entity);
	void GotoCheckpoint();
	void Die();
	void BruteStop();

private:
	friend struct GlobalSceneInfo;

	Player() = default;
	Player(const Player& other) = delete;

	static Player* ResetPlayer();
};
