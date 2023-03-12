#include "game/animation/an_player.h"

#include "game/entities/player.h"
#include "engine/utils/utils.h"
#include "engine/camera/camera.h"
#include "engine/rvn.h"
#include "engine/world/scene_manager.h"
#include "game/gameplay/gp_player_state.h"

const std::map<PlayerAnimationState, float> PlayerAnimationDurations =
{
{PlayerAnimationState::Jumping, 400},
{PlayerAnimationState::Landing, 200},
{PlayerAnimationState::LandingFall, 400},
{PlayerAnimationState::Vaulting, 0}
};

void AN_AnimatePlayer(Player* player)
{
	if (player->anim_state == PlayerAnimationState::NoAnimation)
		return;

	// updates animation run time
	player->anim_t += Rvn::frame.duration * 1000;

	// check if animation is completed
	bool end_anim = false;

	auto* find_duration = Find(PlayerAnimationDurations, player->anim_state);
	if (!find_duration)
		return;

	float anim_duration = *find_duration;
	if (anim_duration > 0 && player->anim_t >= anim_duration)
	{
		player->anim_t = anim_duration;
		end_anim = true;
	}

	// dispatch call to correct update function depending on player animation state
	bool interrupt = false;
	switch (player->anim_state)
	{
		case PlayerAnimationState::Jumping:
		{
			interrupt = AN_UpdatePlayerJumpingAnimation(player);
			break;
		}

		case PlayerAnimationState::Landing:
		{
			interrupt = AN_UpdatePlayerLandingAnimation(player);
			break;
		}

		case PlayerAnimationState::LandingFall:
		{
			interrupt = AN_UpdatePlayerLandingFallAnimation(player);
			break;
		}

		case PlayerAnimationState::Vaulting:
		{
			interrupt = AN_PlayerVaulting(player);
			{
				if (interrupt)
				{
					GP_ChangePlayerState(player, PlayerState::Standing);
				}
				break;
			}
		}

		default:
			break;
	}

	// stop animation if completed or interrupted
	if (end_anim || interrupt)
	{
		player->anim_state = PlayerAnimationState::NoAnimation;
		player->anim_t = 0;
	}
}


bool AN_UpdatePlayerJumpingAnimation(Player* player)
{
	// // interpolate between 0 and duration the player's height
	// float anim_d            = PLAYER_ANIMATION_DURATIONS[PlayerAnimationState::Jumping];
	// // float new_half_height   = player->height - 0.1 * player->anim_t / anim_d;
	// float h_diff            = player->half_height - new_half_height;

	// // player->half_height = new_half_height;

	// // @todo: should modify collider here
	// // player->entity_ptr->collision_geometry.cylinder.half_length = new_half_height;
	// player->entity_ptr->scale.y -= h_diff;
	// // compensates player shrinkage so he appears to be lifting the legs up
	// player->entity_ptr->position.y += h_diff * 2;

	return false;
}


bool AN_UpdatePlayerLandingAnimation(Player* player)
{
	// bool interrupt = false;
	// // add a linear height step of 0.5m per second
	// float a_step = 0.5 * RVN::frame.duration; 
	// float new_half_height = player->half_height + a_step;
	// if(new_half_height >= player->height)
	// {
	//    new_half_height = player->height;
	//    a_step = player->height - player->entity_ptr->scale.y;
	//    interrupt = true;
	// }

	// player->half_height = new_half_height;

	// // @todo: should modify collider here
	// // player->entity_ptr->collision_geometry.cylinder.half_length = new_half_height;
	// player->entity_ptr->scale.y += a_step;

	// return interrupt;
	return false;
}


