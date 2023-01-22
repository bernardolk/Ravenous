#pragma once

/* 
   --------------------
   > PLAYER STATE CODE
   --------------------
   Work in progress state machine-like modelling of player state 
*/
void GP_player_state_change_jumping_to_falling(Player* player);
void GP_player_state_change_standing_to_falling(Player* player);
void GP_player_state_change_falling_to_standing(Player* player);
void GP_player_state_change_standing_to_jumping(Player* player);
void GP_player_state_change_any_to_sliding(Player* player, vec3 normal);
void GP_player_state_change_any_to_grabbing(Player* player, Entity* entity, vec2 normal_vec, vec3 final_position, float d);
void GP_player_state_change_grabbing_to_vaulting(Player* player);
void GP_player_state_change_standing_to_vaulting(Player* player, Ledge ledge, vec3 final_position);
void GP_player_state_change_vaulting_to_standing(Player* player);
void GP_player_state_change_standing_to_slide_falling(Player* player, Entity* ramp);
void GP_player_state_change_sliding_to_standing(Player* player);
void GP_player_state_change_sliding_to_jumping(Player* player);
void GP_player_state_change_sliding_to_falling(Player* player);



struct PlayerStateChangeArgs
{

	// collision
	Entity* entity = nullptr;
	vec3 normal = vec3(0);
	float penetration = 0;

	// grabbing info
	vec3 final_position = vec3(0);
	Ledge ledge;
};

inline void GP_change_player_state(Player* player, PlayerState new_state, PlayerStateChangeArgs args = {})
{
	/* This will change the player's state to the new state and do actions based on his current state.
	   Hopefuly we can achieve a state machine model where all transitions are mapped and, therefore, predictable.
	*/

	// Player is...

	// IN ANY STATE
	switch(new_state)
	{
		case PLAYER_STATE_GRABBING: return GP_player_state_change_any_to_grabbing(player, args.entity, args.normal, args.final_position, args.penetration);

		case PLAYER_STATE_SLIDING: return GP_player_state_change_any_to_sliding(player, args.normal);
	}

	switch(player->player_state)
	{
		// STANDING
		case PLAYER_STATE_STANDING: switch(new_state)
			{
				case PLAYER_STATE_FALLING: return GP_player_state_change_standing_to_falling(player);

				case PLAYER_STATE_JUMPING: return GP_player_state_change_standing_to_jumping(player);

				case PLAYER_STATE_SLIDE_FALLING: return GP_player_state_change_standing_to_slide_falling(player, args.entity);

				case PLAYER_STATE_VAULTING: return GP_player_state_change_standing_to_vaulting(player, args.ledge, args.final_position);
			}

		// JUMPING
		case PLAYER_STATE_JUMPING: switch(new_state)
			{
				case PLAYER_STATE_FALLING: return GP_player_state_change_jumping_to_falling(player);
			}

		// FALLING
		case PLAYER_STATE_FALLING: switch(new_state)
			{
				case PLAYER_STATE_STANDING: return GP_player_state_change_falling_to_standing(player);
			}

		// GRABBING
		case PLAYER_STATE_GRABBING: switch(new_state)
			{
				case PLAYER_STATE_VAULTING: return GP_player_state_change_grabbing_to_vaulting(player);
			}

		// VAULTING
		case PLAYER_STATE_VAULTING: switch(new_state)
			{
				case PLAYER_STATE_STANDING: return GP_player_state_change_vaulting_to_standing(player);
			}

		// SLIDING
		case PLAYER_STATE_SLIDING: switch(new_state)
			{
				case PLAYER_STATE_STANDING: return GP_player_state_change_sliding_to_standing(player);

				case PLAYER_STATE_JUMPING: return GP_player_state_change_sliding_to_jumping(player);

				case PLAYER_STATE_FALLING: return GP_player_state_change_sliding_to_falling(player);
			}

		default:
			assert(false);
	}
}


inline void GP_player_state_change_jumping_to_falling(Player* player)
{
	player->player_state = PLAYER_STATE_FALLING;
	player->entity_ptr->velocity.y = 0;
	player->jumping_upwards = false;

	// TEMP - DELETE LATER
	player->skip_collision_with_floor = nullptr;
}


inline void GP_player_state_change_standing_to_jumping(Player* player)
{
	auto& v = player->entity_ptr->velocity;
	auto& v_dir = player->v_dir;
	bool no_move_command = v_dir.x == 0 && v_dir.z == 0;

	if(no_move_command)
	{
		player->jumping_upwards = true;
		player->speed = 0;
	}
	else
	{
		if(player->dashing)
			v = v_dir * player->jump_horz_dash_thrust;
		else
			v = v_dir * player->jump_horz_thrust;
	}

	v.y = player->jump_initial_speed;
	player->player_state = PLAYER_STATE_JUMPING;
	player->anim_state = PlayerAnimationState_Jumping;
	player->height_before_fall = player->entity_ptr->position.y;
}


inline void GP_player_state_change_standing_to_falling(Player* player)
{
	player->player_state = PLAYER_STATE_FALLING;
	player->entity_ptr->velocity.y = -1 * player->fall_speed;
	player->entity_ptr->velocity.x *= 0.5;
	player->entity_ptr->velocity.z *= 0.5;
	player->height_before_fall = player->entity_ptr->position.y;
}


