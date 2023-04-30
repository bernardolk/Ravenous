#include "game/entities/player.h"

#include "engine/rvn.h"
#include "engine/camera/camera.h"
#include "engine/collision/cl_controller.h"
#include "engine/collision/cl_types.h"
#include "engine/io/input.h"
#include "engine/render/im_render.h"
#include "engine/world/scene_manager.h"
#include "engine/world/world.h"

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
const float Player::air_acceleration = 0.5f;
const float Player::max_air_speed = 0.2f;
const float Player::air_friction = 5.0f;
const float Player::jump_initial_speed = 10.0f;
const float Player::jump_initial_horizontal_thrust = 3.0f;
const float Player::jump_reduced_horizontal_thrust = 1.5f;
const float Player::jump_horz_dash_thrust = 4.5f;
const float Player::slide_jump_speed = 6.7f;
const float Player::slide_speed = 2.0f;
const float Player::fall_from_edge_push_speed = 1.5f;
const vec3 Player::gravity = vec3(0, -14.0, 0);

void Player::Update(bool update_collider)
{
	// perform updates to bounding boxes, colliders etc
	UpdateModelMatrix();
	if (update_collider)
	{
		UpdateCollider();
		UpdateBoundingBox();
	}

	if (CL_UpdatePlayerWorldCells(this))
	{
		CL_RecomputeCollisionBufferEntities(this);
	}
}

void Player::UpdateState()
{
	T_World* world = T_World::Get();
	float dt = Rvn::GetFrameDuration();

	// -------------
	// Standing
	// -------------
	IfState(Standing)
	{
		MoveForward();

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
				Update();
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
				vec2 fall_momentum_dir = v_dir_historic.xz;
				vec2 fall_momentum = fall_momentum_dir * fall_momentum_intensity;

				velocity = vec3(fall_momentum.x, 0, fall_momentum.y);
				ChangeStateTo(PlayerState::Falling);
				Update(true);
				break;
			}

			// Collided with nothing or with terrain only, break
			if (results.count == 0 || (collided_with_terrain && results.count == 1))
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
		
		position += velocity * dt + gravity * dt * dt / 2.f;
		
		Update(true);

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

		velocity += gravity * dt;
		position += velocity * dt + gravity * dt * dt / 2.f;
		
		Update(true);

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
		if (results.count > 0)
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
		Update(true);


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
	// Air movement
	if (false)
	{
		air_velocity += v_dir * air_acceleration * dt;
		if (length(air_velocity) > max_air_speed)
		{
			air_velocity = v_dir * max_air_speed;
		}
		velocity += air_velocity;
	}
		
	velocity += gravity * dt;

	// Friction
	float h_speed = GetHorizontalSpeed();  
	if (!pressing_forward_while_in_air && h_speed > jump_reduced_horizontal_thrust + 0.001f)
	{
		vec3 movement_direction = GetHorizontalMovementForwardVector();
		vec3 friction = movement_direction * air_friction * dt;
		velocity -= friction;
		h_speed = GetHorizontalSpeed();
		if (h_speed < jump_reduced_horizontal_thrust)
		{
			vec3 hv = jump_reduced_horizontal_thrust * movement_direction;
			velocity.x = hv.x;
			velocity.z = hv.z;
		}
	}
}

void Player::ChangeStateTo(PlayerState new_state, PlayerStateChangeArgs args)
{
	IfStateChange(Jumping, Falling)
	{
		player_state = PlayerState::Falling;
		velocity.y = 0;
		// jumping_upwards = false;

		// TEMP - DELETE LATER
		skip_collision_with_floor = nullptr;
	}

	else IfStateChange(Standing, Jumping)
	{
		// If jumping up
		if ( v_dir.x == 0 && v_dir.z == 0 && GetSpeed() == 0)
		{
			BruteStop();
		}
		else
		{
			if (dashing)
				velocity = v_dir * jump_horz_dash_thrust;
			else
				velocity = v_dir * jump_initial_horizontal_thrust;
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
		air_velocity = vec3(0.f);
		MultiplySpeed(0.f);

		// take momentum hit from hitting the ground
		// velocity.x *= 0.5;
		// velocity.z *= 0.5;

		player_state = PlayerState::Standing;

		// conditional animation: if falling from jump, land, else, land from fall
		if (height < height) // TODO: ??
			anim_state = PlayerAnimationState::Landing;
		else
			anim_state = PlayerAnimationState::LandingFall;

		MaybeHurtFromFall();
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
		standing_entity_ptr = args.entity;

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
		vaulting_entity_ptr = grabbing_entity;
		grabbing_entity = nullptr;
	}

	else IfStateChange(Standing, Vaulting)
	{
		auto* GII = GlobalInputInfo::Get();
		GII->block_mouse_move = true;
		auto* player_camera = GlobalSceneInfo::GetGameCam();

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
		standing_entity_ptr = vaulting_entity_ptr;
		vaulting_entity_ptr = nullptr;
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

		velocity = v_dir * jump_initial_horizontal_thrust;
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
	{
		fatal_error(TEXT("There is no link to change Player State from ", player_state, " to ", new_state, "."));
	}
}

vec3 Player::MoveForward()
{
	bool no_move_command = v_dir.x == 0 && v_dir.z == 0;

	float dt = Rvn::frame.duration;

	// Updates v_dir_historic
	if (v_dir.x != 0 || v_dir.y != 0 || v_dir.z != 0)
	{
		v_dir_historic = v_dir;
	}
	else if (v_dir_historic == vec3(0))
	{
		v_dir_historic = normalize(ToXz(GlobalSceneInfo::Get()->views[GameCam]->front));
	}

	float d_speed = acceleration * dt;
	
	// If stopped
	if (GetSpeed() > 0 && no_move_command)
	{
		d_speed = 0;
	}

	SetSpeed(GetSpeed() + d_speed);

	float speed_limit = GetSpeedLimit();
	
	if (GetSpeed() > speed_limit)
	{
		SetSpeed(speed_limit);
	}

	vec3 next_position = position + velocity * dt;
	
	// update things
	bounding_box.Translate(next_position - position);
	position = next_position;
	Update();

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

void GP_CheckPlayerGrabbedLedge(Player* player, T_World* world)
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
