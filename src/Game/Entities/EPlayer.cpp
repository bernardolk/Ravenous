#include "game/entities/EPlayer.h"

#include "engine/catalogues.h"
#include "engine/rvn.h"
#include "engine/camera/camera.h"
#include "engine/collision/ClController.h"
#include "engine/collision/ClTypes.h"
#include "engine/io/input.h"
#include "engine/render/ImRender.h"
#include "engine/world/World.h"
#include "game/input/PlayerInput.h"

#define IfStateChange(X, Y) if (PlayerState == NPlayerState::X && NewState == NPlayerState::Y)
#define IfState(X) if (PlayerState == NPlayerState::X)

// -----------------
// Player Constants
// -----------------
const float EPlayer::Acceleration = 12.0f;
const float EPlayer::RunSpeed = 4.0f;
const float EPlayer::DashSpeed = 8.0f;
const float EPlayer::WalkSpeed = 0.95f;
const float EPlayer::FallSpeed = 0.01f;
const float EPlayer::AirSteeringVelocity = 1.5f;
const float EPlayer::MaxAirSpeed = 0.2f;
const float EPlayer::AirFriction = 5.0f;
const float EPlayer::JumpInitialSpeed = 7.0f;
const float EPlayer::JumpReducedHorizontalThrust = 1.5f;
const float EPlayer::MinimumJumpHorizontalThrustWhenDashing = 5.5f;
const float EPlayer::MinimumJumpHorizontalThrustWhenRunning = 3.0f;
const float EPlayer::JumpFromSlopeHorizontalThrust = 4.0f;
const float EPlayer::SlideJumpSpeed = 6.7f;
const float EPlayer::SlideSpeed = 2.0f;
const float EPlayer::FallFromEdgePushSpeed = 1.5f;
const vec3 EPlayer::Gravity = vec3(0, -18.0, 0);

void EPlayer::Update()
{
	static_cast<EEntity*>(this)->Update();

	if (ClUpdatePlayerWorldCells(this))
	{
		ClRecomputeCollisionBufferEntities();
	}
}

