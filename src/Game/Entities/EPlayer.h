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
	Standing = 100,

	// Air based states
	Falling = 200,
	Jumping = 201,

	// Partially uncontrolled movement states
	Sliding      = 300,
	SlideFalling = 301,

	// Special movement states
	Grabbing = 400,
	Vaulting = 401,
};

struct RPlayerStateChangeArgs
{
	// collision
	EEntity* Entity = nullptr;
	vec3 Normal = vec3(0);
	float Penetration = 0;

	union
	{
		// grabbing info
		struct VaultingData
		{
			vec3 FinalPosition;
			RLedge Ledge;
		} VaultingData;
	};

	RPlayerStateChangeArgs() :
		VaultingData() {}
};

enum class RPlayerAnimationState
{
	NoAnimation,
	Jumping,
	Landing,
	LandingFall,
	Vaulting
};

void ForceInterruptPlayerAnimation(EPlayer* Player);


struct EntityType(EPlayer)
{
	Reflected()
	DeclSingleton(EPlayer)
	// geometry
	float radius = 0.2f;
	float height = 1.75f;

	// movement variables
	vec3 v_dir = vec3(0.f);                            // intended movement direction
	vec3 last_recorded_movement_direction = vec3(0.f); // last non zero movement direction

	// movement constants
	static const float Acceleration;
	static const float RunSpeed;
	static const float DashSpeed;
	static const float WalkSpeed;
	static const float FallSpeed;
	static const float AirSteeringVelocity;
	static const float MaxAirSpeed;
	static const float AirFriction;
	static const float JumpInitialSpeed;
	static const float JumpReducedHorizontalThrust;
	static const float JumpFromSlopeHorizontalThrust;
	static const float MinimumJumpHorizontalThrustWhenDashing;
	static const float MinimumJumpHorizontalThrustWhenRunning;
	static const float SlideJumpSpeed;
	static const float SlideSpeed;
	static const float FallFromEdgePushSpeed;
	static const vec3 Gravity;

	// other constants
	static constexpr float SlopeMinAngle = 0.4;

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
	float anim_t = 0;                                                      // animation timer
	RPlayerAnimationState anim_state = RPlayerAnimationState::NoAnimation; // animation state
	vec3 anim_final_pos = vec3(0);                                         // final position after translation animation
	vec3 anim_orig_pos = vec3(0);                                          // original position
	vec3 anim_final_dir = vec3(0);                                         // final player orientation
	vec3 anim_orig_dir = vec3(0);                                          // original player orientation
	bool anim_finished_turning = false;                                    // player has finished turning his camera

	void Update();

	void UpdateState();

	vec3 GetFeetPosition() const { return Position; }

	vec3 MoveForward();

	vec3 GetUpperBoundPosition() const { return -Position + vec3(0.0f, height, 0.0f); }

	vec3 GetEyePosition() const { return Position + vec3(0, height - 0.1f, 0); }

	float GetSpeed() const { return length(Velocity); }

	float GetSpeedLimit() const;

	float GetHorizontalSpeed() const { return length(vec2(Velocity.xz)); }

	vec3 GetHorizontalMovementForwardVector() const { return normalize(ToXz(Velocity)); }

	void MultiplySpeed(float Multiplier) { Velocity = length(Velocity) * multiplier * v_dir; }

	void SetSpeed(float NewSpeed) { Velocity = new_speed * v_dir; }

	void SetHorizontalSpeed(float NewSpeed)
	{
		vec2 Hv = new_speed * normalize(vec2(v_dir.xz));
		Velocity.x = hv.x;
		Velocity.z = hv.y;
	}

	vec3 GetLastTerrainContactPoint() const;

	bool IsMovingThisFrame() const { return length(v_dir) > FloatEpsilon; }

	bool MaybeHurtFromFall();
	void RestoreHealth();
	void SetCheckpoint(EEntity* Entity);
	void GotoCheckpoint();
	void Die();
	void BruteStop();

	void ChangeStateTo(NPlayerState NewState, RPlayerStateChangeArgs Args = {});

private:
	friend struct GlobalSceneInfo;

	void UpdateAirMovement(float Dt);

	static EPlayer* ResetPlayer();
};
