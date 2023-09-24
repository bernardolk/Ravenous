#pragma once

#include "engine/core/core.h"
#include "engine/entities/Entity.h"
#include "engine/entities/traits/EntityTraits.h"
#include "engine/io/InputPhase.h"
#include "engine/utils/utils.h"
#include "engine/collision/ClEdgeDetection.h"

enum class NPlayerState: uint32_t
{
	// Floor based states
	Standing				= 100,

	// Air based states
	Falling					= 200,
	Jumping          		= 201,

	// Partially uncontrolled movement states
	Sliding					= 300,
	SlideFalling			= 301,

	// Special movement states
	Grabbing				= 400,
	Vaulting				= 401,
};

struct RPlayerStateChangeArgs
{
	// collision
	EEntity* entity = nullptr;
	vec3 normal = vec3(0);
	float penetration = 0;

	union
	{
		// grabbing info
		struct VaultingData
		{
			vec3 final_position;
			RLedge ledge;
		} vaulting_data;
	};

	RPlayerStateChangeArgs() : vaulting_data() {}
};

enum class RPlayerAnimationState
{
	NoAnimation,
	Jumping,
	Landing,
	LandingFall,
	Vaulting
};

void ForceInterruptPlayerAnimation(EPlayer* player);


struct EntityType(EPlayer)
{
	Reflected()
	DeclSingleton(EPlayer)

	// geometry
	float radius = 0.2f;
	float height = 1.75f;

	// movement variables
	vec3 v_dir = vec3(0.f);          // intended movement direction
	vec3 last_recorded_movement_direction = vec3(0.f); // last non zero movement direction

	// movement constants
	static const float acceleration;
	static const float run_speed;
	static const float dash_speed;
	static const float walk_speed;
	static const float fall_speed;
	static const float air_steering_velocity;
	static const float max_air_speed;
	static const float air_friction;
	static const float jump_initial_speed;
	static const float jump_reduced_horizontal_thrust;
	static const float jump_from_slope_horizontal_thrust;
	static const float minimum_jump_horizontal_thrust_when_dashing;
	static const float minimum_jump_horizontal_thrust_when_running;
	static const float slide_jump_speed;
	static const float slide_speed;
	static const float fall_from_edge_push_speed;
	static const vec3 gravity;

	// other constants
	static constexpr float slope_min_angle = 0.4;

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
	bool pressing_forward_while_in_air = false;
	bool stopped_pressing_forward_while_in_air = false;
	bool pressing_left_while_in_air = false;
	bool pressing_right_while_in_air = false;
	bool pressing_backward_while_in_air = false;
	bool pressing_forward_while_standing = false;

	bool pressing_left_while_standing = false;
	bool pressing_right_while_standing = false;
	bool pressing_backward_while_standing = false;

	uint64 first_pressed_movement_key_while_standing = KEY_NONE;
	
	NPlayerState player_state;
	NPlayerState initial_player_state;

	vec3 prior_position = vec3(0);
	vec3 initial_velocity = vec3(0);

	vec3 orientation;

	// gameplay system variables
	vec3 last_terrain_contact_normal = vec3(0, 1.f, 0);
	EEntity* grabbing_entity = nullptr;
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
	EEntity* checkpoint = nullptr;
	vec3 checkpoint_pos;

	// animation
	float anim_t = 0;                                                    // animation timer
	RPlayerAnimationState anim_state = RPlayerAnimationState::NoAnimation; // animation state
	vec3 anim_final_pos = vec3(0);                                       // final position after translation animation
	vec3 anim_orig_pos = vec3(0);                                        // original position
	vec3 anim_final_dir = vec3(0);                                       // final player orientation
	vec3 anim_orig_dir = vec3(0);                                        // original player orientation
	bool anim_finished_turning = false;                                  // player has finished turning his camera

	void Update();

	void UpdateState();
	
	vec3 GetFeetPosition() const { return position; }

	vec3 MoveForward();

	vec3 GetUpperBoundPosition() const { return -position + vec3(0.0f, height, 0.0f); }

	vec3 GetEyePosition() const { return position + vec3(0, height - 0.1f, 0); }

	float GetSpeed() const { return length(velocity); }

	float GetSpeedLimit() const;

	float GetHorizontalSpeed() const { return length(vec2(velocity.xz)); }

	vec3 GetHorizontalMovementForwardVector() const { return normalize(ToXz(velocity)); }
	
	void MultiplySpeed(float multiplier) { velocity = length(velocity) * multiplier * v_dir; }
	
	void SetSpeed(float new_speed) { velocity = new_speed * v_dir; }

	void SetHorizontalSpeed(float new_speed) { vec2 hv = new_speed * normalize(vec2(v_dir.xz)); velocity.x = hv.x; velocity.z = hv.y; }
	
	vec3 GetLastTerrainContactPoint() const;

	bool IsMovingThisFrame() const { return length(v_dir) > FloatEpsilon; }
	
	bool MaybeHurtFromFall();
	void RestoreHealth();
	void SetCheckpoint(EEntity* entity);
	void GotoCheckpoint();
	void Die();
	void BruteStop();

	void ChangeStateTo(NPlayerState new_state, RPlayerStateChangeArgs args = {});

private:
	friend struct GlobalSceneInfo;

	void UpdateAirMovement(float dt);
	
	static EPlayer* ResetPlayer();
};