void EPlayer::UpdateState()
{
	RWorld* World = RWorld::Get();
	float Dt = RavenousEngine::GetFrameDuration();

	// -------------
	// Standing
	// -------------
	IfState(Standing)
	{
		MoveForward();
		Update();

		vec3 PlayerBtmSphereCenter = Position + vec3(0, Radius, 0);
		vec3 ContactPoint = PlayerBtmSphereCenter + (-LastTerrainContactNormal * Radius);
		RImDraw::AddLine(IMHASH, PlayerBtmSphereCenter, ContactPoint, COLOR_YELLOW_1);

		/* Current system: Here we are looping at most twice on the:
		   "Do stepover Vtrace, Adjust player's position to terrain, check collisions" loop
		   so we can detect when we try stepping up/down into a place where the player can't
		   fit in.
		*/
		// TODO: Not sure why we are looping here actually. Once I figure out, please explain why we can't run it once.
		for (int It = 0; It < 2; It ++)
		{
			auto Vtrace = ClDoStepoverVtrace(this, World);

			// snap player to the last terrain contact point detected if its a valid stepover hit
			if (Vtrace.Hit && (Vtrace.DeltaY > 0.0004 || Vtrace.DeltaY < 0))
			{
				Position.y -= Vtrace.DeltaY;
				BoundingBox.Translate(vec3(0, -Vtrace.DeltaY, 0));
			}

			// resolve collisions
			auto Results = ClTestAndResolveCollisions(this);

			// iterate on collision Results
			bool bCollidedWithTerrain = false;
			RCollisionResults Slope;
			for (auto& CollisionResult : Results)
			{
				bCollidedWithTerrain = dot(CollisionResult.Normal, UnitY) > 0;

				if (bCollidedWithTerrain)
					LastTerrainContactNormal = CollisionResult.Normal;

				bool CollidedWithSlope = dot(CollisionResult.Normal, UnitY) >= SlopeMinAngle;
				if (CollidedWithSlope && CollisionResult.Entity->Slidable)
					Slope = CollisionResult;
			}

			// if floor is no longer beneath player's feet
			if (!Vtrace.Hit)
			{
				float FallMomentumIntensity = GetSpeed() < FallFromEdgePushSpeed ? FallFromEdgePushSpeed : GetSpeed();
				vec2 FallMomentumDir = LastRecordedMovementDirection.xz;
				vec2 FallMomentum = FallMomentumDir * FallMomentumIntensity;

				Velocity = vec3(FallMomentum.x, 0, FallMomentum.y);
				ChangeStateTo(NPlayerState::Falling);
				Update();
				break;
			}

			// Collided with nothing or with terrain only, break
			if (Results.Num() == 0 || (bCollidedWithTerrain && Results.Num() == 1))
				break;

			if (Slope.Collision)
			{
				RPlayerStateChangeArgs Args;
				Args.Normal = Slope.Normal;
				ChangeStateTo(NPlayerState::Sliding, Args);
				break;
			}
		}

		// Check interactions
		if (bWantToGrab)
		{
			//GP_CheckPlayerGrabbedLedge(this, world);
			Rvn::Print("Ran check player grabbed ledge", 1000);
		}
	}

	// -------------
	// Falling
	// -------------
	else
		IfState(Falling)
		{
			UpdateAirMovement(Dt);

			Update();

			auto Results = ClTestAndResolveCollisions(this);
			for (auto& Result : Results)
			{
				// slope collision
				{
					bool bCollidedWithSlope = dot(Result.Normal, UnitY) >= SlopeMinAngle;
					if (bCollidedWithSlope && Result.Entity->Slidable)
					{
						RPlayerStateChangeArgs Args;
						Args.Normal = Result.Normal;
						ChangeStateTo(NPlayerState::Sliding, Args);
						return;
					}
				}

				// floor collision
				{
					// If collided with terrain
					if (dot(Result.Normal, UnitY) > 0)
					{
						ChangeStateTo(NPlayerState::Standing);
						return;
					}
				}

				// else
				{
					ClWallSlidePlayer(this, Result.Normal);
				}
			}
		}

		// -------------
		// Jumping
		// -------------
		else
			IfState(Jumping)
			{
				UpdateAirMovement(Dt);

				Update();

				auto Results = ClTestAndResolveCollisions(this);
				for (auto& Result : Results)
				{
					// collision with terrain while jumping should be super rare I guess ...
					// slope collision
					bool CollidedWithSlope = dot(Result.Normal, UnitY) >= SlopeMinAngle;
					if (CollidedWithSlope && Result.Entity->Slidable)
					{
						RPlayerStateChangeArgs Args;
						Args.Normal = Result.Normal;
						ChangeStateTo(NPlayerState::Sliding, Args);
						return;
					}


					// floor collision 
					// If collided with terrain
					else if (dot(Result.Normal, UnitY) > 0)
					{
						ChangeStateTo(NPlayerState::Standing);
						return;
					}

					else
						ClWallSlidePlayer(this, Result.Normal);
				}

				// @todo - need to include case that player touches inclined terrain
				//          in that case it should also stand (or fall from ledge) and not
				//          directly fall.
				if (Results.Num() > 0)
					ChangeStateTo(NPlayerState::Falling);

				else if (Velocity.y <= 0)
					ChangeStateTo(NPlayerState::Falling);
			}

			// -------------
			// Sliding
			// -------------
			else
				IfState(Sliding)
				{
					RImDraw::AddLine(IMHASH, Position, Position + 1.f * SlidingDirection, COLOR_RED_2);

					Velocity = VDir * SlideSpeed;

					float FrameDuration = RavenousEngine::GetFrameDuration();
					Position += Velocity * FrameDuration;
					Update();

					// RESOLVE COLLISIONS AND CHECK FOR TERRAIN CONTACT
					auto Results = ClTestAndResolveCollisions(this);

					bool CollidedWithTerrain = false;
					for (auto& Result : Results)
					{
						// iterate on collision Results
						CollidedWithTerrain = dot(Result.Normal, UnitY) > 0;
						if (CollidedWithTerrain)
							LastTerrainContactNormal = Result.Normal;
					}

					if (CollidedWithTerrain)
					{
						ChangeStateTo(NPlayerState::Standing);
						return;
					}

					auto Vtrace = ClDoStepoverVtrace(this, World);
					if (!Vtrace.Hit)
					{
						ChangeStateTo(NPlayerState::Falling);
					}
				}
}