bool AN_UpdatePlayerLandingFallAnimation(Player* player)
{
	// float anim_d = PLAYER_ANIMATION_DURATIONS[PlayerAnimationState::LandingFall];
	// bool interrupt = false;
	// // sets the % of the duration of the animation that consists
	// // of player bending his knees on the fall, the rest is standing up again
	// float landing_d = anim_d * 0.25;

	// // landing part
	// if(player->anim_t <= landing_d)
	// {
	//    float new_half_height = player->height - 0.05 * player->anim_t / landing_d;
	//    float h_diff = player->half_height - new_half_height;

	//    player->half_height = new_half_height;

	//    // @todo: should modify collider here
	//    // player->entity_ptr->collision_geometry.cylinder.half_length = new_half_height;
	//    player->entity_ptr->scale.y -= h_diff;
	// }
	// // standing part
	// else if(player->anim_t > landing_d)
	// {
	//    float a_step = 0.5 * RVN::frame.duration; 
	//    float new_half_height = player->half_height + a_step;
	//    if(new_half_height >= player->height)
	//    {
	//       new_half_height = player->height;
	//       a_step = player->height - player->entity_ptr->scale.y;
	//       interrupt = true;
	//    }

	//    player->half_height = new_half_height;

	//    // @todo: should modify collider here
	//    // player->entity_ptr->collision_geometry.cylinder.half_length = new_half_height;
	//    player->entity_ptr->scale.y += a_step;
	// }

	// return interrupt;
	return false;
}


bool AN_PlayerVaulting(Player* player)
{
	vec3& p_pos = player->entity_ptr->position;

	// animation speed in m/s
	const float v_y = 2.f / 1.f;
	const float v_xz = 2.f / 2.f;

	vec3 anim_trajectory = player->anim_final_pos - player->anim_orig_pos;

	vec3 dist = player->anim_final_pos - p_pos;
	auto dist_sign = vec3(sign(dist.x), sign(dist.y), sign(dist.z));
	auto ds = vec3(v_xz * Rvn::frame.duration, v_y * Rvn::frame.duration, v_xz * Rvn::frame.duration);

	// updates player position
	for (int i = 0; i < 3; i++)
	{
		// I feel like the sign here is unnecessary if we have a anim_direction set
		if (abs(dist[i]) >= ds[i] && sign(anim_trajectory[i]) == dist_sign[i])
			p_pos[i] += dist_sign[i] * ds[i];
		else
			p_pos[i] = player->anim_final_pos[i];
	}


	// camera direction animation
	if (!player->anim_finished_turning)
	{
		auto* player_camera = GlobalSceneInfo::GetGameCam();

		vec2 f_dir_xz = to2d_xz(player->anim_final_dir);
		float orig_sva = vector_angle_signed(nrmlz(to2d_xz(player->anim_orig_dir)), f_dir_xz);
		float orig_angle = glm::degrees(orig_sva);
		float orig_sign = sign(orig_angle);
		float turn_angle = 0.5 * orig_sign;
		ChangeCameraDirection(player_camera, turn_angle, 0.f);

		float updated_sva = vector_angle_signed(nrmlz(to2d_xz(player_camera->front)), f_dir_xz);
		float updated_angle = glm::degrees(updated_sva);
		float updated_sign = sign(updated_angle);
		if (updated_sign != orig_sign)
		{
			ChangeCameraDirection(player_camera, -1.0 * updated_angle, 0.f);
			player->anim_finished_turning = true;
		}
	}

	/*
	RVN::print_dynamic("front: " + to_string(nrmlz(to2d_xz(pCam->Front))));
	RVN::print_dynamic("final dir: " + to_string(player->anim_final_dir), 0, vec3(0.8, 0.8, 0.8));
	RVN::print_dynamic("orig angle: " + to_string(orig_angle), 0, vec3(0.8, 0.8, 0.8));
	RVN::print_dynamic("current angle: " +  to_string(updated_angle));
	RVN::print_dynamic("sva cam-final: " +  to_string(updated_sva), 0, vec3(0,0.8,0.1));
	RVN::print_dynamic("sva orig-final: " +  to_string(orig_sva), 0, vec3(0,0.8,0.1));
	RVN::print_dynamic("orig sign: " +  to_string(orig_sign), 0, vec3(0.8,0.0,0.1));
	RVN::print_dynamic("updated sign: " +  to_string(updated_sign), 0, vec3(0.8,0.0,0.1));
	*/

	if (is_equal(p_pos, player->anim_final_pos) && player->anim_finished_turning)
	{
		return true;
	}
	return false;
}


void ForceInterruptPlayerAnimation(Player* player)
{
	// player->anim_state = PlayerAnimationState::NoAnimation;
	// player->anim_t = 0;
	// // player->half_height = player->height;
	// player->entity_ptr->scale.y = player->height;

	// // @todo: should modify collider here
	// // player->entity_ptr->collision_geometry.cylinder.half_length = player->height;
}
