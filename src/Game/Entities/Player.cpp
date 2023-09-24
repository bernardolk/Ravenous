#include "game/entities/player.h"

#include "engine/rvn.h"
#include "engine/camera/camera.h"
#include "engine/collision/ClController.h"
#include "engine/collision/ClTypes.h"
#include "engine/io/input.h"
#include "engine/render/ImRender.h"
#include "engine/world/world.h"
#include "game/input/PlayerInput.h"

#define IfStateChange(X, Y) if (player_state == PlayerState::X && new_state == PlayerState::Y)
#define IfState(X) if (player_state == PlayerState::X)

// -----------------
// Player Constants
// -----------------
const float Player::acceleration = 12.0f;
const float Player::run_speed = 4.0f;
const float Player::dash_speed = 8.0f;
const float Player::walk_speed = 0.95f;
const float Player::fall_speed = 0.01f;
const float Player::air_steering_velocity = 1.5f;
const float Player::max_air_speed = 0.2f;
const float Player::air_friction = 5.0f;
const float Player::jump_initial_speed = 7.0f;
const float Player::jump_reduced_horizontal_thrust = 1.5f;
const float Player::minimum_jump_horizontal_thrust_when_dashing = 5.5f;
const float Player::minimum_jump_horizontal_thrust_when_running = 3.0f;
const float Player::jump_from_slope_horizontal_thrust = 4.0f;
const float Player::slide_jump_speed = 6.7f;
const float Player::slide_speed = 2.0f;
const float Player::fall_from_edge_push_speed = 1.5f;
const vec3 Player::gravity = vec3(0, -18.0, 0);

void Player::Update()
{
	static_cast<E_Entity*>(this)->Update();

	if (CL_UpdatePlayerWorldCells(this))
	{
		CL_RecomputeCollisionBufferEntities();
	}
}

