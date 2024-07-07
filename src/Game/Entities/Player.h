#pragma once

#include "Checkpoint.h"
#include "engine/core/core.h"
#include "engine/entities/Entity.h"
#include "engine/utils/utils.h"
#include "engine/collision/ClEdgeDetection.h"

void ForceInterruptPlayerAnimation(EPlayer* Player);

/* ==========================================
 *	PlayerState (Enum)
 * ========================================== */
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

/* ==========================================
 *	PlayerStateChangeArgs
 * ========================================== */
struct RPlayerStateChangeArgs
{
	// collision
	//@entityptr
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

/* ==========================================
 *	PlayerAnimationState
 * ========================================== */
enum class RPlayerAnimationState
{
	NoAnimation,
	Jumping,
	Landing,
	LandingFall,
	Vaulting
};

/* ==========================================
 *	Player
 *		Player singleton entity
 * ========================================== */
struct EntityType(EPlayer)
{
	Reflected(EPlayer)

	friend struct GlobalSceneInfo;
	template<typename TEntity> friend EHandle<TEntity> SpawnEntity();

	// Player ID	
	static inline RUUID PlayerID = 1;				// ID = 00000-00000-00000-00001 is reserved to Player
	
	//Capsule Geometry
	float Radius = 0.2f;
	float Height = 1.75f;

	// Movement
	vec3 VDir = vec3(0.f);								// intended movement direction
	vec3 LastRecordedMovementDirection = vec3(0.f);		// last non zero movement direction

	// Constants
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

	// Other Constants (TODO: Move)
	static constexpr float SlopeMinAngle = 0.4f;

	// Movement States
	// TODO: Turn into flags
	bool bDashing = false;
	bool bWalking = false;
	bool bJumpingUpwards = false;
	bool bLanding = false;
	bool bJumpingFromSlope = false;
	bool bAction = false;
	bool bWantToGrab = false;
	bool bDodgeButton = false;
	bool bInteractButton = false;
	bool bPressingForwardWhileInAir = false;
	bool bStoppedPressingForwardWhileInAir = false;
	bool bPressingLeftWhileInAir = false;
	bool bPressingRightWhileInAir = false;
	bool bPressingBackwardWhileInAir = false;
	bool bPressingForwardWhileStanding = false;

	bool bPressingLeftWhileStanding = false;
	bool bPressingRightWhileStanding = false;
	bool bPressingBackwardWhileStanding = false;

	uint64 FirstPressedMovementKeyWhileStanding = 0;

	NPlayerState PlayerState;
	NPlayerState InitialPlayerState;
	vec3 PlayerInitialPosition = vec3{0.0f};

	vec3 PriorPosition = vec3{0.0f};
	vec3 InitialVelocity = vec3{0.0f};
	
	// Random Gameplay Systems Data
	vec3 LastTerrainContactNormal = vec3(0, 1.f, 0);
	EEntity* GrabbingEntity = nullptr;
	float GrabReach = 0.9f; // radius + arms reach, 0.5 + 0.4  

	// Sliding
	vec3 SlidingDirection = vec3{0.0f};
	vec3 SlidingNormal = vec3{0.0f};

	// Health
	int InitialLives = 2;
	int Lives = 2;

	// Fall damage
	float HurtHeight1 = 5.0f;
	float HurtHeight2 = 8.0f;
	float HeightBeforeFall = 0.0f;
	float FallHeightLog = 0.0f;

	// Checkpoints
	Handle(ECheckpoint, Checkpoint); 
	
	// Animation
	float AnimT = 0;														// animation timer
	RPlayerAnimationState AnimState = RPlayerAnimationState::NoAnimation;	// animation state
	vec3 AnimFinalPos = vec3{0.0f};                                         // final position after translation animation
	vec3 AnimOrigPos = vec3{0.0f};                                          // original position
	vec3 AnimFinalDir = vec3{0.0f};                                         // final player orientation
	vec3 AnimOrigDir = vec3{0.0f};                                          // original player orientation
	bool AnimFinishedTurning = false;										// player has finished turning his camera

	// =======================
	// Methods
	// =======================
	static EPlayer* Get() { return Instance; }
	
	void Update();
	void UpdateState();
	vec3 GetFeetPosition() const { return Position; }
	vec3 MoveForward();
	
	vec3 GetUpperBoundPosition() const { return -Position + vec3(0.0f, Height, 0.0f); }
	vec3 GetEyePosition() const { return Position + vec3(0, Height - 0.1f, 0); }

	float GetSpeed() const { return length(Velocity); }
	float GetSpeedLimit() const;
	float GetHorizontalSpeed() const { return length(vec2(Velocity.xz)); }

	vec3 GetHorizontalMovementForwardVector() const { return normalize(ToXZ(Velocity)); }

	void MultiplySpeed(float Multiplier) { Velocity = length(Velocity) * Multiplier * VDir; }
	void SetSpeed(float NewSpeed) { Velocity = NewSpeed * VDir; }
	void SetHorizontalSpeed(float NewSpeed)
	{
		vec2 HorizontalVelocity = NewSpeed * normalize(vec2(VDir.xz));
		Velocity.x = HorizontalVelocity.x;
		Velocity.z = HorizontalVelocity.y;
	}

	vec3 GetLastTerrainContactPoint() const;

	bool IsMovingThisFrame() const { return length(VDir) > FloatEpsilon; }

	void MaybeHurtFromFall();
	void RestoreHealth();
	void GotoCheckpoint();
	void Die();
	void BruteStop();

	void ChangeStateTo(NPlayerState NewState, RPlayerStateChangeArgs Args = {});

	// TO BE USED ONLY BY SERIALIZATION CODE
	static void Initialize(EEntity* PlayerEntity);

private:
	inline static EPlayer* Instance = nullptr;

	EPlayer();
	void UpdateAirMovement(float Dt);
};