void EPlayer::UpdateAirMovement(float DeltaTime)
{
	Velocity += Gravity * DeltaTime;

	vec3 JumpingDirection = GetHorizontalMovementForwardVector();
	float HSpeed = GetHorizontalSpeed();

	// Air steering
	if (bPressingLeftWhileInAir)
	{
		vec3 RelativeMotionVec = IsEqual(HSpeed, 0) ? VDir : -normalize(ToXz(Cross(JumpingDirection, UnitY)));
		Position += RelativeMotionVec * AirSteeringVelocity * DeltaTime;
	}
	if (bPressingRightWhileInAir)
	{
		vec3 RelativeMotionVec = IsEqual(HSpeed, 0) ? VDir : normalize(ToXz(Cross(JumpingDirection, UnitY)));
		Position += RelativeMotionVec * AirSteeringVelocity * DeltaTime;
	}
	// Extra steering if jumping upwards
	if ((PlayerState == NPlayerState::Jumping && IsEqual(HSpeed, 0)) || PlayerState == NPlayerState::Falling)
	{
		if (bPressingForwardWhileInAir || bPressingBackwardWhileInAir)
			Position += VDir * AirSteeringVelocity * DeltaTime;
	}

	// Friction
	HSpeed = GetHorizontalSpeed();
	if (bStoppedPressingForwardWhileInAir && HSpeed > JumpReducedHorizontalThrust)
	{
		vec3 Friction = JumpingDirection * AirFriction * DeltaTime;
		Velocity -= Friction;
		HSpeed = GetHorizontalSpeed();
		if (HSpeed < JumpReducedHorizontalThrust)
		{
			vec3 HorizontalVelocity = JumpReducedHorizontalThrust * JumpingDirection;
			Velocity.x = HorizontalVelocity.x;
			Velocity.z = HorizontalVelocity.z;
		}
	}

	Position += Velocity * DeltaTime + Gravity * DeltaTime * DeltaTime / 2.f;
}