inline void GP_player_state_change_falling_to_standing(Player* player)
{
	player->entity_ptr->velocity.y = 0;
	player->speed *= 0.5;

	// take momentum hit from hitting the ground
	// player->entity_ptr->velocity.x *= 0.5;
	// player->entity_ptr->velocity.z *= 0.5;

	player->player_state = PLAYER_STATE_STANDING;

	// conditional animation: if falling from jump, land, else, land from fall
	if(player->height < player->height)
		player->anim_state = PlayerAnimationState_Landing;
	else
		player->anim_state = PlayerAnimationState_LandingFall;

	player->MaybeHurtFromFall();
}


inline void GP_player_state_change_any_to_sliding(Player* player, vec3 normal)
{
	/* Parameters:
	   - vec3 normal : the normal of the slope (collider triangle) player is currently sliding
	*/

	player->player_state = PLAYER_STATE_SLIDING;

	auto down_vec_into_n = project_vec_into_ref(-UnitY, normal);
	auto sliding_direction = normalize(-UnitY - down_vec_into_n);
	player->sliding_direction = sliding_direction;
	player->sliding_normal = normal;

}


inline void GP_player_state_change_standing_to_slide_falling(Player* player, Entity* ramp)
{
	player->standing_entity_ptr = ramp;

	// make player 'snap' to slope velocity-wise
	// player->entity_ptr->velocity = player->slide_speed * ramp->collision_geometry.slope.tangent;

	player->player_state = PLAYER_STATE_SLIDE_FALLING;
}


// @TODO REFACTOR -> From aabb and 2d normals to full 3d
inline void GP_player_state_change_any_to_grabbing(Player* player, Entity* entity, vec2 normal_vec, vec3 final_position, float penetration)
{
	// vec3 rev_normal = rev_2Dnormal(normal_vec);

	// // this will be an animation in the future
	// float turn_angle = glm::degrees(vector_angle_signed(to2d_xz(pCam->Front), normal_vec)) - 180;
	// camera_change_direction(pCam, turn_angle, 0.f);
	// // CL_snap_player(player, normal_vec, penetration);

	// player->player_state          = PLAYER_STATE_GRABBING;
	// player->grabbing_entity       = entity;
	// player->entity_ptr->velocity  = vec3(0);
	// // after we are able to move while grabbing the ledge, this should move away from here
	// {
	//    player->anim_final_dir        = rev_normal;
	//    player->anim_final_pos        = final_position;
	//    player->anim_orig_pos         = player->entity_ptr->position;
	//    player->anim_orig_dir         = normalize(to_xz(pCam->Front));
	//    player->entity_ptr->velocity  = vec3(0);
	// }
}

// DONE
inline void GP_player_state_change_grabbing_to_vaulting(Player* player)
{
	player->player_state = PLAYER_STATE_VAULTING;
	player->anim_state = PlayerAnimationState_Vaulting;
	player->vaulting_entity_ptr = player->grabbing_entity;
	player->grabbing_entity = nullptr;
}

inline void GP_player_state_change_standing_to_vaulting(Player* player, Ledge ledge, vec3 final_position)
{
	auto* GII = GlobalInputInfo::Get();
	GII->block_mouse_move = true;
	auto* player_camera = GlobalSceneInfo::GetGameCam();
	
	player->player_state = PLAYER_STATE_VAULTING;
	player->anim_state = PlayerAnimationState_Vaulting;
	player->entity_ptr->velocity = vec3(0);

	player->anim_orig_pos = player->entity_ptr->position;
	player->anim_orig_dir = normalize(to_xz(player_camera->front));

	auto inward_normal = normalize(cross(ledge.a - ledge.b, UnitY));
	player->anim_final_pos = final_position;
	player->anim_final_dir = inward_normal;
}

// DONE
inline void GP_player_state_change_vaulting_to_standing(Player* player)
{
	auto* GII = GlobalInputInfo::Get();
	
	GII->forget_last_mouse_coords = true;
	GII->block_mouse_move = false;
	player->player_state = PLAYER_STATE_STANDING;
	player->standing_entity_ptr = player->vaulting_entity_ptr;
	player->vaulting_entity_ptr = nullptr;
	player->anim_finished_turning = false;
}


inline void GP_player_state_change_sliding_to_standing(Player* player)
{
	player->sliding_direction = vec3(0);
	player->sliding_normal = vec3(0);
	player->player_state = PLAYER_STATE_STANDING;
}


inline void GP_player_state_change_sliding_to_jumping(Player* player)
{
	player->v_dir = player->sliding_normal;
	player->sliding_normal = vec3(0);

	player->entity_ptr->velocity = player->v_dir * player->jump_horz_thrust;
	player->entity_ptr->velocity.y = player->jump_initial_speed;

	player->player_state = PLAYER_STATE_JUMPING;
	player->anim_state = PlayerAnimationState_Jumping;
	player->height_before_fall = player->entity_ptr->position.y;
}


inline void GP_player_state_change_sliding_to_falling(Player* player)
{
	player->player_state = PLAYER_STATE_FALLING;
	player->entity_ptr->velocity.y = -player->fall_speed;
	player->height_before_fall = player->entity_ptr->position.y;
}