void Player::UpdateState()
{
	World* world = World::Get();
	float dt = Rvn::GetFrameDuration();

	// -------------
	// Standing
	// -------------
	IfState(Standing)
	{
		MoveForward();
		Update();

		vec3 player_btm_sphere_center = position + vec3(0, radius, 0);
		vec3 contact_point = player_btm_sphere_center + -last_terrain_contact_normal * radius;
		ImDraw::AddLine(IMHASH, player_btm_sphere_center, contact_point, COLOR_YELLOW_1);

		/* Current system: Here we are looping at most twice on the:
		   "Do stepover Vtrace, Adjust player's position to terrain, check collisions" loop
		   so we can detect when we try stepping up/down into a place where the player can't
		   fit in.
		*/
		// TODO: Not sure why we are looping here actually. Once I figure out, please explain why we can't run it once.
		for (int it = 0; it < 2; it ++)
		{
			auto vtrace = CL_DoStepoverVtrace(this, world);

			// snap player to the last terrain contact point detected if its a valid stepover hit
			if (vtrace.hit && (vtrace.delta_y > 0.0004 || vtrace.delta_y < 0))
			{
				position.y -= vtrace.delta_y;
				bounding_box.Translate(vec3(0, -vtrace.delta_y, 0));
			}

			// resolve collisions
			auto results = CL_TestAndResolveCollisions(this);

			// iterate on collision results
			bool collided_with_terrain = false;
			ClResults slope;
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
				float fall_momentum_intensity = GetSpeed() < fall_from_edge_push_speed ? fall_from_edge_push_speed : GetSpeed();
				vec2 fall_momentum_dir = last_recorded_movement_direction.xz;
				vec2 fall_momentum = fall_momentum_dir * fall_momentum_intensity;

				velocity = vec3(fall_momentum.x, 0, fall_momentum.y);
				ChangeStateTo(PlayerState::Falling);
				Update();
				break;
			}

			// Collided with nothing or with terrain only, break
			if (results.Num() == 0 || (collided_with_terrain && results.Num() == 1))
				break;

			if (slope.collision)
			{
				PlayerStateChangeArgs args;
				args.normal = slope.normal;
				ChangeStateTo(PlayerState::Sliding, args);
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
	else IfState(Falling)
	{
		UpdateAirMovement(dt);
				
		Update();

		auto results = CL_TestAndResolveCollisions(this);
		for (auto& result : results)
		{
			// slope collision
			{
				bool collided_with_slope = dot(result.normal, UnitY) >= slope_min_angle;
				if (collided_with_slope && result.entity->slidable)
				{
					PlayerStateChangeArgs args;
					args.normal = result.normal;
					ChangeStateTo(PlayerState::Sliding, args);
					return;
				}
			}

			// floor collision
			{
				// If collided with terrain
				if (dot(result.normal, UnitY) > 0)
				{
					ChangeStateTo(PlayerState::Standing);
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
	else IfState(Jumping)
	{
		UpdateAirMovement(dt);
		
		Update();

		auto results = CL_TestAndResolveCollisions(this);
		for (auto& result : results)
		{
			// collision with terrain while jumping should be super rare I guess ...
			// slope collision
			bool collided_with_slope = dot(result.normal, UnitY) >= slope_min_angle;
			if (collided_with_slope && result.entity->slidable)
			{
				PlayerStateChangeArgs args;
				args.normal = result.normal;
				ChangeStateTo(PlayerState::Sliding, args);
				return;
			}
			

			// floor collision 
			// If collided with terrain
			else if (dot(result.normal, UnitY) > 0)
			{
				ChangeStateTo(PlayerState::Standing);
				return;
			}
			
			else
				CL_WallSlidePlayer(this, result.normal);
		}

		// @todo - need to include case that player touches inclined terrain
		//          in that case it should also stand (or fall from ledge) and not
		//          directly fall.
		if (results.Num() > 0)
			ChangeStateTo(PlayerState::Falling);

		else if (velocity.y <= 0)
			ChangeStateTo(PlayerState::Falling);
	}
	
	// -------------
	// Sliding
	// -------------
	else IfState(Sliding)
	{
		ImDraw::AddLine(IMHASH, position, position + 1.f * sliding_direction, COLOR_RED_2);

		velocity = v_dir * slide_speed;

		position += velocity * Rvn::frame.duration;
		Update();


		// RESOLVE COLLISIONS AND CHECK FOR TERRAIN CONTACT
		auto results = CL_TestAndResolveCollisions(this);

		bool collided_with_terrain = false;
		for (auto& result: results)
		{
			// iterate on collision results
			collided_with_terrain = dot(result.normal, UnitY) > 0;
			if (collided_with_terrain)
				last_terrain_contact_normal = result.normal;
		}

		if (collided_with_terrain)
		{
			ChangeStateTo(PlayerState::Standing);
			return;
		}

		auto vtrace = CL_DoStepoverVtrace(this, world);
		if (!vtrace.hit)
		{
			ChangeStateTo(PlayerState::Falling);
		}
	}
}

void Player::UpdateAirMovement(float dt)
{
	velocity += gravity * dt;

	vec3 jumping_direction = GetHorizontalMovementForwardVector();
	float h_speed = GetHorizontalSpeed();  

	// Air steering
	if (pressing_left_while_in_air)
	{
		vec3 relative_motion_vec = IsEqual(h_speed, 0) ? v_dir : -normalize(ToXz(Cross(jumping_direction, UnitY)));
		position += relative_motion_vec * air_steering_velocity * dt;
	}
	if (pressing_right_while_in_air)
	{
		vec3 relative_motion_vec = IsEqual(h_speed, 0) ? v_dir : normalize(ToXz(Cross(jumping_direction, UnitY)));
		position += relative_motion_vec * air_steering_velocity * dt;
	}
	// Extra steering if jumping upwards
	if ((player_state == PlayerState::Jumping && IsEqual(h_speed, 0)) || player_state == PlayerState::Falling)
	{
		if (pressing_forward_while_in_air || pressing_backward_while_in_air)
			position += v_dir * air_steering_velocity * dt;
	}
	
	// Friction
	h_speed = GetHorizontalSpeed();  
	if (stopped_pressing_forward_while_in_air && h_speed > jump_reduced_horizontal_thrust)
	{
		vec3 friction = jumping_direction * air_friction * dt;
		velocity -= friction;
		h_speed = GetHorizontalSpeed();
		if (h_speed < jump_reduced_horizontal_thrust)
		{
			vec3 hv = jump_reduced_horizontal_thrust * jumping_direction;
			velocity.x = hv.x;
			velocity.z = hv.z;
		}
	}

	position += velocity * dt + gravity * dt * dt / 2.f;
}

void Player::ChangeStateTo(PlayerState new_state, PlayerStateChangeArgs args)
{
	IfStateChange(Jumping, Falling)
	{
		player_state = PlayerState::Falling;
		velocity.y = 0;
		// jumping_upwards = false;
	}

	else IfStateChange(Standing, Jumping)
	{
		// If jumping up
		if (!IsMovingThisFrame() && GetSpeed() == 0)
		{
			BruteStop();
		}
		else if (pressing_forward_while_standing)
		{
			float horizontal_thrust = GetHorizontalSpeed();
			if (dashing && horizontal_thrust < minimum_jump_horizontal_thrust_when_dashing)
				horizontal_thrust = minimum_jump_horizontal_thrust_when_dashing;
			else if (!dashing && horizontal_thrust < minimum_jump_horizontal_thrust_when_running)
				horizontal_thrust = minimum_jump_horizontal_thrust_when_running;

			auto* player_camera = CameraManager::Get()->GetGameCamera();
			velocity = player_camera->front * horizontal_thrust;
		}

		velocity.y = jump_initial_speed;
		player_state = PlayerState::Jumping;
		anim_state = PlayerAnimationState::Jumping;
		height_before_fall = position.y;
	}

	else IfStateChange(Standing, Falling)
	{
		player_state = PlayerState::Falling;
		velocity.y = -1 * fall_speed;
		velocity.x *= 0.5;
		velocity.z *= 0.5;
		height_before_fall = position.y;
	}

	else IfStateChange(Falling, Standing)
	{
		velocity.y = 0;
		
		player_state = PlayerState::Standing;

		// conditional animation: if falling from jump, land, else, land from fall
		if (height < height) // TODO: ??
			anim_state = PlayerAnimationState::Landing;
		else
			anim_state = PlayerAnimationState::LandingFall;

		MaybeHurtFromFall();
	}

	else IfStateChange(Jumping, Standing)
	{
		velocity.y = 0;
		
		MultiplySpeed(0.f);

		player_state = PlayerState::Standing;
		anim_state = PlayerAnimationState::Landing;
		//MaybeHurtFromFall();
	}

	else IfStateChange(Standing, Sliding)
	{
		/* Parameters:
		   - vec3 normal : the normal of the slope (collider triangle) player is currently sliding
		*/

		player_state = PlayerState::Sliding;

		auto down_vec_into_n = ProjectVecIntoRef(-UnitY, args.normal);
		auto sliding_direction = normalize(-UnitY - down_vec_into_n);
		sliding_direction = sliding_direction;
		sliding_normal = args.normal;
	}

	else IfStateChange(Standing, SlideFalling)
	{
		// make player 'snap' to slope velocity-wise
		// velocity = slide_speed * ramp->collision_geometry.slope.tangent;

		player_state = PlayerState::SlideFalling;
	}

	else IfStateChange(Jumping, Grabbing)
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

	else IfStateChange(Grabbing, Vaulting)
	{
		player_state = PlayerState::Vaulting;
		anim_state = PlayerAnimationState::Vaulting;
		grabbing_entity = nullptr;
	}

	else IfStateChange(Standing, Vaulting)
	{
		auto* GII = GlobalInputInfo::Get();
		GII->block_mouse_move = true;
		auto* player_camera = CameraManager::Get()->GetGameCamera();

		player_state = PlayerState::Vaulting;
		anim_state = PlayerAnimationState::Vaulting;
		velocity = vec3(0);

		anim_orig_pos = position;
		anim_orig_dir = normalize(ToXz(player_camera->front));

		auto inward_normal = normalize(Cross(args.vaulting_data.ledge.a - args.vaulting_data.ledge.b, UnitY));
		anim_final_pos = args.vaulting_data.final_position;
		anim_final_dir = inward_normal;
	}

	else IfStateChange(Vaulting, Standing)
	{
		auto* GII = GlobalInputInfo::Get();

		GII->forget_last_mouse_coords = true;
		GII->block_mouse_move = false;
		player_state = PlayerState::Standing;
		anim_finished_turning = false;
	}

	else IfStateChange(Sliding, Standing)
	{
		sliding_direction = vec3(0);
		sliding_normal = vec3(0);
		player_state = PlayerState::Standing;
	}

	else IfStateChange(Sliding, Jumping)
	{
		v_dir = sliding_normal;
		sliding_normal = vec3(0);

		velocity = v_dir * jump_from_slope_horizontal_thrust;
		velocity.y = jump_initial_speed;

		player_state = PlayerState::Jumping;
		anim_state = PlayerAnimationState::Jumping;
		height_before_fall = position.y;
	}

	else IfStateChange(Sliding, Falling)
	{
		player_state = PlayerState::Falling;
		velocity.y = -fall_speed;
		height_before_fall = position.y;
	}

	else 
		fatal_error("There is no link to change Player State from %i to %i.", player_state, new_state);
}

vec3 Player::MoveForward()
{
	bool no_move_command = v_dir.x == 0 && v_dir.z == 0;

	float dt = Rvn::frame.duration;

	// Limiting movement angle when moving in diagonals
	auto* player_camera = CameraManager::Get()->GetGameCamera();

	// TODO: Implement for the other axis as well
	//		Cleanup the mess of using KEYs for this
	//		Cleanup the mess on process input code
	//		Maybe this should be moved to where we set v_dir
	if (first_pressed_movement_key_while_standing == KEY_MOVE_UP)
	{
		if (pressing_left_while_standing)
			v_dir = normalize(rotate(player_camera->front, PI/10.f, UnitY));
		else if (pressing_right_while_standing)
			v_dir = normalize(rotate(player_camera->front, PI/10.f, -UnitY));
	}
	
	
	if (!IsEqual(length(v_dir), 0))
		last_recorded_movement_direction = v_dir;
	else if (last_recorded_movement_direction == vec3(0))
		last_recorded_movement_direction = normalize(ToXz( CameraManager::Get()->GetGameCamera()->front ));
	
	float d_speed = acceleration * dt;
	
	// If stopped
	if (GetSpeed() > 0 && no_move_command)
		d_speed = 0;

	SetSpeed(GetSpeed() + d_speed);

	float speed_limit = GetSpeedLimit();
	
	if (GetSpeed() > speed_limit)
		SetSpeed(speed_limit);

	vec3 next_position = position + velocity * dt;
	
	// update things
	position = next_position;
	return next_position;
}

vec3 Player::GetLastTerrainContactPoint() const
{
	const vec3 player_btm_sphere_center = position + vec3(0, radius, 0);
	return player_btm_sphere_center + -last_terrain_contact_normal * radius;
}

bool Player::MaybeHurtFromFall()
{
	float fall_height = height_before_fall - position.y;
	fall_height_log = fall_height;
	if (fall_height >= hurt_height_2)
	{
		lives -= 2;
		return true;
	}
	if (fall_height >= hurt_height_1)
	{
		lives -= 1;
		return true;
	}
	return false;
}

void Player::RestoreHealth()
{
	lives = initial_lives;
}

void Player::SetCheckpoint(E_Entity* entity)
{
	return;
	/*
	if (entity->type != EntityType_Checkpoint)
		assert(false);

	checkpoint_pos = position;
	checkpoint = entity;
	*/
}

void Player::GotoCheckpoint()
{
	position = checkpoint_pos;
}

void Player::Die()
{
	lives = initial_lives;
	velocity = vec3(0);
	player_state = PlayerState::Standing;
	ForceInterruptPlayerAnimation(this);
	GotoCheckpoint();
}

void Player::BruteStop()
{
	// bypass deaceleration steps. Stops player right on his tracks.
	velocity = vec3(0);
}

Player* Player::ResetPlayer()
{
	auto* player = Get();
	*player = Player{};
	return player;
}

float Player::GetSpeedLimit() const
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

void GP_CheckPlayerGrabbedLedge(Player* player, World* world)
{
	Ledge ledge = CL_PerformLedgeDetection(player, world);
	if (ledge.empty)
		return;
	vec3 position = CL_GetFinalPositionLedgeVaulting(player, ledge);

	PlayerStateChangeArgs args;
	args.vaulting_data.ledge = ledge;
	args.vaulting_data.final_position = position;
	player->ChangeStateTo(PlayerState::Vaulting, args);
}