void EPlayer::ChangeStateTo(NPlayerState NewState, RPlayerStateChangeArgs Args)
{
	IfStateChange(Jumping, Falling)
	{
		PlayerState = NPlayerState::Falling;
		Velocity.y = 0;
		// jumping_upwards = false;
	}

	else
		IfStateChange(Standing, Jumping)
		{
			// If jumping up
			if (!IsMovingThisFrame() && GetSpeed() == 0)
			{
				BruteStop();
			}
			else if (bPressingForwardWhileStanding)
			{
				float HorizontalThrust = GetHorizontalSpeed();
				if (bDashing && HorizontalThrust < MinimumJumpHorizontalThrustWhenDashing)
					HorizontalThrust = MinimumJumpHorizontalThrustWhenDashing;
				else if (!bDashing && HorizontalThrust < MinimumJumpHorizontalThrustWhenRunning)
					HorizontalThrust = MinimumJumpHorizontalThrustWhenRunning;

				auto* PlayerCamera = RCameraManager::Get()->GetGameCamera();
				Velocity = PlayerCamera->Front * HorizontalThrust;
			}

			Velocity.y = JumpInitialSpeed;
			PlayerState = NPlayerState::Jumping;
			AnimState = RPlayerAnimationState::Jumping;
			HeightBeforeFall = Position.y;
		}

		else
			IfStateChange(Standing, Falling)
			{
				PlayerState = NPlayerState::Falling;
				Velocity.y = -1 * FallSpeed;
				Velocity.x *= 0.5;
				Velocity.z *= 0.5;
				HeightBeforeFall = Position.y;
			}

			else
				IfStateChange(Falling, Standing)
				{
					Velocity.y = 0;

					PlayerState = NPlayerState::Standing;

					// conditional animation: if falling from jump, land, else, land from fall
					if (Height < Height) // TODO: WTF??
						AnimState = RPlayerAnimationState::Landing;
					else
						AnimState = RPlayerAnimationState::LandingFall;

					MaybeHurtFromFall();
				}

				else
					IfStateChange(Jumping, Standing)
					{
						Velocity.y = 0;

						MultiplySpeed(0.f);

						PlayerState = NPlayerState::Standing;
						AnimState = RPlayerAnimationState::Landing;
						//MaybeHurtFromFall();
					}

					else
						IfStateChange(Standing, Sliding)
						{
							/* Parameters:
							   - vec3 normal : the normal of the slope (collider triangle) player is currently sliding
							*/

							PlayerState = NPlayerState::Sliding;

							auto DownVecIntoN = ProjectVecIntoRef(-UnitY, Args.Normal);
							auto SlidingDirection = normalize(-UnitY - DownVecIntoN);
							SlidingDirection = SlidingDirection;
							SlidingNormal = Args.Normal;
						}

						else
							IfStateChange(Standing, SlideFalling)
							{
								// make player 'snap' to slope velocity-wise
								// velocity = SlideSpeed * ramp->collision_geometry.slope.tangent;

								PlayerState = NPlayerState::SlideFalling;
							}

							else
								IfStateChange(Jumping, Grabbing)
								{
									// vec3 rev_normal = rev_2Dnormal(normal_vec);

									// // this will be an animation in the future
									// float turn_angle = glm::degrees(vector_angle_signed(to2d_xz(pCam->Front), normal_vec)) - 180;
									// camera_change_direction(pCam, turn_angle, 0.f);
									// // CL_snap_player(player, normal_vec, penetration);

									// player_state          = PlayerState::Grabbing;
									// grabbing_entity       = entity;
									// velocity  = vec3(0);
									// // after we are able to move while grabbing the ledge, this should move away from here
									// {
									//    anim_final_dir        = rev_normal;
									//    anim_final_pos        = FinalPosition;
									//    anim_orig_pos         = position;
									//    anim_orig_dir         = normalize(to_xz(pCam->Front));
									//    velocity  = vec3(0);
									// }
								}

								else
									IfStateChange(Grabbing, Vaulting)
									{
										PlayerState = NPlayerState::Vaulting;
										AnimState = RPlayerAnimationState::Vaulting;
										GrabbingEntity = nullptr;
									}

									else
										IfStateChange(Standing, Vaulting)
										{
											auto* GII = GlobalInputInfo::Get();
											GII->BlockMouseMove = true;
											auto* PlayerCamera = RCameraManager::Get()->GetGameCamera();

											PlayerState = NPlayerState::Vaulting;
											AnimState = RPlayerAnimationState::Vaulting;
											Velocity = vec3(0);

											AnimOrigPos = Position;
											AnimOrigDir = normalize(ToXz(PlayerCamera->Front));

											auto InwardNormal = normalize(Cross(Args.VaultingData.Ledge.A - Args.VaultingData.Ledge.B, UnitY));
											AnimFinalPos = Args.VaultingData.FinalPosition;
											AnimFinalDir = InwardNormal;
										}

										else
											IfStateChange(Vaulting, Standing)
											{
												auto* GII = GlobalInputInfo::Get();

												GII->ForgetLastMouseCoords = true;
												GII->BlockMouseMove = false;
												PlayerState = NPlayerState::Standing;
												AnimFinishedTurning = false;
											}

											else
												IfStateChange(Sliding, Standing)
												{
													SlidingDirection = vec3(0);
													SlidingNormal = vec3(0);
													PlayerState = NPlayerState::Standing;
												}

												else
													IfStateChange(Sliding, Jumping)
													{
														VDir = SlidingNormal;
														SlidingNormal = vec3(0);

														Velocity = VDir * JumpFromSlopeHorizontalThrust;
														Velocity.y = JumpInitialSpeed;

														PlayerState = NPlayerState::Jumping;
														AnimState = RPlayerAnimationState::Jumping;
														HeightBeforeFall = Position.y;
													}

													else
														IfStateChange(Sliding, Falling)
														{
															PlayerState = NPlayerState::Falling;
															Velocity.y = -FallSpeed;
															HeightBeforeFall = Position.y;
														}

														else
															FatalError("There is no link to change Player State from %i to %i.", PlayerState, NewState);
}

