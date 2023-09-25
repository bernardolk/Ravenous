#include "game/entities/EPlayer.h"

#include "engine/rvn.h"
#include "engine/camera/camera.h"
#include "engine/collision/ClController.h"
#include "engine/collision/ClTypes.h"
#include "engine/io/input.h"
#include "engine/render/ImRender.h"
#include "engine/world/World.h"
#include "game/input/PlayerInput.h"

#define IfStateChange(X, Y) if (player_state == NPlayerState::X && new_state == NPlayerState::Y)
#define IfState(X) if (player_state == NPlayerState::X)

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

EPlayer::EPlayer() = default;

void EPlayer::Update()
{
	static_cast<EEntity*>(this)->Update();

	if (CL_UpdatePlayerWorldCells(this))
	{
		CL_RecomputeCollisionBufferEntities();
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

		vec3 PlayerBtmSphereCenter = Position + vec3(0, radius, 0);
		vec3 ContactPoint = player_btm_sphere_center + -last_terrain_contact_normal * radius;
		RImDraw::AddLine(IMHASH, player_btm_sphere_center, contact_point, COLOR_YELLOW_1);

		/* Current system: Here we are looping at most twice on the:
		   "Do stepover Vtrace, Adjust player's position to terrain, check collisions" loop
		   so we can detect when we try stepping up/down into a place where the player can't
		   fit in.
		*/
		// TODO: Not sure why we are looping here actually. Once I figure out, please explain why we can't run it once.
		for (int It = 0; It < 2; It ++)
		{
			auto Vtrace = CL_DoStepoverVtrace(this, World);

			// snap player to the last terrain contact point detected if its a valid stepover hit
			if (vtrace.hit && (vtrace.delta_y > 0.0004 || vtrace.delta_y < 0))
			{
				Position.y -= vtrace.delta_y;
				BoundingBox.Translate(vec3(0, -vtrace.delta_y, 0));
			}

			// resolve collisions
			auto Results = CL_TestAndResolveCollisions(this);

			// iterate on collision results
			bool CollidedWithTerrain = false;
			RCollisionResults Slope;
			for (auto& collision_result : results)
			{
				collided_with_terrain = dot(collision_result.normal, UnitY) > 0;

				if (collided_with_terrain)
					last_terrain_contact_normal = collision_result.normal;

				bool collided_with_slope = dot(collision_result.normal, UnitY) >= slope_min_angle;
				if (collided_with_slope && collision_result.entity->slidable)
					slope = collision_result;
			}

			// if floor is no longer beneath player's feet
			if (!vtrace.hit)
			{
				float FallMomentumIntensity = GetSpeed() < fall_from_edge_push_speed ? fall_from_edge_push_speed : GetSpeed();
				vec2 FallMomentumDir = last_recorded_movement_direction.xz;
				vec2 FallMomentum = fall_momentum_dir * fall_momentum_intensity;

				Velocity = vec3(fall_momentum.x, 0, fall_momentum.y);
				ChangeStateTo(NPlayerState::Falling);
				Update();
				break;
			}

			// Collided with nothing or with terrain only, break
			if (results.Num() == 0 || (CollidedWithTerrain && results.Num() == 1))
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
		if (want_to_grab)
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

			auto Results = CL_TestAndResolveCollisions(this);
			for (auto& result : results)
			{
				// slope collision
				{
					bool collided_with_slope = dot(result.normal, UnitY) >= slope_min_angle;
					if (collided_with_slope && result.entity->slidable)
					{
						RPlayerStateChangeArgs args;
						args.Normal = result.normal;
						ChangeStateTo(NPlayerState::Sliding, args);
						return;
					}
				}

				// floor collision
				{
					// If collided with terrain
					if (dot(result.normal, UnitY) > 0)
					{
						ChangeStateTo(NPlayerState::Standing);
						return;
					}
				}

				// else
				{
					CL_WallSlidePlayer(this, result.normal);
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

				auto Results = CL_TestAndResolveCollisions(this);
				for (auto& result : results)
				{
					// collision with terrain while jumping should be super rare I guess ...
					// slope collision
					bool collided_with_slope = dot(result.normal, UnitY) >= slope_min_angle;
					if (collided_with_slope && result.entity->slidable)
					{
						RPlayerStateChangeArgs args;
						args.Normal = result.normal;
						ChangeStateTo(NPlayerState::Sliding, args);
						return;
					}


					// floor collision 
					// If collided with terrain
					else if (dot(result.normal, UnitY) > 0)
					{
						ChangeStateTo(NPlayerState::Standing);
						return;
					}

					else
						CL_WallSlidePlayer(this, result.normal);
				}

				// @todo - need to include case that player touches inclined terrain
				//          in that case it should also stand (or fall from ledge) and not
				//          directly fall.
				if (results.Num() > 0)
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
					RImDraw::AddLine(IMHASH, Position, Position + 1.f * sliding_direction, COLOR_RED_2);

					Velocity = v_dir * slide_speed;

					float FrameDuration = RavenousEngine::GetFrameDuration();

					Position += Velocity * frame_duration;
					Update();


					// RESOLVE COLLISIONS AND CHECK FOR TERRAIN CONTACT
					auto Results = CL_TestAndResolveCollisions(this);

					bool CollidedWithTerrain = false;
					for (auto& result : results)
					{
						// iterate on collision results
						collided_with_terrain = dot(result.normal, UnitY) > 0;
						if (collided_with_terrain)
							last_terrain_contact_normal = result.normal;
					}

					if (CollidedWithTerrain)
					{
						ChangeStateTo(NPlayerState::Standing);
						return;
					}

					auto Vtrace = CL_DoStepoverVtrace(this, World);
					if (!vtrace.hit)
					{
						ChangeStateTo(NPlayerState::Falling);
					}
				}
}

void EPlayer::UpdateAirMovement(float Dt)
{
	Velocity += gravity * dt;

	vec3 JumpingDirection = GetHorizontalMovementForwardVector();
	float HSpeed = GetHorizontalSpeed();

	// Air steering
	if (pressing_left_while_in_air)
	{
		vec3 RelativeMotionVec = IsEqual(h_speed, 0) ? v_dir : -normalize(ToXz(Cross(jumping_direction, UnitY)));
		Position += relative_motion_vec * air_steering_velocity * dt;
	}
	if (pressing_right_while_in_air)
	{
		vec3 RelativeMotionVec = IsEqual(h_speed, 0) ? v_dir : normalize(ToXz(Cross(jumping_direction, UnitY)));
		Position += relative_motion_vec * air_steering_velocity * dt;
	}
	// Extra steering if jumping upwards
	if ((player_state == NPlayerState::Jumping && IsEqual(HSpeed, 0)) || player_state == NPlayerState::Falling)
	{
		if (pressing_forward_while_in_air || pressing_backward_while_in_air)
			Position += v_dir * air_steering_velocity * dt;
	}

	// Friction
	HSpeed = GetHorizontalSpeed();
	if (stopped_pressing_forward_while_in_air && HSpeed > jump_reduced_horizontal_thrust)
	{
		vec3 Friction = jumping_direction * air_friction * dt;
		Velocity -= friction;
		HSpeed = GetHorizontalSpeed();
		if (HSpeed < jump_reduced_horizontal_thrust)
		{
			vec3 Hv = jump_reduced_horizontal_thrust * jumping_direction;
			Velocity.x = hv.x;
			Velocity.z = hv.z;
		}
	}

	Position += Velocity * dt + gravity * dt * dt / 2.f;
}

void EPlayer::ChangeStateTo(NPlayerState new_state, RPlayerStateChangeArgs Args)
{
	IfStateChange(Jumping, Falling)
	{
		player_state = NPlayerState::Falling;
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
			else if (pressing_forward_while_standing)
			{
				float HorizontalThrust = GetHorizontalSpeed();
				if (dashing && HorizontalThrust < minimum_jump_horizontal_thrust_when_dashing)
					HorizontalThrust = minimum_jump_horizontal_thrust_when_dashing;
				else if (!dashing && HorizontalThrust < minimum_jump_horizontal_thrust_when_running)
					HorizontalThrust = minimum_jump_horizontal_thrust_when_running;

				auto* PlayerCamera = RCameraManager::Get()->GetGameCamera();
				Velocity = player_camera->front * horizontal_thrust;
			}

			Velocity.y = jump_initial_speed;
			player_state = NPlayerState::Jumping;
			anim_state = RPlayerAnimationState::Jumping;
			height_before_fall = Position.y;
		}

		else
			IfStateChange(Standing, Falling)
			{
				player_state = NPlayerState::Falling;
				Velocity.y = -1 * fall_speed;
				Velocity.x *= 0.5;
				Velocity.z *= 0.5;
				height_before_fall = Position.y;
			}

			else
				IfStateChange(Falling, Standing)
				{
					Velocity.y = 0;

					player_state = NPlayerState::Standing;

					// conditional animation: if falling from jump, land, else, land from fall
					if (height < height) // TODO: ??
						anim_state = RPlayerAnimationState::Landing;
					else
						anim_state = RPlayerAnimationState::LandingFall;

					MaybeHurtFromFall();
				}

				else
					IfStateChange(Jumping, Standing)
					{
						Velocity.y = 0;

						MultiplySpeed(0.f);

						player_state = NPlayerState::Standing;
						anim_state = RPlayerAnimationState::Landing;
						//MaybeHurtFromFall();
					}

					else
						IfStateChange(Standing, Sliding)
						{
							/* Parameters:
							   - vec3 normal : the normal of the slope (collider triangle) player is currently sliding
							*/

							player_state = NPlayerState::Sliding;

							auto DownVecIntoN = ProjectVecIntoRef(-UnitY, args.normal);
							auto SlidingDirection = normalize(-UnitY - down_vec_into_n);
							sliding_direction = sliding_direction;
							sliding_normal = args.normal;
						}

						else
							IfStateChange(Standing, SlideFalling)
							{
								// make player 'snap' to slope velocity-wise
								// velocity = slide_speed * ramp->collision_geometry.slope.tangent;

								player_state = NPlayerState::SlideFalling;
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
									//    anim_final_pos        = final_position;
									//    anim_orig_pos         = position;
									//    anim_orig_dir         = normalize(to_xz(pCam->Front));
									//    velocity  = vec3(0);
									// }
								}

								else
									IfStateChange(Grabbing, Vaulting)
									{
										player_state = NPlayerState::Vaulting;
										anim_state = RPlayerAnimationState::Vaulting;
										grabbing_entity = nullptr;
									}

									else
										IfStateChange(Standing, Vaulting)
										{
											auto* GII = GlobalInputInfo::Get();
											GII->BlockMouseMove = true;
											auto* PlayerCamera = RCameraManager::Get()->GetGameCamera();

											player_state = NPlayerState::Vaulting;
											anim_state = RPlayerAnimationState::Vaulting;
											Velocity = vec3(0);

											anim_orig_pos = Position;
											anim_orig_dir = normalize(ToXz(player_camera->front));

											auto InwardNormal = normalize(Cross(args.vaulting_data.ledge.a - args.vaulting_data.ledge.b, UnitY));
											anim_final_pos = args.vaulting_data.final_position;
											anim_final_dir = inward_normal;
										}

										else
											IfStateChange(Vaulting, Standing)
											{
												auto* GII = GlobalInputInfo::Get();

												GII->ForgetLastMouseCoords = true;
												GII->BlockMouseMove = false;
												player_state = NPlayerState::Standing;
												anim_finished_turning = false;
											}

											else
												IfStateChange(Sliding, Standing)
												{
													sliding_direction = vec3(0);
													sliding_normal = vec3(0);
													player_state = NPlayerState::Standing;
												}

												else
													IfStateChange(Sliding, Jumping)
													{
														v_dir = sliding_normal;
														sliding_normal = vec3(0);

														Velocity = v_dir * jump_from_slope_horizontal_thrust;
														Velocity.y = jump_initial_speed;

														player_state = NPlayerState::Jumping;
														anim_state = RPlayerAnimationState::Jumping;
														height_before_fall = Position.y;
													}

													else
														IfStateChange(Sliding, Falling)
														{
															player_state = NPlayerState::Falling;
															Velocity.y = -fall_speed;
															height_before_fall = Position.y;
														}

														else
															fatal_error("There is no link to change Player State from %i to %i.", player_state, new_state);
}

vec3 EPlayer::MoveForward()
{
	bool NoMoveCommand = v_dir.x == 0 && v_dir.z == 0;

	float Dt = RavenousEngine::GetFrameDuration();

	// Limiting movement angle when moving in diagonals
	auto* PlayerCamera = RCameraManager::Get()->GetGameCamera();

	// TODO: Implement for the other axis as well
	//		Cleanup the mess of using KEYs for this
	//		Cleanup the mess on process input code
	//		Maybe this should be moved to where we set v_dir
	if (first_pressed_movement_key_while_standing == KEY_MOVE_UP)
	{
		if (pressing_left_while_standing)
			v_dir = normalize(rotate(player_camera->front, PI / 10.f, UnitY));
		else if (pressing_right_while_standing)
			v_dir = normalize(rotate(player_camera->front, PI / 10.f, -UnitY));
	}


	if (!IsEqual(length(v_dir), 0))
		last_recorded_movement_direction = v_dir;
	else if (last_recorded_movement_direction == vec3(0))
		last_recorded_movement_direction = normalize(ToXz(RCameraManager::Get()->GetGameCamera()->Front));

	float DSpeed = acceleration * Dt;

	// If stopped
	if (GetSpeed() > 0 && NoMoveCommand)
		DSpeed = 0;

	SetSpeed(GetSpeed() + DSpeed);

	float SpeedLimit = GetSpeedLimit();

	if (GetSpeed() > SpeedLimit)
		SetSpeed(SpeedLimit);

	vec3 NextPosition = Position + Velocity * dt;

	// update things
	Position = next_position;
	return next_position;
}

vec3 EPlayer::GetLastTerrainContactPoint() const
{
	const vec3 PlayerBtmSphereCenter = Position + vec3(0, radius, 0);
	return player_btm_sphere_center + -last_terrain_contact_normal * radius;
}

bool EPlayer::MaybeHurtFromFall()
{
	float FallHeight = height_before_fall - Position.y;
	fall_height_log = FallHeight;
	if (FallHeight >= hurt_height_2)
	{
		lives -= 2;
		return true;
	}
	if (FallHeight >= hurt_height_1)
	{
		lives -= 1;
		return true;
	}
	return false;
}

void EPlayer::RestoreHealth()
{
	lives = initial_lives;
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
	Position = checkpoint_pos;
}

void EPlayer::Die()
{
	lives = initial_lives;
	Velocity = vec3(0);
	player_state = NPlayerState::Standing;
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
	*Player = EPlayer{};
	return Player;
}

float EPlayer::GetSpeedLimit() const
{
	if (dashing)
	{
		return dash_speed;
	}
	else if (walking)
	{
		return walk_speed;
	}
	else
	{
		return run_speed;
	}
}

void GpCheckPlayerGrabbedLedge(EPlayer* Player, RWorld* World)
{
	RLedge Ledge = CL_PerformLedgeDetection(Player, World);
	if (Ledge.Empty)
		return;
	vec3 Position = CL_GetFinalPositionLedgeVaulting(player, ledge);

	RPlayerStateChangeArgs Args;
	Args.VaultingData.Ledge = Ledge;
	Args.VaultingData.FinalPosition = position;
	Player->ChangeStateTo(NPlayerState::Vaulting, Args);
}