vec3 EPlayer::MoveForward()
{
	bool NoMoveCommand = VDir.x == 0 && VDir.z == 0;

	float DeltaTime = RavenousEngine::GetFrameDuration();

	// Limiting movement angle when moving in diagonals
	auto* PlayerCamera = RCameraManager::Get()->GetGameCamera();

	// TODO: Implement for the other axis as well
	//		Cleanup the mess of using KEYs for this
	//		Cleanup the mess on process input code
	//		Maybe this should be moved to where we set v_dir
	if (FirstPressedMovementKeyWhileStanding == RGameInputKey::MoveForward)
	{
		if (bPressingLeftWhileStanding)
			VDir = normalize(rotate(PlayerCamera->Front, PI / 10.f, UnitY));
		else if (bPressingRightWhileStanding)
			VDir = normalize(rotate(PlayerCamera->Front, PI / 10.f, -UnitY));
	}


	if (!IsEqual(length(VDir), 0))
		LastRecordedMovementDirection = VDir;
	else if (LastRecordedMovementDirection == vec3(0))
		LastRecordedMovementDirection = normalize(ToXz(RCameraManager::Get()->GetGameCamera()->Front));

	float DSpeed = Acceleration * DeltaTime;

	// If stopped
	if (GetSpeed() > 0 && NoMoveCommand)
		DSpeed = 0;

	SetSpeed(GetSpeed() + DSpeed);
	
	float SpeedLimit = GetSpeedLimit();
	if (GetSpeed() > SpeedLimit)
		SetSpeed(SpeedLimit);

	// update things
	vec3 NextPosition = Position + Velocity * DeltaTime;
	Position = NextPosition;
	return NextPosition;
}

vec3 EPlayer::GetLastTerrainContactPoint() const
{
	const vec3 PlayerBtmSphereCenter = Position + vec3(0, Radius, 0);
	return PlayerBtmSphereCenter + -LastTerrainContactNormal * Radius;
}

bool EPlayer::MaybeHurtFromFall()
{
	float FallHeight = HeightBeforeFall - Position.y;
	FallHeightLog = FallHeight;
	if (FallHeight >= HurtHeight2)
	{
		Lives -= 2;
		return true;
	}
	if (FallHeight >= HurtHeight1)
	{
		Lives -= 1;
		return true;
	}
	return false;
}

void EPlayer::RestoreHealth()
{
	Lives = InitialLives;
}

void EPlayer::SetCheckpoint(EEntity* Entity)
{
	return;
	/*
	if (entity->type != EntityType_Checkpoint)
		assert(false);

	checkpoint_pos = position;
	checkpoint = entity;
	*/
}

void EPlayer::GotoCheckpoint()
{
	Position = CheckpointPos;
}

void EPlayer::Die()
{
	Lives = InitialLives;
	Velocity = vec3(0);
	PlayerState = NPlayerState::Standing;
	ForceInterruptPlayerAnimation(this);
	GotoCheckpoint();
}

void EPlayer::BruteStop()
{
	// bypass deaceleration steps. Stops player right on his tracks.
	Velocity = vec3(0);
}

EPlayer* EPlayer::ResetPlayer()
{
	auto* Player = Get();
	new (Player) EPlayer;
	Initialize(Player);
	return Player;
}

void EPlayer::Initialize(EPlayer* Player)
{
	// Set player's assets
	REntityAttributes Attrs;
	Attrs.Name = "Player";
	Attrs.Mesh = "capsule";
	Attrs.Shader = "model";
	Attrs.Texture = "pink";
	Attrs.CollisionMesh = "capsule";
	Attrs.Scale = vec3(1);

	SetEntityAssets(Player, Attrs);
}

float EPlayer::GetSpeedLimit() const
{
	if (bDashing)
	{
		return DashSpeed;
	}
	else if (bWalking)
	{
		return WalkSpeed;
	}
	else
	{
		return RunSpeed;
	}
}

void GpCheckPlayerGrabbedLedge(EPlayer* Player, RWorld* World)
{
	RLedge Ledge = ClPerformLedgeDetection(Player, World);
	if (Ledge.Empty)
		return;
	
	vec3 Position = ClGetFinalPositionLedgeVaulting(Player, Ledge);

	RPlayerStateChangeArgs Args;
	Args.VaultingData.Ledge = Ledge;
	Args.VaultingData.FinalPosition = Position;
	Player->ChangeStateTo(NPlayerState::Vaulting, Args);
}
